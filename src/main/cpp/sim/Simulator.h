#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED
/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S, Broeckhove J
 *  Aerts S, De Haes C, Van der Cruysse J & Van Hauwe L
 */

/**
 * @file
 * Header for the Simulator class.
 */

#include "core/Cluster.h"
#include "core/DiseaseProfile.h"
#include "core/LogMode.h"
#include "core/RngHandler.h"
#include "multiregion/Visitor.h"
#include "multiregion/VisitorJournal.h"
#include "pop/Population.h"
#include "sim/SimulationConfig.h"

#include <memory>
#include <queue>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <spdlog/spdlog.h>

namespace stride {

class Calendar;

/**
 * Main class that contains and direct the virtual world.
 */
class Simulator
{
public:
	// Default constructor for empty Simulator.
	Simulator();

	/// Get the population.
	PopulationRef GetPopulation() const { return m_population; }

	/// Gets the simulator's configuration.
	SingleSimulationConfig GetConfiguration() const { return m_config; }

	/// Gets the simulator's date
	unsigned int GetDate() const;

	/// Change track_index_case setting.
	void SetTrackIndexCase(bool track_index_case);

	/// Run one time step, computing full simulation (default) or only index case.
	multiregion::SimulationStepOutput TimeStep(const multiregion::SimulationStepInput& input);

	/// Tests if this simulation has run to completion.
	bool IsDone() const { return m_calendar->GetSimulationDay() >= m_config.common_config->number_of_days; }

	/// Tests if the person is a visitor to this simulation.
	bool IsVisitor(PersonId id) const { return m_visitors.IsVisitor(id); }

	/// Runs the given action on every resident who is currently present
	/// in the simulation. More than one invocation of `action` may be
	/// running simultaneously. `action` must be invocable and have
	/// signature `void(const Person& person, unsigned int thread_number)`.
	template <typename TAction>
	void ParallelForeachPresentResident(const TAction& action)
	{
		m_population->parallel_for(m_num_threads, [this, &action](const Person& p, unsigned int thread_number) {
			if (!IsVisitor(p.GetId())) {
				action(p, thread_number);
			}
		});
	}

	/// Runs the given action on every resident of the simulation, present
	/// or otherwise. More than one invocation of `action` may be
	/// running simultaneously. `action` must be invocable and have
	/// signature `void(const Person& person, unsigned int thread_number)`.
	template <typename TAction>
	void ParallelForeachResident(const TAction& action)
	{
		ParallelForeachPresentResident<TAction>(action);
		m_expatriates.SerialForeach(action);
	}

	/// Runs the given action on every person who is currently present
	/// in the simulation, person or otherwise. More than one invocation
	/// of the action may be running simultaneously. `action` must be
	/// invocable and have signature
	/// `void(const Person& person, unsigned int thread_number)`.
	template <typename TAction>
	void ParallelForeachPresentPerson(const TAction& action)
	{
		m_population->parallel_for(m_num_threads, action);
	}

	/// Runs the given action on every resident who is currently present
	/// in the simulation. No more than one invocation of `action` will be
	/// running simultaneously. `action` must be invocable and have
	/// signature `void(const Person& person, unsigned int dummy)`.
	template <typename TAction>
	void SerialForeachPresentResident(const TAction& action)
	{
		m_population->serial_for([this, &action](const Person& p, unsigned int thread_number) {
			if (!IsVisitor(p.GetId())) {
				action(p, thread_number);
			}
		});
	}

	/// Runs the given action on every resident of the simulation, present
	/// or otherwise. No more than one invocation of `action` will be running
	/// simultaneously. `action` must be invocable and have
	/// signature `void(const Person& person, unsigned int dummy)`.
	template <typename TAction>
	void SerialForeachResident(const TAction& action)
	{
		SerialForeachPresentResident<TAction>(action);
		m_expatriates.SerialForeach(action);
	}

	/// Runs the given action on every person who is currently present
	/// in the simulation, person or otherwise. No more than one invocation
	/// of `action` will be running simultaneously. `action` must be
	/// invocable and have signature
	/// `void(const Person& person, unsigned int dummy)`.
	template <typename TAction>
	void SerialForeachPresentPerson(const TAction& action)
	{
		m_population->serial_for(m_num_threads, action);
	}

private:
	/// Accepts visitors from other regions.
	void AcceptVisitors(const multiregion::SimulationStepInput& input);

	/// Returns visitors whose return trip is scheduled today and returns them to their
	/// home regions.
	multiregion::SimulationStepOutput ReturnVisitors();

	/// Adds the given person to the clusters they've been assigned to.
	void AddPersonToClusters(const Person& person);

	/// Removes the given person from the clusters they've been assigned to.
	void RemovePersonFromClusters(const Person& person);

	/// Generates an id for a person that is not in use.
	PersonId GeneratePersonId();

	/// Generates a new household cluster and returns its id.
	std::size_t GenerateHousehold();

	/// Recycles the given person id.
	void RecyclePersonId(PersonId id);

	/// Recycles the household with the given id.
	void RecycleHousehold(std::size_t household_id);

	/// Update the contacts in the given clusters.
	template <LogMode log_level, bool track_index_case = false>
	void UpdateClusters();

private:
	SingleSimulationConfig m_config;       ///< Configuration for this simulator.
	std::shared_ptr<spdlog::logger> m_log; ///< Log for this simulator.

private:
	unsigned int m_num_threads;		    ///< The number of (OpenMP) threads.
	std::vector<RngHandler> m_rng_handler;      ///< Pointer to the RngHandlers.
	std::shared_ptr<util::Random> m_travel_rng; ///< A random number generator for travel.
	LogMode m_log_level;			    ///< Specifies logging mode.
	std::shared_ptr<Calendar> m_calendar;       ///< Management of calendar.

private:
	std::shared_ptr<Population> m_population;	     ///< Pointer to the Population.
	stride::multiregion::VisitorJournal m_visitors;       ///< Visitor journal.
	stride::multiregion::ExpatriateJournal m_expatriates; ///< Expatriate journal.

	std::vector<Cluster> m_households;	  ///< Container with household Clusters.
	std::vector<Cluster> m_school_clusters;     ///< Container with school Clusters.
	std::vector<Cluster> m_work_clusters;       ///< Container with work Clusters.
	std::vector<Cluster> m_primary_community;   ///< Container with primary community Clusters.
	std::vector<Cluster> m_secondary_community; ///< Container with secondary community  Clusters.

	std::queue<std::size_t> m_unused_households; ///< A list of unused households which can are eligible for recycling.
	std::queue<PersonId> m_unused_person_ids; ///< A list of unused person IDs which are eligible for recycling.

	DiseaseProfile m_disease_profile; ///< Profile of disease.

	bool m_track_index_case; ///< General simulation or tracking index case.

private:
	friend class SimulatorBuilder;
};

} // end_of_namespace

#endif // end-of-include-guard
