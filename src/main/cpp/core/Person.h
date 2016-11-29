#ifndef PERSON_H_INCLUDED
#define PERSON_H_INCLUDED
/*
 *  This file is part of the indismo software.
 *  It is free software: you can redistribute it and/or modify it
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
 *  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and
 *  Broeckhove J. (2015) Optimizing agent-based transmission models for
 *  infectious diseases, BMC Bioinformatics.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */
/**
 * @file
 * Header file for the Person class.
 */


#include "../sim/WorldEnvironment.h"
#include "util/TrackIndexCase.h"
#include "spdlog/spdlog.h"

#include <cstddef>

namespace indismo {
namespace core {

/**
 * Store and handle person data.
 */
class Person
{
public:
	/// Constructor: set the person data.
	Person(unsigned int id, double age, unsigned int household_id, unsigned int home_district_id,
			unsigned int day_cluster_id, unsigned int day_district_id, unsigned int start_infectiousness,
			unsigned int start_symptomatic, unsigned int time_infectious, unsigned int time_symptomatic)
		: m_id(id), m_age(age), m_gender('M'),
		  m_household(household_id), m_home_district(home_district_id),
		  m_day_cluster(day_cluster_id), m_day_district(day_district_id),
		  m_in_household(true), m_in_home_district(true),
		  m_in_day_cluster(true), m_in_day_district(true),
		  m_susceptible(true),m_infected(false), m_infectious(false),
		  m_symptomatic(false), m_recovered(false), m_immune(false), m_disease_counter(0),
		  m_start_infectiousness(start_infectiousness), m_start_symptomatic(start_symptomatic),
		  m_is_participant(false)
	{
		m_end_infectiousness = start_infectiousness + time_infectious;
		m_end_symptomatic = start_symptomatic + time_symptomatic;
	}

	/// Is this person not equal to the given person?
	bool operator!=(const Person& p) const { return p.m_id != m_id; }

	/// Get the id.
	unsigned int GetId() const { return m_id; }

	/// Get the age.
	double GetAge() const { return m_age; }

	char GetGender() const { return m_gender; }

	/// Get the household id.
	unsigned int GetHouseholdId() const { return m_household; }

	/// Get the home neighborhood id.
	unsigned int GetHomeDistrictId() const { return m_home_district; }

	/// Get the day cluster id.
	unsigned int GetDayClusterId() const { return m_day_cluster; }

	/// Get the day neighborhood id.
	unsigned int GetDayDistrictId() const { return m_day_district; }

	/// Is this person susceptible?
	bool IsSusceptible() const { return m_susceptible; }

	/// Is this person infected?
	bool IsInfected() const { return m_infected; }

	/// Is this person infectious?
	bool IsInfectious() const { return m_infectious; }

	/// Is this person symptomatic?
	bool IsSymptomatic() const { return m_symptomatic; }

	/// Is this person recovered?
	bool IsRecovered() const { return m_recovered; }

	/// Is this person immune?
	bool IsImmune() const { return m_immune; }

	/// Get the disease counter.
	unsigned int GetDiseaseCounter() const { return m_disease_counter; }

	/// Increment the persons disease counter.
	void IncrementDiseaseCounter() { m_disease_counter++; }

	/// Reset the persons disease counter.
	void ResetDiseaseCounter() { m_disease_counter = 0U; }

	///
	unsigned int GetStartInfectiousness() const { return m_start_infectiousness; }

	///
	unsigned int GetEndInfectiousness() const { return m_end_infectiousness; }

	///
	unsigned int GetStartSymptomatic() const { return m_start_symptomatic; }

	///
	unsigned int GetEndSymptomatic() const { return m_end_symptomatic; }

	///
	size_t GetHouseholdSize() const { return m_household_size; }

	///
	void SetHouseholdSize(size_t hh_size) { m_household_size = hh_size; }

	/// Check if a person is present today in a given cluster
	bool IsInCluster(std::string cluster_type) const
	{
		if (cluster_type == "household") {
			return m_in_household;
		} else if (cluster_type == "home_district") {
			return m_in_home_district;
		} else if (cluster_type == "school" || cluster_type == "work") {
			return m_in_day_cluster;
		} else if (cluster_type == "day_district") {
			return m_in_day_district;
		}

		return false;
	}

	/// Log a contact of this person has with another person.
	void LogContact(std::shared_ptr<spdlog::logger> logger,
	                const Person* p2, std::string cluster_type,
	                std::shared_ptr<const WorldEnvironment> world_environ)
	{
	        unsigned int home   = (cluster_type == "household");
	        unsigned int work   = (cluster_type == "work");
	        unsigned int school = (cluster_type == "school");
	        unsigned int other  = (cluster_type == "home_district" || cluster_type == "day_district");

	        logger->info("[CONT] {} {} {} {} {} {} {} {}",
	                m_id, m_age, p2->GetAge(),
	                home, work, school, other, world_environ->GetSimulationDay());

	}

	///
	void LogTransmission(std::shared_ptr<spdlog::logger> logger,
	                const Person* p2, std::string cluster_type,
	                std::shared_ptr<const WorldEnvironment> world_environ)
	{
		logger->info("[TRAN] {} {} {} {}",
		        m_id, p2->GetId(), cluster_type,
		        world_environ->GetSimulationDay());
	}

	/**
	 * Set this person as index case.
	 *
	 * @note StartInfection() is not used since this method can be adapted to estimate R0
	 */
	void SetIndexCase()
	{
		m_susceptible 	= false;
		m_infected 	= true;
		ResetDiseaseCounter();
	}

	/**
	 * Start infection.
	 *
	 * @note To estimate R0, we need to track only the index case(s) so secondary cases are not infectious.
	 */
	void StartInfection()
	{
		assert(IsSusceptible() && "StartInfection: IsSusceptible() fails.");
		m_susceptible 	= false;
		m_infected 	    = true;
		ResetDiseaseCounter();

		if (TRACK_INDEX_CASE) {
			StopInfection();
		}
	}

	/// Stop the infection.
	void StopInfection()
	{
		assert(IsInfected() && "StopInfection: IsInfected() fails.");
		m_infected    = false;
		m_recovered   = true;
	}

	/// Set this persons immune status on TRUE
	void SetImmune()
	{
		m_immune                = true;

		m_susceptible           = false;
		m_start_infectiousness  = 0;
		m_start_symptomatic     = 0;
		m_end_infectiousness    = 0;
		m_end_symptomatic       = 0;
	}

	/// Update the health status and presence in clusters.
	void Update(std::shared_ptr<const WorldEnvironment> world_environ)
	{
		if (IsInfected()) {
			IncrementDiseaseCounter();
			if (GetDiseaseCounter() == m_start_infectiousness) {
				m_infectious = true;
			}
			if (GetDiseaseCounter() == m_end_infectiousness) {
				m_infectious = false;
				if(!m_symptomatic){
					StopInfection();
				}
			}
			if(GetDiseaseCounter() == m_start_symptomatic) {
				m_symptomatic = true;
			}
			if(GetDiseaseCounter() == m_end_symptomatic) {
				m_symptomatic = false;
				if(!m_infectious){
					StopInfection();
				}
			}
		}

		// update presence in clusters
		if (m_age > 18) { // adult
			if (world_environ->IsHoliday() || world_environ->IsWeekend()) {
				m_in_day_cluster = false;
			} else {
				m_in_day_cluster = true;
			}
		} else { // kid, so look at school holidays too
			if (world_environ->IsHoliday() || world_environ->IsSchoolHoliday() || world_environ->IsWeekend()) {
				m_in_day_cluster = false;
			} else {
				m_in_day_cluster = true;
			}
		}
	}

	/**
	 * Participate in social contact study and log person details
	 */
	void ParticipateInSurvey(std::shared_ptr<spdlog::logger> logger)
	{
		m_is_participant = true;
		logger->info("[PART] {} {} {}", m_id, m_age, m_gender);
	}

	/**
	 * Does this person participates in the social contact study?
	 */
	bool IsParticipatingInSurvey() const
	{
		return m_is_participant;
	}

private:
	unsigned int    m_id;		       		 ///< The id.
	double          m_age;		       		 ///< The age.
	char            m_gender;                        ///< The gender.

	unsigned int    m_household;	   		 ///< The household id.
	std::size_t     m_household_size;		 ///< The number of persons in the household.
	unsigned int    m_home_district;   		 ///< The home district id
	unsigned int    m_day_cluster;	   		 ///< The day cluster id
	unsigned int    m_day_district;	   		 ///< The day district id

	bool            m_in_household;			 ///< Is person present in household today?
	bool            m_in_home_district;		 ///< Is person present in home_district today?
	bool            m_in_day_cluster;		 ///< Is person present in day_cluster today?
	bool            m_in_day_district;		 ///< Is person present in day_district today?

	bool            m_susceptible;	   		 ///< Is this person susceptible?
	bool            m_infected;	       		 ///< Is this person infected?
	bool            m_infectious;	  	 	 ///< Is this person infectious?
	bool            m_symptomatic;                   ///< Is this person symptomatic
	bool            m_recovered;                     ///< Is this person recovered?
	bool            m_immune;                        ///< Is this person immune?

	unsigned int    m_disease_counter; 		 ///< The disease counter.

	unsigned int    m_start_infectiousness;          ///< Days after infection to become infectious.
	unsigned int    m_start_symptomatic;             ///< Days after infection to become symptomatic.
	unsigned int    m_end_infectiousness;            ///< Days after infection to end infectious state.
	unsigned int    m_end_symptomatic;               ///< Days after infection to end symptomatic state.

	bool            m_is_participant;		 ///< Is participating in the social contact study
};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
