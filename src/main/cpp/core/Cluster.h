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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Header for the core Cluster class.
 */

#include "ContactHandler.h"
#include "ClusterType.h"
#include "Person.h"
#include "sim/WorldEnvironment.h"
#include "LogMode.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace stride {

/**
 * Represents a location for social contacts, an group of people.
 */
class Cluster
{
public:
	/// Constructor
	Cluster(std::size_t cluster_id, ClusterType cluster_type);

	/// Add the given Person to the Cluster.
	void AddPerson(Person* p)
	{
		m_members.push_back(std::make_pair(p, true));
		m_index_immune++;
	}

	///
	size_t GetSize() const { return m_members.size(); }

	///
	ClusterType GetClusterType() const { return m_cluster_type; }

	///
	void SetClusterType(ClusterType cluster_type) { m_cluster_type = cluster_type; }

	///
	void Update(std::shared_ptr<ContactHandler> contact_handler,
	        std::shared_ptr<const WorldEnvironment> sim_state,
	        LogMode log_mode, bool index_case = false);

private:
	/// Sort members of cluster according to health status
	/// (order: exposed/infected/recovered, susceptible, immune)
	std::tuple<bool, size_t> SortMembers();

        template<LogMode log_level, bool track_index_case>
        friend class Infector;

	/// Check which members are present in the cluster on the current day
	void UpdateMemberPresence();

private:
	std::size_t                               m_cluster_id;     ///< The ID of the Cluster (for logging purposes).
	ClusterType                               m_cluster_type;   ///< The type of the Cluster (for logging purposes).
	std::size_t                               m_index_immune;   ///< Index of the first immune member in the Cluster.
	std::vector<std::pair<Person*, bool>>     m_members;        ///< Container with pointers to the members of the Cluster.
};

} // end_of_namespace

#endif // include-guard
