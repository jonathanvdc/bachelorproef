#ifndef SRC_MAIN_CPP_CORE_HEALTH_H_
#define SRC_MAIN_CPP_CORE_HEALTH_H_
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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

#include "Disease.h"

namespace stride {

enum class HealthStatus
{
	Susceptible = 0U,
	Exposed = 1U,
	Infectious = 2U,
	Symptomatic = 3U,
	InfectiousAndSymptomatic = 4U,
	Recovered = 5U,
	Immune = 6U,
};

/*
 * Represents the status of a Person's health at some point in the simulation.
 */
class Health
{
public:
	/// Initially, a person is Susceptible, and the "days infected" counter is set to 0.
	Health(disease::Fate fate);

	/// Return the person's current health status.
	HealthStatus GetHealthStatus() const { return m_status; }

	/// Return the day infectiousness starts.
	unsigned int GetStartInfectiousness() const { return m_fate.start_infectiousness; }

	/// Return the day infectiousness ends.
	unsigned int GetEndInfectiousness() const { return m_fate.end_infectiousness; }

	/// Return the day symptomaticity starts.
	unsigned int GetStartSymptomatic() const { return m_fate.start_symptomatic; }

	/// Return the day symptomaticity ends.
	unsigned int GetEndSymptomatic() const { return m_fate.end_symptomatic; }

	/// Return whether the person is currently immune.
	bool IsImmune() const { return m_status == HealthStatus::Immune; }

	/// Return whether the person is currently infected by the disease.
	bool IsInfected() const
	{
		switch (m_status) {
		case HealthStatus::Exposed:
		case HealthStatus::Infectious:
		case HealthStatus::Symptomatic:
		case HealthStatus::InfectiousAndSymptomatic:
			return true;
		default:
			return false;
		}
	}

	///
	bool IsInfectious() const
	{
		return m_status == HealthStatus::Infectious || m_status == HealthStatus::InfectiousAndSymptomatic;
	}

	///
	bool IsRecovered() const { return m_status == HealthStatus::Recovered; }

	/// Is this person susceptible?
	bool IsSusceptible() const { return m_status == HealthStatus::Susceptible; }

	/// Is this person symptomatic?
	bool IsSymptomatic() const
	{
		return m_status == HealthStatus::Symptomatic || m_status == HealthStatus::InfectiousAndSymptomatic;
	}

	/// Set immune to true.
	void SetImmune();

	/// Start the infection.
	void StartInfection();

	/// Stop the infection.
	void StopInfection();

	/// Update progress of the disease.
	void Update();

private:
	/// Get the disease counter.
	unsigned int GetDaysInfected() const { return m_days_infected; }

	/// Increment disease counter.
	void IncrementDaysInfected() { m_days_infected++; }

	/// Reset the disease counter.
	void ResetDaysInfected() { m_days_infected = 0U; }

private:
	/// The day counter (starts at 0, increased daily while the person is infected).
	unsigned int m_days_infected;

	/// The current health status.
	HealthStatus m_status;

	/// Times after infection at which the disease will act.
	const disease::Fate m_fate;
};

} // end of namespace

#endif
