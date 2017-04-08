#ifndef MULTIREGION_PARALLEL_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_PARALLEL_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Parallel multi-region data structures for the simulator.
 */

#include <memory>
#include <mutex>
#include <thread>
#include <spdlog/spdlog.h>
#include "multiregion/LocalSimulationTask.h"
#include "multiregion/SequentialSimulationManager.h"
#include "multiregion/SimulationManager.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

namespace stride {
namespace multiregion {

/**
 * A parallel simulation manager implementation.
 */
template <typename TResult, typename... TInitialResultArgs>
class ParallelSimulationManager final : public SimulationManager<TResult, TInitialResultArgs...>
{
private:
	class ParallelTaskCommunicator final
	{
	public:
		ParallelTaskCommunicator(
		    RegionId id, ParallelSimulationManager<TResult, TInitialResultArgs...>* manager)
		    : id(id), manager(manager)
		{
		}

		SimulationStepInput Pull()
		{
			std::lock_guard<std::mutex> lock(manager->comm_mutex);
			return manager->comm_data.Pull(id);
		}

		void Push(const SimulationStepOutput& data)
		{
			std::lock_guard<std::mutex> lock(manager->comm_mutex);
			manager->comm_data.Push(id, manager->tasks[id]->GetConnectedRegions(), data);
		}

	private:
		RegionId id;
		ParallelSimulationManager<TResult, TInitialResultArgs...>* manager;
	};

	bool TryPopReadyAtomic(RegionId& id)
	{
		std::lock_guard<std::mutex> lock(comm_mutex);
		auto success = comm_data.TryPopReady(id);
		if (success) {
			comm_data.ResetDependencies(id, tasks[id]->GetConnectedRegions());
		}
		return success;
	}

	TaskCommunicationData comm_data;
	std::unordered_map<RegionId, std::shared_ptr<LocalSimulationTask<TResult, ParallelTaskCommunicator>>> tasks;
	std::mutex comm_mutex;
	std::size_t number_of_task_threads;
	unsigned int number_of_sim_threads;

public:
	ParallelSimulationManager(std::size_t number_of_task_threads, unsigned int number_of_sim_threads)
	    : number_of_task_threads(number_of_task_threads), number_of_sim_threads(number_of_sim_threads)
	{
	}

	/// Creates and initiates a new simulation task based on the given configuration.
	std::shared_ptr<SimulationTask<TResult>> CreateSimulation(
	    const SingleSimulationConfig& configuration, const std::shared_ptr<spdlog::logger>& log,
	    TInitialResultArgs... args) final override
	{
		// Build a simulator.
		auto sim = SimulatorBuilder::Build(configuration, log, number_of_sim_threads);
		auto id = configuration.travel_model->GetRegionId();
		auto task = std::make_shared<LocalSimulationTask<TResult, ParallelTaskCommunicator>>(
		    sim, ParallelTaskCommunicator(id, this), args...);
		tasks[id] = task;
		comm_data.MarkReady(id);
		return task;
	}

	/// Waits for all tasks to complete.
	void WaitAll() final override
	{
		// Start 'number_of_task_threads' threads.
		std::vector<std::thread> threads;
		for (std::size_t i = 0; i < number_of_task_threads; i++) {
			threads.emplace_back([this]() {
				RegionId ready_id;
				while (TryPopReadyAtomic(ready_id)) {
					auto task = tasks[ready_id];
					if (!task->IsDone()) {
						task->Step();
					}
				}
			});
		}
		// Wait for all the threads to complete.
		for (auto& t : threads) {
			t.join();
		}
	}
};

} // namespace
} // namespace

#endif // end-of-include-guard
