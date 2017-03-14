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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Initialize populations.
 */

#include "PopulationBuilder.h"

#include "core/Disease.h"
#include "core/Health.h"
#include "pop/Person.h"
#include "pop/Population.h"
#include "pop/PopulationModel.h"
#include "util/InstallDirs.h"
#include "util/Random.h"
#include "util/StringUtils.h"

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::disease;
using namespace stride::util;

shared_ptr<Population> PopulationBuilder::Build(
        const boost::property_tree::ptree& pt_config,
        const boost::property_tree::ptree& pt_disease,
        util::Random& rng)
{
        //------------------------------------------------
        // Setup.
        //------------------------------------------------
        const auto pop                    = make_shared<Population>();
        Population& population            = *pop;
        const double seeding_rate         = pt_config.get<double>("run.seeding_rate");
        const double immunity_rate        = pt_config.get<double>("run.immunity_rate");
        const string disease_config_file  = pt_config.get<string>("run.disease_config_file");

        const auto disease = Disease::Parse(pt_disease);

        //------------------------------------------------
        // Check input.
        //------------------------------------------------
        bool status = (seeding_rate <= 1) && (immunity_rate <= 1) && ((seeding_rate + immunity_rate) <= 1);
        if ( !status ) {
                throw runtime_error(string(__func__) + "> Bad input data.");
        }

        //------------------------------------------------
        // Add persons to population.
        //------------------------------------------------
        const auto file_name = pt_config.get<string>("run.population_file");;
        const auto file_path = InstallDirs::GetDataDir() /= file_name;
        if ( !is_regular_file(file_path) ) {
                throw runtime_error(string(__func__)
                        + "> Population file " + file_path.string() + " not present.");
        }

        boost::filesystem::ifstream pop_file;
        pop_file.open(file_path.string());
        if ( !pop_file.is_open()) {
                throw runtime_error(string(__func__)
                        + "> Error opening population file " + file_path.string());
        }

        if (boost::algorithm::ends_with(file_name, ".csv")) {
                // Read population data file.
                string line;
                getline(pop_file, line); // step over file header
                unsigned int person_id = 0U;
                while (getline(pop_file, line)) {
                        const auto values = StringUtils::Split(line, ",");
                        population.emplace_back(Person(
                                person_id,
                                StringUtils::FromString<unsigned int>(values[0]), // age
                                StringUtils::FromString<unsigned int>(values[1]), // household_id
                                StringUtils::FromString<unsigned int>(values[2]), // school_id
                                StringUtils::FromString<unsigned int>(values[3]), // work_id
                                StringUtils::FromString<unsigned int>(values[4]), // primary_community_id
                                StringUtils::FromString<unsigned int>(values[5]), // secondary_community_id
                                disease->Sample(rng))); // Fate
                        ++person_id;
                }
        } else if (boost::algorithm::ends_with(file_name, ".xml")) {
                // Read population model file.
                using boost::property_tree::ptree;
                ptree pt;
                read_xml(pop_file, pt);
                population_model::Model model;
                model.parse(pt);

                // Example:
                std::cout << "child_maximum_age_gap: " << model.household.age_gap.child.maximum << std::endl;
                // TODO: generate `population` from `model`.

        } else {
                throw runtime_error(string(__func__)
                        + "> Population file " + file_path.string() + " must be CSV (population"
                          "data file) or XML (population model file).");
        }



        pop_file.close();

        //------------------------------------------------
        // Customize the population.
        //------------------------------------------------

        const unsigned int max_population_index = population.size() - 1;
        if ( max_population_index <= 1U) {
                throw runtime_error(string(__func__) + "> Problem with population size.");
        }
        //------------------------------------------------
        // Set participants in social contact survey.
        //------------------------------------------------
        const string log_level = pt_config.get<string>("run.log_level", "None");
        if ( log_level == "Contacts" ) {
                const unsigned int num_participants = pt_config.get<double>("run.num_participants_survey");

                // use a while-loop to obtain 'num_participant' unique participants (default sampling is with replacement)
                // A for loop will not do because we might draw the same person twice.
                unsigned int num_samples = 0;
                const shared_ptr<spdlog::logger> logger = spdlog::get("contact_logger");
                while(num_samples < num_participants){
                        Person& p = population[rng(max_population_index)];
                        if ( !p.IsParticipatingInSurvey() ) {
                                p.ParticipateInSurvey();
                                logger->info("[PART] {} {} {}", p.GetId(), p.GetAge(), p.GetGender());
                                num_samples++;
                        }
                }
        }

        //------------------------------------------------
        // Set population immunity.
        //------------------------------------------------
        unsigned int num_immune = floor(static_cast<double>(population.size()) * immunity_rate);
        while (num_immune > 0) {
                Person& p = population[rng(max_population_index)];
                if (p.GetHealth().IsSusceptible()) {
                        p.GetHealth().SetImmune();
                        num_immune--;
                }
        }

        //------------------------------------------------
        // Seed infected persons.
        //------------------------------------------------
        unsigned int num_infected = floor(static_cast<double> (population.size()) * seeding_rate);
        while (num_infected > 0) {
                Person& p = population[rng(max_population_index)];
                if (p.GetHealth().IsSusceptible()) {
                        p.GetHealth().StartInfection();
                        num_infected--;
                }
        }

        //------------------------------------------------
        // Done
        //------------------------------------------------
        return pop;
}

} // end_of_namespace
