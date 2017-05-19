#ifndef PERSON_H_INCLUDED
#define PERSON_H_INCLUDED
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
 * Header file for the Person class.
 */

#include "core/Disease.h"
#include "core/Health.h"

#include <cstddef>
#include <iostream>
#include <memory>

#include "behaviour/behaviour_policies/AlwaysFollowBeliefs.h"
#include "behaviour/behaviour_policies/NoBehaviour.h"

#include "behaviour/belief_policies/NoBelief.h"
#include "behaviour/belief_policies/Threshold.h"

namespace stride {

using PersonId = unsigned int;

class Calendar;
enum class ClusterType;

/**
 * Store and handle person data.
 */
template <class BehaviourPolicy, class BeliefPolicy>
class GenericPersonData
{
public:
	/// Creates a person from the given data.
	GenericPersonData(
	    double age, unsigned int household_id, unsigned int school_id, unsigned int work_id,
	    unsigned int primary_community_id, unsigned int secondary_community_id, disease::Fate fate,
	    double risk_averseness = 0)
	    : m_age(age), m_gender('M'), m_household_id(household_id), m_school_id(school_id), m_work_id(work_id),
	      m_primary_community_id(primary_community_id), m_secondary_community_id(secondary_community_id),
	      m_at_household(true), m_at_school(true), m_at_work(true), m_at_primary_community(true),
	      m_at_secondary_community(true), m_health(fate), m_is_participant(false)
	{
		BeliefPolicy::Initialize(m_belief_data, risk_averseness);
	}

	/// Get the age.
	double GetAge() const { return m_age; }

	/// Get cluster ID of cluster_type
	unsigned int GetClusterId(ClusterType cluster_type) const;

	/// Get cluster ID of cluster_type
	unsigned int& GetClusterId(ClusterType cluster_type);

	/// Return person's gender.
	char GetGender() const { return m_gender; }

	/// Return person's health status.
	Health& GetHealth() { return m_health; }

	/// Return person's health status.
	const Health& GetHealth() const { return m_health; }

	/// Return person's belief status.
	const typename BeliefPolicy::Data& GetBeliefData() const { return m_belief_data; }

	/// Check if a person is present today in a given cluster
	bool IsInCluster(ClusterType c) const;

	/// Does this person participates in the social contact study?
	bool IsParticipatingInSurvey() const { return m_is_participant; }

	/// Participate in social contact study and log person details
	void ParticipateInSurvey() { m_is_participant = true; }

	/// Update the health status and presence in clusters.
	void Update(bool is_work_off, bool is_school_off, double fraction_infected);

	/// Update belief & behaviour upon meeting another Person
	void Update(std::shared_ptr<const GenericPersonData> p);

private:
	double m_age;
	char m_gender;

	/// Which communities does this person belong to?
	unsigned int m_household_id;
	unsigned int m_school_id;
	unsigned int m_work_id;
	unsigned int m_primary_community_id;
	unsigned int m_secondary_community_id;

	/// Which of those communities are they present at today?
	bool m_at_household;
	bool m_at_school;
	bool m_at_work;
	bool m_at_primary_community;
	bool m_at_secondary_community;

	/// Health info for this person.
	Health m_health;

	/// Info about this Person's health beliefs.
	typename BeliefPolicy::Data m_belief_data;

	/// Is this person participating in the social contact study?
	bool m_is_participant;
};

extern template class GenericPersonData<NoBehaviour, NoBelief>;
extern template class GenericPersonData<AlwaysFollowBeliefs, Threshold<true, false>>;
extern template class GenericPersonData<AlwaysFollowBeliefs, Threshold<false, true>>;
extern template class GenericPersonData<AlwaysFollowBeliefs, Threshold<true, true>>;

/**
 * Describes a person: personal data tagged by an id.
 */
template <class BehaviourPolicy, class BeliefPolicy>
class GenericPerson
{
public:
	using PersonData = GenericPersonData<BehaviourPolicy, BeliefPolicy>;
	/// Creates a person from the given information.
	GenericPerson(
	    PersonId id, double age, unsigned int household_id, unsigned int school_id, unsigned int work_id,
	    unsigned int primary_community_id, unsigned int secondary_community_id, disease::Fate fate,
	    double risk_averseness = 0)
	    : m_id(id), m_data(
			    std::make_shared<PersonData>(
				age, household_id, school_id, work_id, primary_community_id, secondary_community_id,
				fate, risk_averseness))
	{
	}

	/// Creates a person from the given id and data.
	GenericPerson(PersonId id, const std::shared_ptr<PersonData>& data) : m_id(id), m_data(data) {}

	/// Checks if this person is equal to the given person.
	bool operator==(const GenericPerson& p) const { return m_id == p.m_id; }

	/// Checks if this person is not equal to the given person.
	bool operator!=(const GenericPerson& p) const { return !(*this == p); }

	/// Get the age.
	double GetAge() const { return m_data->GetAge(); }

	/// Get cluster ID of cluster_type
	unsigned int& GetClusterId(ClusterType cluster_type) const { return m_data->GetClusterId(cluster_type); }

	/// Return person's gender.
	char GetGender() const { return m_data->GetGender(); }

	/// Return person's health status.
	Health& GetHealth() const { return m_data->GetHealth(); }

	/// Return person's belief status.
	const typename BeliefPolicy::Data& GetBeliefData() const { return m_data->GetBeliefData(); }

	/// Get the id.
	PersonId GetId() const { return m_id; }

	/// Creates a deep copy of this person, including its data.
	GenericPerson Clone() const { return GenericPerson(m_id, std::make_shared<PersonData>(*m_data)); }

	/// Creates a deep copy of this person and gives it the given id.
	GenericPerson WithId(PersonId new_id) const;

	/// Check if a person is present today in a given cluster
	bool IsInCluster(ClusterType c) const { return m_data->IsInCluster(c); }

	/// Does this person participates in the social contact study?
	bool IsParticipatingInSurvey() const { return m_data->IsParticipatingInSurvey(); }

	/// Participate in social contact study and log person details
	void ParticipateInSurvey() const { m_data->ParticipateInSurvey(); }

	/// Update the health status and presence in clusters.
	void Update(bool is_work_off, bool is_school_off, double fraction_infected) const
	{
		m_data->Update(is_work_off, is_school_off, fraction_infected);
	}

	/// Update belief & behaviour upon meeting another Person
	void Update(std::shared_ptr<const PersonData> p) const { m_data->Update(p); }

	/// Gets the data that backs this person.
	std::shared_ptr<PersonData> GetData() const { return m_data; }

private:
	PersonId m_id;
	std::shared_ptr<PersonData> m_data;
};

extern template class GenericPerson<NoBehaviour, NoBelief>;
extern template class GenericPerson<AlwaysFollowBeliefs, Threshold<true, false>>;
extern template class GenericPerson<AlwaysFollowBeliefs, Threshold<false, true>>;
extern template class GenericPerson<AlwaysFollowBeliefs, Threshold<true, true>>;

// TODO: Where does this belong; here or Simulator?
using PersonData = GenericPersonData<NoBehaviour, NoBelief>;
using Person = GenericPerson<NoBehaviour, NoBelief>;

} // end_of_namespace

namespace std {
/// An std::hash<T> implementation for Person.
template <>
struct hash<stride::Person>
{
	typedef stride::Person argument_type;
	typedef std::size_t result_type;
	result_type operator()(const argument_type& person) const
	{
		return std::hash<stride::PersonId>()(person.GetId());
	}
};
}

#endif // end of include guard
