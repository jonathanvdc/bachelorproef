#ifndef POPULATION_BUILDER_H_INCLUDED
#define POPULATION_BUILDER_H_INCLUDED

#include "Population.h"
#include "sim/SimulationConfig.h"
#include "util/Random.h"

#include <memory>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <spdlog/spdlog.h>

namespace stride {

/**
 * Initializes Population objects.
 */
class PopulationBuilder
{
public:
	/**
	 * Initializes a Population: add persons, set immunity, seed infection.
	 *
	 * @param config          Single simulation configuration information.
	 * @param pt_disease      Property_tree with disease configuration settings.
	 * @param log             The contact log.
	 * @return                Pointer to the initialized population.
	 */
	static std::shared_ptr<Population> Build(
	    const SingleSimulationConfig& config, const boost::property_tree::ptree& pt_disease, util::Random& rng,
	    const std::shared_ptr<spdlog::logger>& log);
};

} // end_of_namespace

#endif // include guard
