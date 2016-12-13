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

#include "WorldEnvironment.h"
#include "core/ClusterType.h"
#include "core/LogMode.h"
#include "core/Population.h"
#include "core/PopulationBuilder.h"
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

Simulator::Simulator(const boost::property_tree::ptree& pt_config)
	: m_config_pt(pt_config), m_num_threads(1U), m_population(make_shared<Population>())
{
	#pragma omp parallel
	{
		#ifdef _OPENMP
		m_num_threads = omp_get_num_threads();
		#endif
	}

	// initialize world environment
	m_state = make_shared<WorldEnvironment>(pt_config);

	// get log level
	const string l = pt_config.get<string>("run.log_level", "None");
	m_log_level = IsLogMode(l) ?
	                ToLogMode(l)
	                : throw runtime_error(std::string(__func__) + "> Invalid input for LogMode");

	// Get the disease configuration
	ptree pt_disease;
	{
	        const auto file_name { pt_config.get<string>("run.disease_config_file") };
	        const auto file_path { InstallDirs::GetConfigDir() /= file_name };
                if ( !is_regular_file(file_path) ) {
                        throw runtime_error(std::string(__func__)
                                + ">Disease file " + file_path.string() + " not present. Aborting.");
                }
	        read_xml(file_path.string(), pt_disease);
	}

	// Build population and initialize clusters.
	PopulationBuilder::Build(m_population, pt_config, pt_disease);
	InitializeClusters();

	// Build contact handlers.
        // R0 and transmission rate;use linear model fitted to simulation data: Expected(R0) = (b0+b1*transm_rate)
        const double r0   = pt_config.get<double>("run.r0");
        const double b0   = pt_disease.get<double>("disease.transmission.b0");
        const double b1   = pt_disease.get<double>("disease.transmission.b1");
        const double transmission_rate = (r0-b0)/b1;
        for (size_t i = 0; i < m_num_threads; i++) {
                m_contact_handler.emplace_back(make_shared<ContactHandler>(transmission_rate,
                                pt_config.get<double>("run.rng_seed"), m_num_threads, i));
        }
	InitializeContactHandlers();
}


double Simulator::GetAverageClusterSize(const vector<Cluster>& clusters)
{
        double total_size = 0;
        const double num_clusters = clusters.size();
        for (size_t i = 1; i < num_clusters; i++) {
                total_size += clusters[i].GetSize();
        }
        return total_size / (num_clusters - 1); // '-1' since we're counting from 1 not 0
}

vector<double> Simulator::GetContactRates(const vector<double>& mean_nums, unsigned int avg_cluster_size)
{
        vector<double> rates;
        for(double num : mean_nums) {
                rates.emplace_back(num / avg_cluster_size);
        }
        return rates;
}

vector<double> Simulator::GetMeanNumbersOfContacts(ClusterType c_type,  const boost::property_tree::ptree& pt_contacts)
{
        const string key = "matrices." + ToString(c_type);
        vector<double> meanNums;
        for(const auto& participant: pt_contacts.get_child(key)) {
                double total_contacts = 0;
                for (const auto& contact: participant.second.get_child("contacts")) {
                        total_contacts += contact.second.get<double>("rate");
                }
                meanNums.push_back(total_contacts);
        }
        return meanNums;
}

const shared_ptr<const Population> Simulator::GetPopulation() const
{
        return m_population;
}

void Simulator::InitializeClusters()
{
	// get number of clusters and districts
	unsigned int num_households         = 0U;
	unsigned int num_day_clusters       = 0U;
	unsigned int num_home_districts     = 0U;
	unsigned int num_day_districts      = 0U;
	Population& population              = *m_population;

	for (const auto& p : population) {
		if (num_households < p.GetHouseholdId()) {
			num_households = p.GetHouseholdId();
		}
		if (num_day_clusters < p.GetDayClusterId()) {
			num_day_clusters = p.GetDayClusterId();
		}
		if (num_home_districts < p.GetHomeDistrictId()) {
			num_home_districts = p.GetHomeDistrictId();
		}
		if (num_day_districts < p.GetDayDistrictId()) {
			num_day_districts = p.GetDayDistrictId();
		}
	}

	// add extra '0' nbh (=not present)
	num_households++;
	num_day_clusters++;
	num_home_districts++;
	num_day_districts++;

	// keep separate id counter to provide a unique id for every cluster
	unsigned int cluster_id = 1;

	for (size_t i = 1; i <= num_households; i++) {
		m_households.emplace_back(Cluster(cluster_id, ClusterType::Household));
		cluster_id++;
	}
	for (size_t i = 1; i <= num_day_clusters; i++) {
		// Day clusters are initialized as school clusters.
		// However, when a person older than 24 is added to such a cluster,
		// the cluster type will be changed to "work".
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
	for (auto p: population) {
		if (p.GetHouseholdId() > 0) {
			m_households[p.GetHouseholdId()].AddPerson(&p);
		}
		if (p.GetDayClusterId() > 0) {
			m_day_clusters[p.GetDayClusterId()].AddPerson(&p);
			if (m_day_clusters[p.GetDayClusterId()].GetClusterType() == ClusterType::School) {
				// Check if new person is under 24, otherwise cluster is a workplace
				if (p.GetAge() > 24U) {
					m_day_clusters[p.GetDayClusterId()].SetClusterType(ClusterType::Work);
				}
			}
		}
		if (p.GetHomeDistrictId() > 0) {
			m_home_districts[p.GetHomeDistrictId()].AddPerson(&p);
		}
		if (p.GetDayDistrictId() > 0) {
			m_day_districts[p.GetDayDistrictId()].AddPerson(&p);
		}
	}
	// Set household sizes for persons
	for (auto& p: population) {
		if (p.GetHouseholdId() > 0) {
			size_t hh_size = m_households[p.GetHouseholdId()].GetSize();
			p.SetHouseholdSize(hh_size);
		}
	}
}

void Simulator::InitializeContactHandlers()
{
        // Get the contact configuration to initialize contact matrices for each cluster type
        ptree pt_contacts;
        {
                const auto file_name { m_config_pt.get("run.age_contact_matrix_file", "contact_matrix.xml") };
                const auto file_path { InstallDirs::GetConfigDir() /= file_name };
                if ( !is_regular_file(file_path) ) {
                        throw runtime_error(std::string(__func__)
                                + "> Contact file " + file_path.string() + " not present. Aborting.");
                }
                read_xml(file_path.string(), pt_contacts);
        }

        // Calculate average cluster sizes
        const double avg_household_size     = GetAverageClusterSize(m_households);
        const double avg_home_district_size = GetAverageClusterSize(m_home_districts);
        const double avg_day_cluster_size   = GetAverageClusterSize(m_day_clusters);
        const double avg_day_district_size  = GetAverageClusterSize(m_day_districts);

        // Household contact matrices
        const vector<double> household_contact_nums = GetMeanNumbersOfContacts(ClusterType::Household, pt_contacts);
        const vector<double> household_contact_rates = GetContactRates(household_contact_nums, avg_household_size);

        // Home district contact matrices
        const vector<double> home_district_contact_nums = GetMeanNumbersOfContacts(ClusterType::HomeDistrict, pt_contacts);
        const vector<double> home_district_contact_rates = GetContactRates(home_district_contact_nums, avg_home_district_size);

        // Work contact matrices
        const vector<double> work_contact_nums = GetMeanNumbersOfContacts(ClusterType::Work, pt_contacts);
        const vector<double> work_contact_rates = GetContactRates(work_contact_nums, avg_day_cluster_size);

        // School contact matrices
        const vector<double> school_contact_nums = GetMeanNumbersOfContacts(ClusterType::School, pt_contacts);
        const vector<double> school_contact_rates = GetContactRates(school_contact_nums, avg_day_cluster_size);

        // Day district contact matrices
        const vector<double> day_district_contact_nums = GetMeanNumbersOfContacts(ClusterType::DayDistrict, pt_contacts);
        const vector<double> day_district_contact_rates = GetContactRates(day_district_contact_nums, avg_day_district_size);

        for (auto contact_handler : m_contact_handler) {
                contact_handler->AddMeanNumsContacts(ClusterType::Household, household_contact_nums);
                contact_handler->AddContactRates(ClusterType::Household, household_contact_rates);

                contact_handler->AddMeanNumsContacts(ClusterType::HomeDistrict, home_district_contact_nums);
                contact_handler->AddContactRates(ClusterType::HomeDistrict, home_district_contact_rates);

                // TODO calculate separate average sizes for work and school clusters
                contact_handler->AddMeanNumsContacts(ClusterType::Work, work_contact_nums);
                contact_handler->AddContactRates(ClusterType::Work, work_contact_rates);

                contact_handler->AddMeanNumsContacts(ClusterType::School, school_contact_nums);
                contact_handler->AddContactRates(ClusterType::School, school_contact_rates);

                contact_handler->AddMeanNumsContacts(ClusterType::DayDistrict, day_district_contact_nums);
                contact_handler->AddContactRates(ClusterType::DayDistrict, day_district_contact_rates);
        }
}

void Simulator::RunTimeStep(bool track_index_case)
{
        for (auto& p : *m_population) {
                p.Update(m_state);
        }

	UpdateCluster(m_households, track_index_case);
        UpdateCluster(m_day_clusters, track_index_case);
        UpdateCluster(m_home_districts, track_index_case);
        UpdateCluster(m_day_districts, track_index_case);

        m_state->AdvanceDay();
}

void Simulator::UpdateCluster(vector<Cluster>& clusters, bool track_index_case)
{
	#pragma omp parallel
	{
	        const unsigned int thread_i = omp_get_thread_num();
		#pragma omp for schedule(runtime)
		for (size_t cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
		        clusters[cluster_i].Update(m_contact_handler[thread_i], m_state, m_log_level, track_index_case);
		}
	}
}


} // end_of_namespace
