/**
 * @file
 * Configuration data structures for the simulator, built with multi-region in mind.
 */

#include "sim/SimulationConfig.h"

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include "calendar/Calendar.h"
#include "core/LogMode.h"
#include "multiregion/TravelModel.h"
#include "util/Errors.h"
#include "util/InstallDirs.h"
#include "util/TimeStamp.h"

using namespace stride::util;

namespace stride {

CommonSimulationConfig::CommonSimulationConfig()
    : track_index_case(false), rng_seed(), r0(), seeding_rate(), immunity_rate(), number_of_days(),
      disease_config_file_name(), number_of_survey_participants(), initial_calendar(), contact_matrix_file_name()
{
}

void CommonSimulationConfig::Parse(const boost::property_tree::ptree& pt)
{
	rng_seed = pt.get<unsigned int>("rng_seed");
	r0 = pt.get<double>("r0");
	seeding_rate = pt.get<double>("seeding_rate");
	immunity_rate = pt.get<double>("immunity_rate");
	number_of_days = pt.get<unsigned int>("num_days");
	disease_config_file_name = pt.get<std::string>("disease_config_file");
	number_of_survey_participants = pt.get<unsigned int>("num_participants_survey", 1);

	auto start_date_string = pt.get<std::string>("start_date", "2016-01-01");
	auto file_name = pt.get<std::string>("holidays_file", "holidays_flanders_2016.json");
	auto start_date = boost::gregorian::from_simple_string(start_date_string);
	initial_calendar.Initialize(start_date, file_name);

	contact_matrix_file_name = pt.get<std::string>("age_contact_matrix_file", "contact_matrix.xml");
}

LogConfig::LogConfig() : output_prefix(), generate_person_file(), log_level() {}

void LogConfig::Parse(const boost::property_tree::ptree& pt)
{
	output_prefix = pt.get<std::string>("output_prefix", "");
	generate_person_file = pt.get<double>("generate_person_file", 0) == 1;
	auto log_level_string = pt.get<std::string>("log_level", "None");
	log_level = IsLogMode(log_level_string)
			? ToLogMode(log_level_string)
			: throw std::runtime_error(std::string(__func__) + "> Invalid input for LogMode.");
}

void SingleSimulationConfig::Parse(const boost::property_tree::ptree& pt)
{
	MultiSimulationConfig multi_config;
	multi_config.Parse(pt);
	*this = multi_config.AsSingleConfig();
}

MultiSimulationConfig SingleSimulationConfig::AsMultiConfig() const
{
	return {common_config, log_config, {travel_model}};
}

void MultiSimulationConfig::Parse(const boost::property_tree::ptree& pt)
{
	common_config = std::make_shared<CommonSimulationConfig>();
	common_config->Parse(pt);
	log_config = std::make_shared<LogConfig>();
	log_config->Parse(pt);
	region_models.clear();
	for (const auto& item : pt) {
		if (item.first == "population_file") {
			region_models.push_back(
			    std::make_shared<stride::multiregion::RegionTravel>(
				region_models.size(), item.second.get_value<std::string>(),
				pt.get<std::string>("geodistribution_profile", "")));
		} else if (item.first == "travel_model") {
			auto parsed_region_models =
			    stride::multiregion::RegionTravel::ParseRegionTravel(item.second, region_models.size());
			region_models.insert(
			    region_models.end(), parsed_region_models.begin(), parsed_region_models.end());
		} else if (item.first == "travel_file") {
			const auto file_name = item.second.get_value<std::string>();
			boost::property_tree::ptree pt;
			InstallDirs::ReadXmlFile(file_name, InstallDirs::GetDataDir(), pt);
			auto parsed_region_models = stride::multiregion::RegionTravel::ParseRegionTravel(
			    pt.get_child("travel_model"), region_models.size());
			region_models.insert(
			    region_models.end(), parsed_region_models.begin(), parsed_region_models.end());
		}
	}
}

std::vector<SingleSimulationConfig> MultiSimulationConfig::GetSingleConfigs() const
{
	std::vector<SingleSimulationConfig> results;
	for (const auto& travel_model : region_models) {
		SingleSimulationConfig config;
		config.common_config = common_config;
		config.log_config = log_config;
		config.travel_model = travel_model;
		results.push_back(config);
	}
	return results;
}

SingleSimulationConfig MultiSimulationConfig::AsSingleConfig() const
{
	if (region_models.size() != 1) {
		throw std::runtime_error(
		    std::string(__func__) +
		    "> Could not reduce a multi-simulation configuration to a single-region simulation configuration.");
	}

	return {common_config, log_config, region_models[0]};
}

} // namespace