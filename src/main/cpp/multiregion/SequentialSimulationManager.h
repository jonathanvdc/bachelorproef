#ifndef MULTIREGION_SEQUENTIAL_SIMULATION_MANAGER_H_INCLUDED
#define MULTIREGION_SEQUENTIAL_SIMULATION_MANAGER_H_INCLUDED

/**
 * @file
 * Sequential multi-region data structures for the simulator.
 */

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include <spdlog/spdlog.h>
#include "multiregion/SimulationManager.h"
#include "multiregion/Visitor.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"

namespace stride {
namespace multiregion {

/**
 * A sequential simulation task.
 */
template <typename TResult, typename TCommunicator>
class SequentialSimulationTask final : public SimulationTask<TResult>
{
public:
	template <typename... TInitialResultArgs>
	SequentialSimulationTask(
	    const std::shared_ptr<Simulator>& sim, const TCommunicator& communicator, TInitialResultArgs... args)
	    : sim(sim), communicator(communicator), result(args...)
	{
	}

	/// Fetches this simulation task's result.
	TResult GetResult() final override { return result; }

	/// Starts this simulation.
	void Start() final override
	{
		while (!sim->IsDone()) {
			Step();
		}
	}

	/// Performs a single step in the simulation.
	void Step()
	{
		auto pull = communicator.Pull();
		result.BeforeSimulatorStep(*sim->GetPopulation());
		sim->TimeStep();
		result.AfterSimulatorStep(*sim->GetPopulation());
		communicator.Push({}, {});
	}

	/// Waits for this simulation to complete.
	void Wait() final override { Start(); }

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

struct PullResult
{
	std::vector<IncomingVisitor> visitors;
	std::vector<Person> expatriates;
};

/**
 * A sequential simulation manager implementation.
 */
template <typename TResult, typename... TInitialResultArgs>
class SequentialSimulationManager final : public SimulationManager<TResult, TInitialResultArgs...>
{
private:
	class SequentialTaskCommunicator final
	{
	public:
		SequentialTaskCommunicator(
		    RegionId id, SequentialSimulationManager<TResult, TInitialResultArgs...>* manager)
		    : id(id), manager(manager)
		{
		}

		PullResult Pull()
		{
			while (!CanPull()) {
				auto first_task = *manager->ready_tasks.begin();
				manager->tasks[first_task]->Step();
			}

			auto visitors = std::move(manager->visitor_queues[id]);
			auto expats = std::move(manager->expat_queues[id]);
			manager->visitor_queues[id].clear();
			manager->expat_queues[id].clear();
			manager->ready_tasks.erase(id);
			return { visitors, expats };
		}

		void Push(const std::vector<OutgoingVisitor>& visitors, const std::vector<Person>& expatriates)
		{
			for (const auto& outgoing_visitor : visitors) {
				manager->visitor_queues[outgoing_visitor.visited_region].emplace_back(
				    outgoing_visitor.person, id, outgoing_visitor.return_day);
			}
			manager->task_unsatisfied_dependencies[id] = manager->tasks[id]->GetConnectedRegions();
			manager->task_unsatisfied_dependencies[id].erase(id);
			if (manager->task_unsatisfied_dependencies[id].size() == 0) {
				manager->ready_tasks.insert(id);
			}

			for (auto region_id : manager->tasks[id]->GetConnectedRegions()) {
				manager->task_unsatisfied_dependencies[region_id].erase(id);
				if (manager->task_unsatisfied_dependencies[region_id].size() == 0) {
					manager->ready_tasks.insert(region_id);
				}
			}
		}

	private:
		bool CanPull() const { return manager->ready_tasks.find(id) != manager->ready_tasks.end(); }
		RegionId id;
		SequentialSimulationManager<TResult, TInitialResultArgs...>* manager;
	};

	std::unordered_set<RegionId> ready_tasks;
	std::unordered_map<RegionId, std::vector<IncomingVisitor>> visitor_queues;
	std::unordered_map<RegionId, std::vector<Person>> expat_queues;
	std::unordered_map<RegionId, std::unordered_set<RegionId>> task_unsatisfied_dependencies;
	std::unordered_map<RegionId, std::shared_ptr<SequentialSimulationTask<TResult, SequentialTaskCommunicator>>>
	    tasks;
	unsigned int number_of_sim_threads;

public:
	SequentialSimulationManager(unsigned int number_of_sim_threads) : number_of_sim_threads(number_of_sim_threads)
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
		auto task = std::make_shared<SequentialSimulationTask<TResult, SequentialTaskCommunicator>>(
		    sim, SequentialTaskCommunicator(id, this), args...);
		tasks[id] = task;
		ready_tasks.insert(id);
		return task;
	}
};

} // namespace
} // namespace

#endif // end-of-include-guard
