#ifndef MULTIREGION_SEQUENTAL_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_SEQUENTAL_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Sequential multi-region data structures for the simulator.
 */

#include <memory>
#include <omp.h>
#include <spdlog/spdlog.h>
#include "multiregion/SimulationManager.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

namespace stride {
namespace multiregion {

/**
 * A sequential simulation task.
 */
template <typename TResult>
class SequentialSimulationTask final : public SimulationTask<TResult>
{
    public:
	SequentialSimulationTask(const std::shared_ptr<Simulator>& sim) : sim(sim), result(), has_completed(false)
	{
	}

	/// Fetches this simulation task's result.
	TResult GetResult() final override
	{
		return result;
	}

	/// Starts this simulation.
	void Start() final override
	{
		if (has_completed)
			return;

		has_completed = true;
		for (unsigned int i = 0; i < sim->GetConfiguration().common_config->number_of_days; i++) {
			result.BeforeSimulatorStep(*sim->GetPopulation());
			sim->TimeStep();
			result.AfterSimulatorStep(*sim->GetPopulation());
		}
	}

	/// Waits for this simulation to complete.
	void Wait() final override
	{
		Start();
	}

    protected:
	/// Applies the given aggregation function to this simulation task's population.
	boost::any AggregateAny(std::function<boost::any(const Population&)> apply) final override
	{
		return apply(*sim->GetPopulation());
	}

    private:
	std::shared_ptr<Simulator> sim;
	TResult result;
	bool has_completed;
};

/**
 * A sequential simulation manager implementation.
 */
template <typename TResult>
class SequentialSimulationManager final : public SimulationManager<TResult>
{
    public:
	SequentialSimulationManager(unsigned int number_of_sim_threads) : number_of_sim_threads(number_of_sim_threads)
	{
	}

	/// Creates and initiates a new simulation task based on the given configuration.
	std::shared_ptr<SimulationTask<TResult>> CreateSimulation(
	    const SingleSimulationConfig& configuration, const std::shared_ptr<spdlog::logger>& log) final override
	{
		// Build a simulator.
		auto sim = SimulatorBuilder::Build(configuration, log, number_of_sim_threads);
		return std::make_shared<SequentialSimulationTask<TResult>>(sim);
	}

    private:
	unsigned int number_of_sim_threads;
};

} // namespace
} // namespace

#endif // end-of-include-guard
