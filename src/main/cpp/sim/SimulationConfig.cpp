/**
 * @file
 * Configuration data structures for the simulator, built with multi-region in mind.
 */

#include <memory>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "calendar/Calendar.h"
#include "core/LogMode.h"
#include "sim/SimulationConfig.h"
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

LogConfig::LogConfig() : output_prefix(), generate_person_file(), log_level()
{
}

void LogConfig::Parse(const boost::property_tree::ptree& pt)
{
	output_prefix = pt.get<std::string>("output_prefix", "");
	if (output_prefix.length() == 0) {
		output_prefix = TimeStamp().ToTag();
	}
	generate_person_file = pt.get<double>("generate_person_file") == 1;
	auto log_level_string = pt.get<std::string>("log_level", "None");
	log_level = IsLogMode(log_level_string)
			? ToLogMode(log_level_string)
			: throw std::runtime_error(std::string(__func__) + "> Invalid input for LogMode.");
}

void MultiSimulationConfig::Parse(const boost::property_tree::ptree& pt)
{
	common_config = std::make_shared<CommonSimulationConfig>();
	common_config->Parse(pt);
	log_config = std::make_shared<LogConfig>();
	log_config->Parse(pt);
}

std::vector<SingleSimulationConfig> MultiSimulationConfig::get_single_configs() const
{
	std::vector<SingleSimulationConfig> results;
	for (const auto& file_name : population_file_names) {
		SingleSimulationConfig config;
		config.common_config = common_config;
		config.population_file_name = file_name;
		results.push_back(config);
	}
	return results;
}

} // namespace