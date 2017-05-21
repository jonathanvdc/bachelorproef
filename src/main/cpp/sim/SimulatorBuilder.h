#ifndef SIMULATOR_BUILDER_H_INCLUDED
#define SIMULATOR_BUILDER_H_INCLUDED

#include "Simulator.h"
#include "sim/SimulationConfig.h"

#include <memory>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <spdlog/spdlog.h>

namespace stride {

class Population;
class Calendar;

/**
 * Main class that contains and direct the virtual world.
 */
class SimulatorBuilder
{
public:
	/// Build simulator.
	static std::shared_ptr<Simulator> Build(
	    const std::string& config_file_name, const std::shared_ptr<spdlog::logger>& log,
	    unsigned int num_threads = 1U, bool track_index_case = false);

	/// Build simulator.
	static std::shared_ptr<Simulator> Build(
	    const boost::property_tree::ptree& pt_config, const std::shared_ptr<spdlog::logger>& log,
	    unsigned int num_threads = 1U, bool track_index_case = false);

	/// Build simulator.
	static std::shared_ptr<Simulator> Build(
	    const SingleSimulationConfig& config, const std::shared_ptr<spdlog::logger>& log,
	    unsigned int num_threads = 1U);

	/// Build simulator.
	static std::shared_ptr<Simulator> Build(
	    const boost::property_tree::ptree& pt_config, const boost::property_tree::ptree& pt_disease,
	    const boost::property_tree::ptree& pt_contact, const std::shared_ptr<spdlog::logger>& log,
	    unsigned int number_of_threads = 1U, bool track_index_case = false);

	/// Build simulator.
	static std::shared_ptr<Simulator> Build(
	    const SingleSimulationConfig& config, const boost::property_tree::ptree& pt_disease,
	    const boost::property_tree::ptree& pt_contact, const std::shared_ptr<spdlog::logger>& log,
	    unsigned int number_of_threads = 1U);

private:
	/// Initialize the clusters.
	static void InitializeClusters(std::shared_ptr<Simulator> sim);
};

} // end_of_namespace

#endif // end-of-include-guard
