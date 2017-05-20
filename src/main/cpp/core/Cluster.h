#ifndef CLUSTER_H_INCLUDED
#define CLUSTER_H_INCLUDED
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
 * Header for the core Cluster class.
 */

#include "behaviour/information_policies/NoLocalInformation.h"
#include "core/ClusterType.h"
#include "core/ContactProfile.h"
#include "core/LogMode.h"
#include "pop/Person.h"

#include <array>
#include <cstddef>
#include <vector>
//#include <memory>

namespace stride {

class RngHandler;
class Calendar;

using ClusterId = std::size_t;

/**
 * Represents a location for social contacts, an group of people.
 */
class Cluster
{
public:
	/// Constructor
	Cluster(ClusterId cluster_id, ClusterType cluster_type);

	/// Add the given Person to the Cluster.
	void AddPerson(const Person& p);

	/// Removes the given person from this cluster.
	void RemovePerson(const Person& p);

	/// Returns the ID of the cluster.
	ClusterId GetId() const { return m_cluster_id; }

	/// Returns the vector of people.
	std::vector<Person> GetPeople() const;

	/// Return number of persons in this cluster.
	std::size_t GetSize() const { return m_members.size(); }

	/// Return the type of this cluster.
	ClusterType GetClusterType() const { return m_cluster_type; }

	/// Get basic contact rate in this cluster.
	double GetContactRate(const Person& p) const
	{
		return g_profiles.at(ToSizeType(m_cluster_type))[EffectiveAge(p.GetAge())] / m_members.size();
	}

public:
	/// Add contact profile.
	static void AddContactProfile(ClusterType cluster_type, const ContactProfile& profile);

private:
	/// Sort members w.r.t. health status (order: exposed/infected/recovered, susceptible, immune).
	std::tuple<bool, std::size_t> SortMembers();

	/// Infector calculates contacts and transmissions.
	template <LogMode log_level, bool track_index_case, typename local_information_policy>
	friend class Infector;

	/// Calculate which members are present in the cluster on the current day.
	void UpdateMemberPresence();

private:
	/// The ID of the Cluster (for logging purposes).
	ClusterId m_cluster_id;

	/// The type of the Cluster (for logging purposes).
	ClusterType m_cluster_type;

	/// Index of the first immune member in the Cluster.
	std::size_t m_index_immune;

	/// Container with pointers to Cluster members.
	std::vector<std::pair<Person, bool>> m_members;

	const ContactProfile& m_profile;

private:
	static std::array<ContactProfile, NumOfClusterTypes()> g_profiles;
};

} // end_of_namespace

#endif // include-guard
