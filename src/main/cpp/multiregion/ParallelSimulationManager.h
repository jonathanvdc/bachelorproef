#ifndef MULTIREGION_PARALLEL_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_PARALLEL_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Parallel multi-region data structures for the simulator.
 */

#include <memory>
#include <thread>
#include <omp.h>
#include <spdlog/spdlog.h>
#include "multiregion/SequentialSimulationManager.h"
#include "multiregion/SimulationManager.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

namespace stride {
namespace multiregion {

/**
 * A parallel simulation task.
 */
template <typename TResult>
class ParallelSimulationTask final : public SimulationTask<TResult>
{
public:
	template <typename... TInitialResultArgs>
	ParallelSimulationTask(const std::shared_ptr<Simulator>& sim, TInitialResultArgs... args)
	    : sequential_task(sim, args...)
	{
	}

	/// Fetches this simulation task's result.
	TResult GetResult() final override { return sequential_task.GetResult(); }

	/// Starts this simulation.
	void Start() final override
	{
		if (task_thread != nullptr)
			return;

		task_thread =
		    std::make_shared<std::thread>(&SequentialSimulationTask<TResult>::Start, &sequential_task);
	}

	/// Waits for this simulation to complete.
	void Wait() final override
	{
		Start();
		task_thread->join();
	}

	/// Applies the given aggregation function to this simulation task's population.
	boost::any AggregateAny(std::function<boost::any(const Population&)> apply) final override
	{
		return sequential_task.AggregateAny(apply);
	}

private:
	std::shared_ptr<std::thread> task_thread;
	SequentialSimulationTask<TResult> sequential_task;
};

/**
 * A parallel simulation manager implementation.
 */
template <typename TResult, typename... TInitialResultArgs>
class ParallelSimulationManager final : public SimulationManager<TResult, TInitialResultArgs...>
{
public:
	ParallelSimulationManager(unsigned int number_of_sim_threads) : number_of_sim_threads(number_of_sim_threads) {}

	/// Creates and initiates a new simulation task based on the given configuration.
	std::shared_ptr<SimulationTask<TResult>> CreateSimulation(const SingleSimulationConfig& configuration,
								  const std::shared_ptr<spdlog::logger>& log,
								  TInitialResultArgs... args) final override
	{
		// Build a simulator.
		auto sim = SimulatorBuilder::Build(configuration, log, number_of_sim_threads);
		return std::make_shared<ParallelSimulationTask<TResult>>(sim, args...);
	}

private:
	unsigned int number_of_sim_threads;
};

} // namespace
} // namespace

#endif // end-of-include-guard
