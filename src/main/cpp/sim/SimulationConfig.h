#ifndef SIM_SIMULATION_CONFIG_H_INCLUDED
#define SIM_SIMULATION_CONFIG_H_INCLUDED

/**
 * @file
 * Configuration data structures for the simulator built, with multi-region in mind.
 */

#include <memory>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "calendar/Calendar.h"
#include "core/LogMode.h"

namespace stride {

/**
 * Contains configuration info that is common to all sub-simulations within a
 * single simulation.
 */
struct CommonSimulationConfig final
{
	CommonSimulationConfig();

	bool track_index_case;

	/// The initial seed for the random number generator.
	unsigned int rng_seed;

	/// The r0 value.
	double r0;

	/// The seeding rate.
	double seeding_rate;

	/// The immunity rate.
	double immunity_rate;

	/// The number of days that are simulated.
	unsigned int number_of_days;

	/// The name of the disease configuration file.
	std::string disease_config_file_name;

	/// The number of people that participate in the survey.
	unsigned int number_of_survey_participants;

	/// The calendar at the start of the simulation.
	Calendar initial_calendar;

	/// The name of the age contact matrix file.
	std::string contact_matrix_file_name;

	/// Fills this configuration with data from the given ptree.
	void Parse(const boost::property_tree::ptree& pt);
};

/**
 * The configuration for the simulation's log.
 */
struct LogConfig final
{
	LogConfig();

	/// The prefix to prepend to the log file name.
	std::string output_prefix;

	/// Tells if a person file should be generated.
	bool generate_person_file;

	/// The log level for the simulation.
	LogMode log_level;

	/// Fills this configuration with data from the given ptree.
	void Parse(const boost::property_tree::ptree& pt);
};

/**
 * Contains configuration info for exactly one sub-simulation.
 */
struct SingleSimulationConfig final
{
	/// The configuration that is common to all sub-simulations.
	std::shared_ptr<CommonSimulationConfig> common_config;

	/// The population file for this simulation.
	std::string population_file_name;
};

/**
 * Contains configuration info for a simulation and all of its sub-simulations.
 */
struct MultiSimulationConfig final
{
	/// The configuration that is common to all sub-simulations.
	std::shared_ptr<CommonSimulationConfig> common_config;

	/// The configuration of the log.
	std::shared_ptr<LogConfig> log_config;

	/// The list of population files for populations to simulate.
	std::vector<std::string> population_file_names;

	/// Creates a vector that contains all single-simulation configurations.
	std::vector<SingleSimulationConfig> get_single_configs() const;

	/// Fills this configuration with data from the given ptree.
	void Parse(const boost::property_tree::ptree& pt);
};

} // namespace

#endif // end-of-include-guard
