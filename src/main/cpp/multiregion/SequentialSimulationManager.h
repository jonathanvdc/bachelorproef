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

/// The result of a pull operation.
struct PullResult final
{
	/// The list of all incoming visitors.
	std::vector<IncomingVisitor> visitors;

	/// The list of all returning expatriates.
	std::vector<Person> expatriates;
};

/// Defines a communication buffer for a single task.
class TaskCommunicationBuffer final
{
public:
	/// Tells if this communication buffer is ready for a pull operation
	/// by checking if all of its dependencies have been satisfied.
	bool IsReady() const { return unsatisfied_dependencies.size() == 0; }

	/// Satisfied the given dependency.
	void SatisfyDependency(RegionId dependency) { unsatisfied_dependencies.erase(dependency); }

	/// Pulls the data from this buffer.
	PullResult Pull()
	{
		PullResult result = std::move(pull_buffer);
		pull_buffer.visitors.clear();
		pull_buffer.expatriates.clear();
		return std::move(result);
	}

	/// Pushes a visitor from the given region into this buffer.
	void PushVisitor(RegionId source_region_id, const OutgoingVisitor& visitor)
	{
		pull_buffer.visitors.emplace_back(visitor.person, source_region_id, visitor.return_day);
	}

	/// Pushes an expatriate from the given region into.
	void PushExpatriate(const Person& expatriate) { pull_buffer.expatriates.emplace_back(expatriate); }

	/// Sets this buffer's dependencies to the given set of dependencies.
	void ResetDependencies(const std::unordered_set<RegionId>& new_unsatisfied_dependencies)
	{
		unsatisfied_dependencies = new_unsatisfied_dependencies;
	}

private:
	/// The task's next pull result, which is mutable.
	PullResult pull_buffer;

	/// The set of all task dependencies that have not been satisfied yet.
	std::unordered_set<RegionId> unsatisfied_dependencies;
};

/// Contains common data for a graph of communicating tasks.
class TaskCommunicationData final
{
public:
	/// Tries to find a task that's ready.
	bool TryPopReady(RegionId& id)
	{
		if (ready_tasks.empty()) {
			return false;
		}

		id = *ready_tasks.begin();
		ready_tasks.erase(id);
		return true;
	}

	/// Marks the task with the given id as ready.
	void MarkReady(RegionId id) { ready_tasks.insert(id); }

	/// Pulls input data for the task with the given id.
	PullResult Pull(RegionId id) { return buffers[id].Pull(); }

	/// Pushes output data for the task with the given id and dependencies.
	void Push(
	    RegionId id, const std::unordered_set<RegionId>& dependencies, const std::vector<OutgoingVisitor>& visitors,
	    const std::vector<OutgoingVisitor>& expatriates)
	{
		for (const auto& outgoing_visitor : visitors) {
			buffers[outgoing_visitor.visited_region].PushVisitor(id, outgoing_visitor);
		}
		for (const auto& returning_expatriate : expatriates) {
			buffers[returning_expatriate.visited_region].PushExpatriate(returning_expatriate.person);
		}
		buffers[id].ResetDependencies(dependencies);
		for (const auto& dep : dependencies) {
			auto& buf = buffers[dep];
			buf.SatisfyDependency(id);
			if (buf.IsReady()) {
				ready_tasks.insert(dep);
			}
		}
		if (buffers[id].IsReady()) {
			MarkReady(id);
		}
	}

private:
	std::unordered_set<RegionId> ready_tasks;
	std::unordered_map<RegionId, TaskCommunicationBuffer> buffers;
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

		PullResult Pull() { return manager->comm_data.Pull(id); }

		void Push(const std::vector<OutgoingVisitor>& visitors, const std::vector<OutgoingVisitor>& expatriates)
		{
			manager->comm_data.Push(id, manager->tasks[id]->GetConnectedRegions(), visitors, expatriates);
		}

	private:
		RegionId id;
		SequentialSimulationManager<TResult, TInitialResultArgs...>* manager;
	};

	TaskCommunicationData comm_data;
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
		comm_data.MarkReady(id);
		return task;
	}

	/// Waits for all tasks to complete.
	void WaitAll() final override
	{
		RegionId ready_id;
		while (comm_data.TryPopReady(ready_id)) {
			auto task = tasks[ready_id];
			if (!task->IsDone()) {
				task->Step();
			}
		}
	}
};

} // namespace
} // namespace

#endif // end-of-include-guard
