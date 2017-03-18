#ifndef MULTIREGION_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Configuration data structures for the simulator built, with multi-region in mind.
 */

#include <memory>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace multiregion {

/*
 * Defines a simulation configuration for a single simulation.
 */
struct SingleSimulationConfig final
{
	SingleSimulationConfig(bool track_index_case, std::string population_file_name)
	    : track_index_case(track_index_case), population_file_name(population_file_name)
	{
	}

	bool track_index_case;

    /// The global configuration.
    boost::property_tree::ptree global_configuration;

	/// The name of the population file.
	std::string population_file_name;
};

/**
 * An abstract class that initiates simulations.
 */
struct SimulationManager
{
	/// Initializes a new simulation instance based on the given configuration.
	virtual std::unique_ptr<SimulationInstance> initialize(const SingleSimulationConfig& configuration) = 0;
};

/**
 * An abstract class that can be used to communicate with and control a single simulation.
 */
struct SimulationInstance
{
};

} // namespace
} // namespace

#endif // end-of-include-guard
