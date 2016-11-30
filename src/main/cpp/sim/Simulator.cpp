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
#include "core/LogMode.h"
#include "core/Population.h"
#include "core/PopulationBuilder.h"

#include <omp.h>
#include <iostream>

namespace indismo {

using namespace std;

Simulator::Simulator(const boost::property_tree::ptree& pt_config)
	: m_num_threads(1U),
	  m_population(std::make_shared<core::Population>())

{
	#pragma omp parallel
	{
		#ifdef _OPENMP
		m_num_threads = omp_get_num_threads();
		#endif
	}

	// initialize world environment
	m_state = std::make_shared<WorldEnvironment>(pt_config);

	// get log level
	m_log_level = pt_config.get<std::string>("run.log_level", "None");

	// R0 and transmission rate
	// use linear model fitted to simulation data: Expected(R0) = (b0+b1*transm_rate)
	const std::string disease_config_file = pt_config.get<std::string>("run.disease_config_file");
	boost::property_tree::ptree pt_disease;
	read_xml(disease_config_file, pt_disease);

	const double r0	  = pt_config.get<double>("run.r0");
	const double b0   = pt_disease.get<double>("disease.transmission.b0");
	const double b1   = pt_disease.get<double>("disease.transmission.b1");
	const double transmission_rate = (r0-b0)/b1;

	for (size_t i = 0; i < m_num_threads; i++) {
		m_contact_handler.push_back(std::make_shared<ContactHandler>(transmission_rate,
				pt_config.get<double>("run.rng_seed"), m_num_threads, i));
	}

	InitializePopulation(pt_config);
	InitializeClusters();

	// Calculate average cluster sizes
	double avg_household_size     = GetAverageClusterSize(m_households);
	double avg_home_district_size = GetAverageClusterSize(m_home_districts);
	double avg_day_cluster_size   = GetAverageClusterSize(m_day_clusters);
	double avg_day_district_size  = GetAverageClusterSize(m_day_districts);

	// Initialize contact matrices for each cluster type
	boost::property_tree::ptree pt_contacts;
	std::string age_contact_matrix_file = pt_config.get("run.age_contact_matrix_file", "./config/contact_matrix.xml");
	read_xml(age_contact_matrix_file, pt_contacts);

	// Household contact matrices
	std::vector<double> household_contact_nums = GetMeanNumbersOfContacts("household", pt_contacts);
	std::vector<double> household_contact_rates = GetContactRates(household_contact_nums, avg_household_size);

	// Home district contact matrices
	std::vector<double> home_district_contact_nums = GetMeanNumbersOfContacts("home_district", pt_contacts);
	std::vector<double> home_district_contact_rates = GetContactRates(home_district_contact_nums, avg_home_district_size);

	// Work contact matrices
	std::vector<double> work_contact_nums = GetMeanNumbersOfContacts("work", pt_contacts);
	std::vector<double> work_contact_rates = GetContactRates(work_contact_nums, avg_day_cluster_size);

	// School contact matrices
	std::vector<double> school_contact_nums = GetMeanNumbersOfContacts("school", pt_contacts);
	std::vector<double> school_contact_rates = GetContactRates(school_contact_nums, avg_day_cluster_size);

	// Day district contact matrices
	std::vector<double> day_district_contact_nums = GetMeanNumbersOfContacts("day_district", pt_contacts);
	std::vector<double> day_district_contact_rates = GetContactRates(day_district_contact_nums, avg_day_district_size);

	for (auto contact_handler : m_contact_handler) {
		contact_handler->addMeanNumsContacts("household", household_contact_nums);
		contact_handler->addContactRates("household", household_contact_rates);

		contact_handler->addMeanNumsContacts("home_district", home_district_contact_nums);
		contact_handler->addContactRates("home_district", home_district_contact_rates);

		// TODO calculate separate average sizes for work and school clusters
		contact_handler->addMeanNumsContacts("work", work_contact_nums);
		contact_handler->addContactRates("work", work_contact_rates);

		contact_handler->addMeanNumsContacts("school", school_contact_nums);
		contact_handler->addContactRates("school", school_contact_rates);

		contact_handler->addMeanNumsContacts("day_district", day_district_contact_nums);
		contact_handler->addContactRates("day_district", day_district_contact_rates);
	}
}

unsigned int Simulator::GetInfectedCount() const
{
	return m_population->GetInfectedCount();
}

unsigned int Simulator::GetPopulationSize() const
{
	return m_population->GetSize();
}

const std::shared_ptr<const core::Population> Simulator::GetPopulation() const
{
	return m_population;
}

std::vector<double> Simulator::GetContactRates(const std::vector<double>& mean_nums, unsigned int avg_cluster_size)
{
	std::vector<double> rates;

	for(double num : mean_nums) {
		rates.push_back(num / avg_cluster_size);
	}

	return rates;
}

std::vector<double> Simulator::GetMeanNumbersOfContacts(std::string cluster_type,  const boost::property_tree::ptree& pt_contacts)
{
	std::string key = "matrices." + cluster_type;
	std::vector<double> meanNums;

	for(auto& participant: pt_contacts.get_child(key)) {
		double total_contacts = 0;
		for (auto& contact: participant.second.get_child("contacts")) {
			total_contacts += contact.second.get<double>("rate");
		}

		meanNums.push_back(total_contacts);
	}

	return meanNums;
}

void Simulator::InitializeClusters()
{
	// get number of clusters and districts
	unsigned int num_households         = 0U;
	unsigned int num_day_clusters       = 0U;
	unsigned int num_home_districts     = 0U;
	unsigned int num_day_districts      = 0U;
	core::Population& population        = *m_population;

	for (auto p : population) {
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
		m_households.push_back(core::Cluster(cluster_id, "household"));
		cluster_id++;
	}

	for (size_t i = 1; i <= num_day_clusters; i++) {
		// Day clusters are initialized as school clusters.
		// However, when a person older than 24 is added to such a cluster,
		// the cluster type will be changed to "work".
		m_day_clusters.push_back(core::Cluster(cluster_id, "school"));
		cluster_id++;
	}

	for (size_t i = 1; i <= num_home_districts; i++) {
		m_home_districts.push_back(core::Cluster(cluster_id, "home_district"));
		cluster_id++;
	}

	for (size_t i = 1; i <= num_day_districts; i++) {
		m_day_districts.push_back(core::Cluster(cluster_id, "day_district"));
		cluster_id++;
	}

	for (size_t i = 0U; i < population.size(); i++) {
		if (population[i].GetHouseholdId() > 0) {
			m_households[population[i].GetHouseholdId()].AddPerson(&population[i]);
		}
		if (population[i].GetDayClusterId() > 0) {
			m_day_clusters[population[i].GetDayClusterId()].AddPerson(&population[i]);
			if (m_day_clusters[population[i].GetDayClusterId()].GetClusterType() == "school") {
				// Check if new person is under 24, otherwise cluster is a workplace
				size_t age = population[i].GetAge();
				if (age > 24U) {
					m_day_clusters[population[i].GetDayClusterId()].SetClusterType("work");
				}
			}
		}
		if (population[i].GetHomeDistrictId() > 0) {
			m_home_districts[population[i].GetHomeDistrictId()].AddPerson(&population[i]);
		}
		if (population[i].GetDayDistrictId() > 0) {
			m_day_districts[population[i].GetDayDistrictId()].AddPerson(&population[i]);
		}
	}

	// Set household sizes for persons
	for (size_t i = 0U; i < population.size(); i++) {
		if (population[i].GetHouseholdId() > 0) {
			size_t hh_size = m_households[population[i].GetHouseholdId()].GetSize();
			population[i].SetHouseholdSize(hh_size);
		}
	}
}

void Simulator::InitializePopulation(const boost::property_tree::ptree& pt_config)
{
	PopulationBuilder::Build(m_population, pt_config);
}

void Simulator::RunTimeStep()
{
		UpdateContacts(m_households);
		UpdateContacts(m_day_clusters);
		UpdateContacts(m_home_districts);
		UpdateContacts(m_day_districts);
		for (auto& p : *m_population) {
			p.Update(m_state);
		}
		m_state->AdvanceDay();
}

double Simulator::GetAverageClusterSize(const std::vector<core::Cluster>& clusters)
{
	double total_size = 0;
	double num_clusters = clusters.size();
	for (size_t i = 1; i < num_clusters; i++) {
		total_size += clusters[i].GetSize();
	}

	return total_size / (num_clusters - 1); // '-1' since we're counting from 1 not 0
}

void Simulator::UpdateContacts(std::vector<core::Cluster>& clusters)
{
	#pragma omp parallel
	{
		unsigned int thread_i = 0U;
		#ifdef _OPENMP
		thread_i = omp_get_thread_num();
		#endif
		#pragma omp for schedule(runtime)
		for (size_t cluster_i = 0; cluster_i < clusters.size(); cluster_i++) {
			if (m_log_level == "Contacts") {
				clusters[cluster_i].Update<LogMode::Contacts>(m_contact_handler[thread_i], m_state);
			} else if (m_log_level == "Transmissions") {
				clusters[cluster_i].Update<LogMode::Transmissions>(m_contact_handler[thread_i], m_state);
			} else {
				clusters[cluster_i].Update<LogMode::None>(m_contact_handler[thread_i], m_state);
			}
		}

	}
}


} // end_of_namespace
