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

#include "core/Health.h"
#include "core/Person.h"
#include "core/Population.h"
#include "util/InstallDirs.h"
#include "util/Random.h"
#include "util/StringUtils.h"

#include <spdlog/spdlog.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace stride::util;

bool PopulationBuilder::Build(shared_ptr<Population> pop,
                        const boost::property_tree::ptree& pt_config,
                        const boost::property_tree::ptree& pt_disease)
{
        //------------------------------------------------
        // setup.
        //------------------------------------------------
        Population& population            = *pop;
        const double seeding_rate         = pt_config.get<double>("run.seeding_rate");
        const double immunity_rate        = pt_config.get<double>("run.immunity_rate");
        const unsigned int rng_seed       = pt_config.get<double>("run.rng_seed");
        const string disease_config_file  = pt_config.get<string>("run.disease_config_file");

        //------------------------------------------------
        // Check input.
        //------------------------------------------------
        bool status = (seeding_rate <= 1) && (immunity_rate <= 1) && ((seeding_rate + immunity_rate) <= 1);

        //------------------------------------------------
        // Add persons to population.
        //------------------------------------------------
        if (status) {
                status = false;
                const auto file_name = pt_config.get<string>("run.population_file");;
                const auto file_path = InstallDirs::GetDataDir() /= file_name;
                if ( !is_regular_file(file_path) ) {
                        throw runtime_error(string(__func__)
                                + "> Population file " + file_path.string() + " not present. Aborting.");
                }

                ifstream pop_file;
                pop_file.open(file_path.string());
                if ( !pop_file.is_open()) {
                        throw runtime_error(string(__func__)
                                + "> Error opening population file " + file_path.string() + ". Aborting.");
                }

                const vector<double> start_infectiousness = GetDistribution(pt_disease,"disease.start_infectiousness");
                const vector<double> start_symptomatic    = GetDistribution(pt_disease,"disease.start_symptomatic");
                const vector<double> time_infectious      = GetDistribution(pt_disease,"disease.time_infectious");
                const vector<double> time_symptomatic     = GetDistribution(pt_disease,"disease.time_symptomatic");
                util::Random rng_disease(rng_seed); // random numbers for disease characteristics

                string line;
                getline(pop_file, line); // step over file header
                unsigned int person_id = 0U;
                while (getline(pop_file, line)) {
                        //make use of stochastic disease characteristics
                        const auto person_start_infectiousness = SampleFromDistribution(rng_disease, start_infectiousness);
                        const auto person_start_symptomatic    = SampleFromDistribution(rng_disease, start_symptomatic);
                        const auto person_time_infectious      = SampleFromDistribution(rng_disease, time_infectious);
                        const auto person_time_symptomatic     = SampleFromDistribution(rng_disease, time_symptomatic);

                        const auto values = util::StringUtils::Tokenize(line, ";");
                        population.emplace_back(Person(person_id,
                                util::StringUtils::FromString<unsigned int>(values[0]),
                                util::StringUtils::FromString<unsigned int>(values[1]),
                                util::StringUtils::FromString<unsigned int>(values[3]),
                                util::StringUtils::FromString<unsigned int>(values[2]),
                                util::StringUtils::FromString<unsigned int>(values[4]),
                                person_start_infectiousness, person_start_symptomatic,
                                person_time_infectious, person_time_symptomatic));
                        ++person_id;
                }
                pop_file.close();
                status = true;
        }

        //------------------------------------------------
        // Customize the population.
        //------------------------------------------------
        if (status) {

                //------------------------------------------------
                // Set up.
                //------------------------------------------------
                util::Random rng(rng_seed);
                const unsigned int max_population_index = population.size() - 1;

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
                unsigned int num_immune = floor(static_cast<double> (population.size()) * immunity_rate);
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
        }

        //------------------------------------------------
        // Done
        //------------------------------------------------
        return status;
}


vector<double> PopulationBuilder::GetDistribution(const boost::property_tree::ptree& pt_root, const string& xml_tag)
{
        vector<double> values;
        boost::property_tree::ptree subtree = pt_root.get_child(xml_tag);
        for(const auto& tree : subtree) {
                values.push_back(tree.second.get<double>(""));
        }
        return values;
}

unsigned int PopulationBuilder::SampleFromDistribution(util::Random& rng, const vector<double>& distribution)
{
        double random_value = rng.NextDouble();
        for(unsigned int i = 0; i < distribution.size(); i++) {
                if (random_value <= distribution[i]) {
                        return i;
                }
        }
        cerr << "WARNING: PROBLEM WITH DISEASE DISTRIBUTION [PopulationBuilder]" << endl;
        return distribution.size();
}

} // end_of_namespace
