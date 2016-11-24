#ifndef POPULATION_BUILDER_H_INCLUDED
#define POPULATION_BUILDER_H_INCLUDED
/*
 *  This file is part of the indismo software.
 *  It is free software: you can redistribute it and/or modify it
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
 *  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and
 *  Broeckhove J. (2015) Optimizing agent-based transmission models for
 *  infectious diseases, BMC Bioinformatics.
 *
 *  Copyright 2015, Willem L, Stijven S & Broeckhove J
 */
/**
 * @file
 * Initialize populations.
 */

#include "core/Population.h"
#include "util/StringUtils.h"
#include "util/Random.h"

#include "spdlog/spdlog.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace indismo {

/**
 * Initializes Population objects.
 */
class PopulationBuilder
{
public:
	/**
	 * Constructor that initializes a Population:
	 * - Add persons
	 * - Set population immunity
	 * - Set the first infected cases
	 *
	 * @param population		    Pointer to the initialized population.
	 * @param pt_config				The property_tree with configuration settings.
	 */
	static bool Build(std::shared_ptr<core::Population> population,
			const boost::property_tree::ptree& pt_config)
	{
		bool status = true;

		const std::string population_file = pt_config.get<std::string>("run.population_file");
		const double seeding_rate         = pt_config.get<double>("run.seeding_rate");
		const double immunity_rate        = pt_config.get<double>("run.immunity_rate");
		const unsigned int rng_seed       = pt_config.get<double>("run.rng_seed");

		const std::string disease_config_file = pt_config.get<std::string>("run.disease_config_file");
		boost::property_tree::ptree pt_disease;
		read_xml(disease_config_file, pt_disease);

		std::vector<double> start_infectiousness = GetDistribution(pt_disease,"disease.start_infectiousness");
		std::vector<double> start_symptomatic    = GetDistribution(pt_disease,"disease.start_symptomatic");
		std::vector<double> time_infectious      = GetDistribution(pt_disease,"disease.time_infectious");
		std::vector<double> time_symptomatic     = GetDistribution(pt_disease,"disease.time_symptomatic");

		//------------------------------------------------
		// Check input.
		//------------------------------------------------
		if(seeding_rate > 1 || immunity_rate > 1 || seeding_rate + immunity_rate > 1){
			status = false;
		}

		if(seeding_rate > 1 || immunity_rate > 1 || seeding_rate + immunity_rate > 1){
					status = false;
		}

		//------------------------------------------------
		// Add persons to population.
		//------------------------------------------------
		if(status) {
			status = false;
			std::ifstream popFile;
			popFile.open(population_file);
			if (popFile.is_open()) {
				unsigned int person_start_infectiousness, person_start_symptomatic,
					person_time_infectious, person_time_symptomatic;
				util::Random rng_disease(rng_seed); // random numbers for disease characteristics

				std::string line;
				std::getline(popFile, line); // step over file header
				unsigned int person_index = 0U;
				while (std::getline(popFile, line)) {

					//make use of stochastic disease characteristics
					person_start_infectiousness = SampleFromDistribution(rng_disease,start_infectiousness);
					person_start_symptomatic    = SampleFromDistribution(rng_disease,start_symptomatic);
					person_time_infectious      = SampleFromDistribution(rng_disease,time_infectious);
					person_time_symptomatic     = SampleFromDistribution(rng_disease,time_symptomatic);

					const auto values = util::StringUtils::Tokenize(line, ";");
					population->AddPerson(person_index,
							util::StringUtils::FromString<unsigned int>(values[0]),
							util::StringUtils::FromString<unsigned int>(values[1]),
							util::StringUtils::FromString<unsigned int>(values[3]),
							util::StringUtils::FromString<unsigned int>(values[2]),
							util::StringUtils::FromString<unsigned int>(values[4]),
							person_start_infectiousness, person_start_symptomatic,
							person_time_infectious, person_time_symptomatic);
					++person_index;
				}
				popFile.close();
				status = true;
			}
			else {
				status = false;
				std::cerr << "PROBLEM WITH POPULATION FILE: " << population_file << std::endl;
			}
		}

		//------------------------------------------------
		// Set participants in social contact survey. (OPTION)
		//------------------------------------------------
		std::string log_level = pt_config.get<std::string>("run.log_level", "None");
		if((log_level == "Contacts") & status)
		{
			unsigned int pop_size = population->GetSize();
			unsigned int num_participants = pt_config.get<double>("run.num_participants_survey");
			util::Random rng_survey(rng_seed);

			// use a while-loop to obtain 'num_participant' unique participants (default sampling is with replacement)
			unsigned int num_samples = 0;
			std::shared_ptr<spdlog::logger> logger = spdlog::get("contact_logger");
			while(num_samples < num_participants){
				unsigned int participant_id = rng_survey(pop_size);
				if(population->GetPerson(participant_id).IsParticipatingInSurvey()==false){
					population->SetParticipant(participant_id,logger);
					num_samples++;
				}
			}

		}


		//------------------------------------------------
		// Set population immunity.
		//------------------------------------------------
		if (status) {
			unsigned int population_size = population->GetSize();
			unsigned int num_immune = floor(
					static_cast<double> (population_size) * immunity_rate);
			unsigned int max_population_index = population_size - 1;
			unsigned int population_index = 0;
			util::Random rng(rng_seed+1);    //TODO: other options?!
			while (num_immune > 0) {
				population_index = rng(max_population_index);
				if((*population).GetPerson(population_index).IsSusceptible())
				{
					(*population).SetImmune(population_index);
					num_immune--;
				}
			}
		}

		//------------------------------------------------
		// Seed infected persons.
		//------------------------------------------------
		if (status) {
			unsigned int population_size = population->GetSize();
			unsigned int num_infected = floor(
					static_cast<double> (population_size) * seeding_rate);
			unsigned int max_population_index = population_size - 1;
			unsigned int population_index = 0;
			util::Random rng(rng_seed);

//			// TEMPORARY FEATURE: sample in a certain district
//			unsigned int seed_district = 95; // 95: Antwerp and 137: Hasselt
//			std::cout << "infectious seeding district: " << seed_district << std::endl;
			while (num_infected > 0) {
				//for (unsigned int i = 0; i < num_infected; ++i) {
				population_index = rng(max_population_index);
				if((*population).GetPerson(population_index).IsSusceptible())
				{
//					// TEMPORARY FEATURE: sample in a certain district
//					if(((*population).GetPerson(population_index).GetHomeDistrictId()== seed_district || (*population).GetPerson(population_index).GetDayDistrictId() == seed_district) &&
//					   ((*population).GetPerson(population_index).GetAge()>20 && ((*population).GetPerson(population_index).GetAge()<40))){
						(*population).SetIndexCase(population_index);
						num_infected--;
//					}
				}
			}
		}

		//------------------------------------------------
		// Done
		//------------------------------------------------
		return status;
	}

private:
	static std::vector<double> GetDistribution(boost::property_tree::ptree& pt_root, std::string xml_tag)
	{
		std::vector<double> values;
		boost::property_tree::ptree subtree = pt_root.get_child(xml_tag);
		for(const auto& tree : subtree) {
			values.push_back(tree.second.get<double>(""));
		}
		return values;
	}

	static unsigned int SampleFromDistribution(util::Random& rng, const std::vector<double>& distribution)
		{
			double random_value = rng.NextDouble();
			for(unsigned int i = 0; i < distribution.size(); i++) {
				if(random_value <= distribution[i]) {
					return i;
				}
			}
			std::cerr << "WARNING: PROBLEM WITH DISEASE DISTRIBUTION [PopulationBuilder]" << std::endl;
			return distribution.size();
		}

};

} // end_of_namespace

#endif // include guard
