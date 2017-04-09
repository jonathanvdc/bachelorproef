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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
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

namespace stride {

using PersonId = unsigned int;

class Calendar;
enum class ClusterType;

/**
 * Store and handle person data.
 */
class PersonData
{
public:
	/// Creates a person from the given data.
	PersonData(
	    double age, unsigned int household_id, unsigned int school_id, unsigned int work_id,
	    unsigned int primary_community_id, unsigned int secondary_community_id, disease::Fate fate)
	    : m_age(age), m_gender('M'), m_household_id(household_id), m_school_id(school_id), m_work_id(work_id),
	      m_primary_community_id(primary_community_id), m_secondary_community_id(secondary_community_id),
	      m_at_household(true), m_at_school(true), m_at_work(true), m_at_primary_community(true),
	      m_at_secondary_community(true), m_health(fate), m_is_participant(false)
	{
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

	/// Check if a person is present today in a given cluster
	bool IsInCluster(ClusterType c) const;

	/// Does this person participates in the social contact study?
	bool IsParticipatingInSurvey() const { return m_is_participant; }

	/// Participate in social contact study and log person details
	void ParticipateInSurvey() { m_is_participant = true; }

	/// Update the health status and presence in clusters.
	void Update(bool is_work_off, bool is_school_off);

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

	/// Is this person participating in the social contact study?
	bool m_is_participant;
};

/**
 * Describes a person: personal data tagged by an id.
 */
class Person
{
public:
	/// Creates a person from the given information.
	Person(
	    PersonId id, double age, unsigned int household_id, unsigned int school_id, unsigned int work_id,
	    unsigned int primary_community_id, unsigned int secondary_community_id, disease::Fate fate)
	    : m_id(id), m_data(std::make_unique<PersonData>(
			    age, household_id, school_id, work_id, primary_community_id, secondary_community_id, fate))
	{
	}

	Person(const Person& other) : m_id(other.m_id), m_data(std::make_unique<PersonData>(*other.m_data)) {}
	Person& operator=(const Person& other)
	{
		if (this == &other) {
			return *this;
		}

		m_id = other.m_id;
		m_data = std::make_unique<PersonData>(*other.m_data);
		return *this;
	}

	Person(Person&& other) = default;
	Person& operator=(Person&& other) = default;

	/// Checks if this person is equal to the given person.
	bool operator==(const Person& p) const { return m_id == p.m_id; }

	/// Checks if this person is not equal to the given person.
	bool operator!=(const Person& p) const { return !(*this == p); }

	/// Get the age.
	double GetAge() const { return m_data->GetAge(); }

	/// Get cluster ID of cluster_type
	unsigned int& GetClusterId(ClusterType cluster_type) const { return m_data->GetClusterId(cluster_type); }

	/// Return person's gender.
	char GetGender() const { return m_data->GetGender(); }

	/// Return person's health status.
	Health& GetHealth() const { return m_data->GetHealth(); }

	/// Get the id.
	PersonId GetId() const { return m_id; }

	/// Creates a copy of this person and gives it the given id.
	Person WithId(PersonId new_id) const;

	/// Check if a person is present today in a given cluster
	bool IsInCluster(ClusterType c) const { return m_data->IsInCluster(c); }

	/// Does this person participates in the social contact study?
	bool IsParticipatingInSurvey() const { return m_data->IsParticipatingInSurvey(); }

	/// Participate in social contact study and log person details
	void ParticipateInSurvey() const { m_data->ParticipateInSurvey(); }

	/// Update the health status and presence in clusters.
	void Update(bool is_work_off, bool is_school_off) const { m_data->Update(is_work_off, is_school_off); }

	/// Gets the data that backs this person.
	PersonData& GetData() const { return *m_data; }

private:
	PersonId m_id;
	std::unique_ptr<PersonData> m_data;
};

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
