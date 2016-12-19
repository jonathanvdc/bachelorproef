/*
 * Disease.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: elise
 */

#include "Disease.h"

#include <assert.h>

namespace stride {


Disease::Disease(unsigned int start_infectiousness, unsigned int start_symptomatic,
		unsigned int time_infectious, unsigned int time_symptomatic):
		m_disease_status(DiseaseStatus::Susceptible), m_disease_counter(0U),
		m_start_infectiousness(start_infectiousness), m_start_symptomatic(start_symptomatic)
	{
		m_end_infectiousness = start_infectiousness + time_infectious;
		m_end_symptomatic = start_symptomatic + time_symptomatic;
	}

Disease::~Disease() {}

void Disease::SetImmune() {
	m_disease_status = DiseaseStatus::Immune;
	m_start_infectiousness= 0U;
	m_start_symptomatic = 0U;
	m_end_infectiousness = 0U;
	m_end_symptomatic = 0U;
}


void Disease::StartInfection() {
	assert(m_disease_status == DiseaseStatus::Susceptible && "StartInfection: m_disease_status == DiseaseStatus::Susceptible fails.");
	m_disease_status = DiseaseStatus::Exposed;
	ResetDiseaseCounter();

}

void Disease::StopInfection() {
	assert((m_disease_status == DiseaseStatus::Exposed || m_disease_status == DiseaseStatus::Infectious
			|| m_disease_status == DiseaseStatus::Symptomatic || m_disease_status == DiseaseStatus::InfectiousAndSymptomatic)
			&& "StopInfection: person not infected");

	m_disease_status = DiseaseStatus::Recovered;
}

void Disease::Update() {
	bool infected = m_disease_status == DiseaseStatus::Exposed
			|| m_disease_status == DiseaseStatus::Infectious
			|| m_disease_status == DiseaseStatus::Symptomatic
			|| m_disease_status == DiseaseStatus::InfectiousAndSymptomatic;

	if (infected) {

		IncrementDiseaseCounter();

		if (GetDiseaseCounter() == m_start_infectiousness) {
			if (m_disease_status == DiseaseStatus::Symptomatic) {
				m_disease_status = DiseaseStatus::InfectiousAndSymptomatic;
			} else {
				m_disease_status = DiseaseStatus::Infectious;
			}

		}
		if (GetDiseaseCounter() == m_end_infectiousness) {
			if (m_disease_status == DiseaseStatus::InfectiousAndSymptomatic) {
				m_disease_status = DiseaseStatus::Symptomatic;
			} else {
				StopInfection();
			}

		}
		if (GetDiseaseCounter() == m_start_symptomatic) {
			if (m_disease_status == DiseaseStatus::Infectious) {
				m_disease_status = DiseaseStatus::InfectiousAndSymptomatic;
			} else {
				m_disease_status = DiseaseStatus::Symptomatic;
			}

		}
		if (GetDiseaseCounter() == m_end_symptomatic) {
			if (m_disease_status == DiseaseStatus::InfectiousAndSymptomatic) {
				m_disease_status = DiseaseStatus::Infectious;
			} else {
				StopInfection();
			}

		}

	}
}

} /* namespace stride */

