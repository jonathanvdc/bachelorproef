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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S, Broeckhove J
 *  Aerts S, De Haes C, Van der Cruysse J & Van Hauwe L
 */

/**
 * @file
 * Initialize populations.
 */

#include "PopulationBuilder.h"

#include "core/Disease.h"
#include "core/Health.h"
#include "geo/Profile.h"
#include "pop/Person.h"
#include "pop/Population.h"
#include "pop/PopulationGenerator.h"
#include "pop/PopulationModel.h"
#include "util/Errors.h"
#include "util/InstallDirs.h"
#include "util/Random.h"
#include "util/StringUtils.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <spdlog/spdlog.h>

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
    const SingleSimulationConfig& config, const boost::property_tree::ptree& pt_disease, util::Random& rng,
    const std::shared_ptr<spdlog::logger>& log)
{
	// Setup.
	const auto pop = make_shared<Population>();
	Population& population = *pop;
	const double seeding_rate = config.common_config->seeding_rate;
	const double immunity_rate = config.common_config->immunity_rate;
	const string disease_config_file = config.common_config->disease_config_file_name;

	const auto disease = Disease::Parse(pt_disease);

	// Check input.
	bool status = (seeding_rate <= 1) && (immunity_rate <= 1) && ((seeding_rate + immunity_rate) <= 1);
	if (!status) {
		FATAL_ERROR("Bad input data.");
	}

	// Add persons to population.
	const auto pop_file_name = config.GetPopulationPath();
	const auto pop_file_path = InstallDirs::GetDataDir() /= pop_file_name;
	if (!is_regular_file(pop_file_path)) {
		FATAL_ERROR("Population file " + pop_file_path.string() + " not present.");
	}

	boost::filesystem::ifstream pop_file;
	pop_file.open(pop_file_path.string());
	if (!pop_file.is_open()) {
		FATAL_ERROR("Error opening population file " + pop_file_path.string());
	}

	if (boost::algorithm::ends_with(pop_file_name, ".csv")) {
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
			    disease->Sample(rng)));			      // Fate
			++person_id;
		}
	} else if (boost::algorithm::ends_with(pop_file_name, ".xml")) {
		// Read geodistribution profile.
		const auto geo_file_name = config.GetGeodistributionProfilePath();
		const auto geo_file_path = InstallDirs::GetDataDir() /= geo_file_name;
		if (!is_regular_file(geo_file_path)) {
			FATAL_ERROR("Geodistribution profile " + geo_file_path.string() + " not present.");
		}

		boost::filesystem::ifstream geo_file;
		geo_file.open(geo_file_path.string());
		if (!geo_file.is_open()) {
			FATAL_ERROR("Error opening geodistribution profile " + geo_file_path.string());
		}

		const geo::ProfileRef geo_profile = geo::Profile::Parse(geo_file);
		geo_file.close();

		// Read population model file.
		boost::property_tree::ptree pt;
		read_xml(pop_file, pt);
		population_model::Model model;
		model.parse(pt);

		// Generate population.
		population_model::Generator generator(model, geo_profile, *disease, rng);
		population = generator.Generate();

		if (!generator.FitsModel(population, true)) {
			FATAL_ERROR("Generated population doesn't fit model " + pop_file_name);
		}
	} else {
		FATAL_ERROR(
		    "Population file " + pop_file_path.string() +
		    " must be CSV (population data file) or XML (population model file).");
	}

	pop_file.close();

	const unsigned int max_population_index = population.size() - 1;
	if (population.size() <= 2U) {
		FATAL_ERROR("Population is too small.");
	}

	// Set participants in social contact survey.
	const auto log_level = config.log_config->log_level;
	if (log_level == LogMode::Contacts) {
		const unsigned int num_participants = config.common_config->number_of_survey_participants;

		// use a while-loop to obtain 'num_participant' unique participants (default sampling is with
		// replacement)
		// A for loop will not do because we might draw the same person twice.
		unsigned int num_samples = 0;
		while (num_samples < num_participants) {
			Person& p = population[rng(max_population_index)];
			if (!p.IsParticipatingInSurvey()) {
				p.ParticipateInSurvey();
				log->info("[PART] {} {} {}", p.GetId(), p.GetAge(), p.GetGender());
				num_samples++;
			}
		}
	}

	// Set population immunity.
	unsigned int num_immune = floor(static_cast<double>(population.size()) * immunity_rate);
	while (num_immune > 0) {
		Person& p = population[rng(max_population_index)];
		if (p.GetHealth().IsSusceptible()) {
			p.GetHealth().SetImmune();
			num_immune--;
		}
	}

	// Seed infected persons.
	unsigned int num_infected = floor(static_cast<double>(population.size()) * seeding_rate);
	while (num_infected > 0) {
		Person& p = population[rng(max_population_index)];
		if (p.GetHealth().IsSusceptible()) {
			p.GetHealth().StartInfection();
			num_infected--;
		}
	}

	// Done
	return pop;
}

} // end_of_namespace
