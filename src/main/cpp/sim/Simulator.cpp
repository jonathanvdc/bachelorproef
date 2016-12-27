/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the Simulator class.
 */

#include "Simulator.h"

#include "core/ClusterType.h"
#include "core/ContactProfile.h"
#include "core/Infector.h"
#include "core/LogMode.h"
#include "core/MasterProfile.h"
#include "core/Population.h"
#include "core/PopulationBuilder.h"
#include "sim/Calendar.h"
#include "util/InstallDirs.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <iostream>
#include <stdexcept>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::util;

Simulator::Simulator(const boost::property_tree::ptree& pt_config)
	: m_config_pt(pt_config), m_num_threads(1U), m_population(nullptr)
{
	#pragma omp parallel
	{
		m_num_threads = omp_get_num_threads();
	}

	// Initialize calendar.
	m_calendar = make_shared<Calendar>(pt_config);

	// Get log level.
	const string l = pt_config.get<string>("run.log_level", "None");
	m_log_level = IsLogMode(l) ? ToLogMode(l) : throw runtime_error(string(__func__) + "> Invalid input for LogMode.");

	// Get the disease configuration.
	ptree pt_disease;
	const auto file_name { pt_config.get<string>("run.disease_config_file") };
	const auto file_path { InstallDirs::GetDataDir() /= file_name };
	if ( !is_regular_file(file_path) ) {
	        throw runtime_error(std::string(__func__)  + "> No file " + file_path.string());
	}
	read_xml(file_path.string(), pt_disease);

	// Build population.
	cerr << "Building the population. "<< endl;
	m_population = PopulationBuilder::Build(pt_config, pt_disease);

        // Initialize clusters.
	cerr << "Initializing the clusters. "<< endl;
	InitializeClusters();

	// Contact handlers: R0 and transmission rate;use linear model fitted
	// to simulation data: Expected(R0) = (b0+b1*transm_rate)
	cerr << "Setting up contact handlers. "<< endl;
        const double r0   = pt_config.get<double>("run.r0");
        const double b0   = pt_disease.get<double>("disease.transmission.b0");
        const double b1   = pt_disease.get<double>("disease.transmission.b1");
        const double transmission_rate = (r0 - b0) / b1;
        const double seed = pt_config.get<double>("run.rng_seed");

        for (size_t i = 0; i < m_num_threads; i++) {
                m_contact_handler.emplace_back(ContactHandler(transmission_rate, seed, m_num_threads, i));
        }

        // Initialize contact profiles.
        cerr << "Initializing contact profiles. "<< endl;
	InitializeContactProfiles();
}

const shared_ptr<const Population> Simulator::GetPopulation() const
{
        return m_population;
}

void Simulator::InitializeClusters()
{
	// Determine number of clusters and districts.
	unsigned int num_households         = 0U;
	unsigned int num_day_clusters       = 0U;
	unsigned int num_home_districts     = 0U;
	unsigned int num_day_districts      = 0U;
	Population& population              = *m_population;

	for (const auto& p : population) {
	        num_households      = std::max(num_households,     p.GetClusterId(ClusterType::Household));
	        num_day_clusters    = std::max(num_day_clusters,   p.GetClusterId(ClusterType::Work));
	        num_home_districts  = std::max(num_home_districts, p.GetClusterId(ClusterType::HomeDistrict));
	        num_day_districts   = std::max(num_day_districts,  p.GetClusterId(ClusterType::DayDistrict));
	}

	// Add extra '0' nbh (=not present).
	num_households++;
	num_day_clusters++;
	num_home_districts++;
	num_day_districts++;

	// Keep separate id counter to provide a unique id for every cluster.
	unsigned int cluster_id = 1;

	for (size_t i = 1; i <= num_households; i++) {
		m_households.emplace_back(Cluster(cluster_id, ClusterType::Household));
		cluster_id++;
	}
	for (size_t i = 1; i <= num_day_clusters; i++) {
		// Day clusters are initialized as school clusters. However, when an adult is
	        // added to such a cluster, the cluster type will be changed to "work".
		m_day_clusters.emplace_back(Cluster(cluster_id, ClusterType::School));
		cluster_id++;
	}
	for (size_t i = 1; i <= num_home_districts; i++) {
		m_home_districts.emplace_back(Cluster(cluster_id, ClusterType::HomeDistrict));
		cluster_id++;
	}
	for (size_t i = 1; i <= num_day_districts; i++) {
		m_day_districts.emplace_back(Cluster(cluster_id, ClusterType::DayDistrict));
		cluster_id++;
	}
	for (auto& p: population) {
	        const auto hh_id = p.GetClusterId(ClusterType::Household);
		if (hh_id > 0) {
		        m_households[hh_id].AddPerson(&p);
		}
		const auto wo_id = p.GetClusterId(ClusterType::Work);
		if (wo_id > 0) {
		        m_day_clusters[wo_id].AddPerson(&p);
		}
		const auto hd_id = p.GetClusterId(ClusterType::HomeDistrict);
		if (hd_id > 0) {
		        m_home_districts[hd_id].AddPerson(&p);
		}
		const auto dd_id = p.GetClusterId(ClusterType::DayDistrict);
		if (dd_id > 0) {
		        m_day_districts[dd_id].AddPerson(&p);
		}
	}
	// Set household sizes for persons
	for (auto& p: population) {
	        const auto hh_id = p.GetClusterId(ClusterType::Household);
		if (hh_id > 0) {
			p.SetHouseholdSize(m_households[hh_id].GetSize());
		}
	}
}

void Simulator::InitializeContactProfiles()
{
        // Get the contact configuration to initialize contact matrices for each cluster type
        ptree pt;
        const auto file_name { m_config_pt.get("run.age_contact_matrix_file", "contact_matrix.xml") };
        const auto file_path { InstallDirs::GetDataDir() /= file_name };
        if ( !is_regular_file(file_path) ) {
                throw runtime_error(string(__func__)  + "> No file " + file_path.string());
        }
        read_xml(file_path.string(), pt);

        MasterProfile::AddProfile(ClusterType::Household,     ContactProfile(ClusterType::Household, pt));
        MasterProfile::AddProfile(ClusterType::School,        ContactProfile(ClusterType::School, pt));
        MasterProfile::AddProfile(ClusterType::Work,          ContactProfile(ClusterType::Work, pt));
        MasterProfile::AddProfile(ClusterType::HomeDistrict,  ContactProfile(ClusterType::HomeDistrict, pt));
        MasterProfile::AddProfile(ClusterType::DayDistrict,   ContactProfile(ClusterType::DayDistrict, pt));
}

template<LogMode log_level, bool track_index_case>
void Simulator::UpdateClusters()
{
        #pragma omp parallel
        {
                const unsigned int thread_i = omp_get_thread_num();

                #pragma omp for schedule(runtime)
                for (size_t cluster_i = 0; cluster_i < m_households.size(); cluster_i++) {
                        Infector<log_level, track_index_case>::Execute(
                                m_households[cluster_i],  m_contact_handler[thread_i], m_calendar);
                }
                #pragma omp for schedule(runtime)
                for (size_t cluster_i = 0; cluster_i < m_day_clusters.size(); cluster_i++) {
                        Infector<log_level, track_index_case>::Execute(
                                m_day_clusters[cluster_i], m_contact_handler[thread_i], m_calendar);
                }
                #pragma omp for schedule(runtime)
                for (size_t cluster_i = 0; cluster_i < m_home_districts.size(); cluster_i++) {
                        Infector<log_level, track_index_case>::Execute(
                                m_home_districts[cluster_i], m_contact_handler[thread_i], m_calendar);
                }
                #pragma omp for schedule(runtime)
                for (size_t cluster_i = 0; cluster_i < m_day_districts.size(); cluster_i++) {
                        Infector<log_level, track_index_case>::Execute(
                                m_day_districts[cluster_i], m_contact_handler[thread_i], m_calendar);
                }
        }
}

void Simulator::UpdateTimeStep(bool track_index_case)
{
        for (auto& p : *m_population) {
                p.Update(m_calendar);
        }

        if (track_index_case) {
                switch (m_log_level) {
                        case LogMode::Contacts:
                                UpdateClusters<LogMode::Contacts, true>(); break;
                        case LogMode::Transmissions:
                                UpdateClusters<LogMode::Transmissions, true>(); break;
                        case LogMode::None:
                                UpdateClusters<LogMode::None, true>(); break;
                        default:
                                throw runtime_error(std::string(__func__) + "Log mode screwed up!");
                }
        } else {
                switch (m_log_level) {
                        case LogMode::Contacts:
                                UpdateClusters<LogMode::Contacts, false>(); break;
                        case LogMode::Transmissions:
                                UpdateClusters<LogMode::Transmissions, false>();  break;
                        case LogMode::None:
                                UpdateClusters<LogMode::None, false>(); break;
                        default:
                                throw runtime_error(std::string(__func__) + "Log mode screwed up!");
                }
        }

        m_calendar->AdvanceDay();
}
} // end_of_namespace
