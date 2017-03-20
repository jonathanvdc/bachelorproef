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

#include "Health.h"

#include <assert.h>

namespace stride {

Health::Health(disease::Fate fate) : m_days_infected(0), m_status(HealthStatus::Susceptible), m_fate(fate) {}

void Health::SetImmune()
{
	m_status = HealthStatus::Immune;
	/*
		m_start_infectiousness = 0U;
		m_start_symptomatic = 0U;
		m_end_infectiousness = 0U;
		m_end_symptomatic = 0U;
	*/
}

void Health::StartInfection()
{
	assert(IsSusceptible() && "Health::StartInfection: person not susceptible");
	m_status = HealthStatus::Exposed;
	ResetDaysInfected();
}

void Health::StopInfection()
{
	assert(IsInfected() && "Health::StopInfection: person not infected");
	m_status = HealthStatus::Recovered;
}

void Health::Update()
{
	if (IsInfected()) {
		IncrementDaysInfected();
		if (GetDaysInfected() == GetStartInfectiousness()) {
			if (m_status == HealthStatus::Symptomatic) {
				m_status = HealthStatus::InfectiousAndSymptomatic;
			} else {
				m_status = HealthStatus::Infectious;
			}
		} else if (GetDaysInfected() == GetEndInfectiousness()) {
			if (m_status == HealthStatus::InfectiousAndSymptomatic) {
				m_status = HealthStatus::Symptomatic;
			} else {
				StopInfection();
			}
		} else if (GetDaysInfected() == GetStartSymptomatic()) {
			if (m_status == HealthStatus::Infectious) {
				m_status = HealthStatus::InfectiousAndSymptomatic;
			} else {
				m_status = HealthStatus::Symptomatic;
			}
		} else if (GetDaysInfected() == GetEndSymptomatic()) {
			if (m_status == HealthStatus::InfectiousAndSymptomatic) {
				m_status = HealthStatus::Infectious;
			} else {
				StopInfection();
			}
		}
	}
}

} // namespace stride
