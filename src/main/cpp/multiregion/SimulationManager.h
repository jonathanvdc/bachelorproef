#ifndef MULTIREGION_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Multi-region data structures for the simulator.
 */

#include <functional>
#include <memory>
#include <vector>
#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include "pop/Population.h"
#include "sim/SimulationConfig.h"

namespace stride {
namespace multiregion {

/**
 * An abstract class that can be used to communicate with and control a single simulation.
 */
template <typename TResult>
class SimulationTask
{
    public:
	/// Fetches this simulation task's result.
	virtual TResult GetResult() = 0;

	/// Waits for this simulation to complete.
	virtual void Wait() = 0;

	/// Applies the given aggregation function to this simulation task's population.
	template <typename TAggregate>
	TAggregate Aggregate(TAggregate apply(const Population&))
	{
		return boost::any_cast<TAggregate>(
		    AggregateAny([apply](const Population& pop) -> boost::any { return boost::any(apply(pop)); }));
	}

	/// Gets the simulation task's population.
	size_t GetPopulationSize()
	{
		return Aggregate<size_t>([](const Population& pop) -> size_t { return pop.size(); });
	}

	/// Gets the number of people that are infected in the simulation task's population.
	size_t GetInfectedCount()
	{
		return Aggregate<size_t>([](const Population& pop) -> size_t { return pop.GetInfectedCount(); });
	}

	/// Gets this simulation task's population.
	Population GetPopulation()
	{
		return Aggregate<Population>([](const Population& pop) -> Population { return pop; });
	}

    protected:
	/// Applies the given aggregation function to this simulation task's population.
	virtual boost::any AggregateAny(std::function<boost::any(const Population&)>) = 0;
};

/**
 * An abstract class that initiates simulations.
 */
template <typename TResult>
struct SimulationManager
{
	/// Creates and initiates a new simulation task based on the given configuration.
	virtual std::shared_ptr<SimulationTask<TResult>> StartSimulation(
	    const SingleSimulationConfig& configuration, const TResult& initialResult,
	    void pre_sim_step(TResult&, const Population&), void post_sim_step(TResult&, const Population&)) = 0;
};

} // namespace
} // namespace

#endif // end-of-include-guard
