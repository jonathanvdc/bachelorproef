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
#include "multiregion/TravelModel.h"

namespace stride {

/**
 * Contains configuration info that is common to all sub-simulations within a
 * single simulation.
 */
struct CommonSimulationConfig final
{
	CommonSimulationConfig();

	bool track_index_case;
	bool generate_vis_file;

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

struct MultiSimulationConfig;

/**
 * Contains configuration info for exactly one sub-simulation.
 */
struct SingleSimulationConfig final
{
	/// The configuration that is common to all sub-simulations.
	std::shared_ptr<CommonSimulationConfig> common_config;

	/// The log configuration.
	std::shared_ptr<LogConfig> log_config;

	/// The travel model for this region.
	stride::multiregion::RegionTravelRef travel_model;

	/// Gets the unique id for the region used by this config.
	stride::multiregion::RegionId GetId() const { return travel_model->GetRegionId(); }

	/// Gets a path to the population file for this simulation.
	std::string GetPopulationPath() const { return travel_model->GetRegionPopulationPath(); };

	/// Gets a path to the geodistribution profile file for this simulation.
	/// (This string is empty if the population is read from a CSV file.)
	std::string GetGeodistributionProfilePath() const
	{
		return travel_model->GetRegionGeodistributionProfilePath();
	};

	/// Gets a path to the reference households file for this simulation.
	/// (This string is empty if the population is read from a CSV file.)
	std::string GetReferenceHouseholdsPath() const { return travel_model->GetRegionReferenceHouseholdsPath(); };

	/// Returns this single-simulation configuration as a multi-simulation
	/// configuration that contains a single sub-simulation.
	MultiSimulationConfig AsMultiConfig() const;

	/// Fills this configuration with data from the given ptree.
	void Parse(const boost::property_tree::ptree& pt);
};

/**
 * Contains configuration info for a simulation and all of its sub-simulations.
 */
struct MultiSimulationConfig final
{
	/// The configuration that is common to all sub-simulations.
	std::shared_ptr<CommonSimulationConfig> common_config;

	/// The log configuration.
	std::shared_ptr<LogConfig> log_config;

	/// The list of models for the regions to simulate.
	std::vector<stride::multiregion::RegionTravelRef> region_models;

	/// Creates a vector that contains all single-simulation configurations.
	std::vector<SingleSimulationConfig> GetSingleConfigs() const;

	/// Returns this multi-simulation configuration as a single sub-simulation
	/// configuration. This requires for there to be exactly one sub-simulation
	/// in this simulation.
	SingleSimulationConfig AsSingleConfig() const;

	/// Fills this configuration with data from the given ptree.
	void Parse(const boost::property_tree::ptree& pt);
};

} // namespace

#endif // end-of-include-guard
