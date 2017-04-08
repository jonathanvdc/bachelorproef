#ifndef LOCAL_SIMULATION_TASK_H_INCLUDED
#define LOCAL_SIMULATION_TASK_H_INCLUDED

/**
 * @file
 * Defines a type of simulation task that runs in the current process.
 */

#include <memory>
#include <unordered_set>
#include "multiregion/SimulationManager.h"
#include "multiregion/Visitor.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

namespace stride {
namespace multiregion {

/**
 * A simulation task that runs in the current process.
 */
template <typename TResult, typename TCommunicator>
class LocalSimulationTask final : public SimulationTask<TResult>
{
public:
	template <typename... TInitialResultArgs>
	LocalSimulationTask(
	    const std::shared_ptr<Simulator>& sim, const TCommunicator& communicator, TInitialResultArgs... args)
	    : sim(sim), communicator(communicator), result(args...)
	{
	}

	/// Fetches this simulation task's result.
	TResult GetResult() final override { return result; }

	/// Tells if this simulation is done.
	bool IsDone() const { return sim->IsDone(); }

	/// Performs a single step in the simulation.
	void Step()
	{
		auto pull = communicator.Pull();
		result.BeforeSimulatorStep(*sim->GetPopulation());
		sim->TimeStep();
		result.AfterSimulatorStep(*sim->GetPopulation());
		communicator.Push({}, {});
	}

	/// Applies the given aggregation function to this simulation task's population.
	boost::any AggregateAny(std::function<boost::any(const PopulationRef&)> apply) final override
	{
		return apply(sim->GetPopulation());
	}

	/// Gets the set of all regions that are connected to this region by an air route.
	const std::unordered_set<RegionId>& GetConnectedRegions() const
	{
		return sim->GetConfiguration().travel_model->GetConnectedRegions();
	}

private:
	std::shared_ptr<Simulator> sim;
	TCommunicator communicator;
	TResult result;
};
}
}

#endif