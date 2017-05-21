#include "Health.h"

#include <assert.h>

namespace stride {

Health::Health(disease::Fate fate) : m_days_infected(0), m_status(HealthStatus::Susceptible), m_fate(fate) {}

void Health::SetImmune()
{
	m_status = HealthStatus::Immune;
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
