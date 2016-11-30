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
#include "Person.h"
#include "sim/WorldEnvironment.h"
#include "LogMode.h"

#include "spdlog/spdlog.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>


namespace indismo {
namespace core {

/**
 * Represents a location for social contacts, an group of people.
 */
class Cluster
{
public:
	/// Constructor
	Cluster(std::size_t cluster_id, std::string cluster_type):
		m_cluster_id(cluster_id),
		m_cluster_type(cluster_type),
		m_index_immune(0)
	{
		m_logger = spdlog::get("contact_logger");
	}

	/// Add the given Person to the Cluster.
	void AddPerson(Person* p)
	{
		m_members.push_back(std::make_pair(p, true));
		m_index_immune++;
	}

	///
	size_t GetSize() const { return m_members.size(); }

	///
	std::string GetClusterType() const { return m_cluster_type; }

	///
	void SetClusterType(std::string cluster_type) { m_cluster_type = cluster_type; }

	/// Update the social contacts between the infectious and susceptible members with sorting on health status.
	template<LogMode log_level>
	void Update(std::shared_ptr<ContactHandler> contact_handler, std::shared_ptr<const WorldEnvironment> sim_state)
	{
		if (log_level < LogMode::Contacts) {
		//if (!LOG_CONTACTS) {
			// check if the cluster has infected members and sort
			bool infectious_cases;
			size_t num_cases;
			std::tie(infectious_cases, num_cases) = SortMembers();

			if (infectious_cases) {

				UpdateMemberPresence();
				size_t cluster_size = GetSize();

				// match infectious in first part with susceptible in second part, skip last part (immune)
				for (size_t i_infected = 0; i_infected < num_cases; i_infected++) {
					// check if member is present today
					if (m_members[i_infected].second) {
						if (m_members[i_infected].first->IsInfectious()) {
							const auto age1 = m_members[i_infected].first->GetAge();
							for (size_t i_contact = num_cases; i_contact < m_index_immune; i_contact++) {
								// check if member is present today
								if (m_members[i_contact].second) {
									auto p2 = m_members[i_contact].first;
									if ((*contact_handler)(age1, m_cluster_type,cluster_size)) {
										//if (LOG_TRANSMISSION) {
										if (log_level == LogMode::Transmissions) {
											// log transmission
											m_members[i_infected].first->LogTransmission(m_logger, p2,
													m_cluster_type, sim_state);
										}
										p2->StartInfection();
									}
								}
							}
						}
					}
				}
			}
		} else {
			UpdateMemberPresence();
			size_t cluster_size = GetSize();

			// check all contacts
			for (size_t i_person1 = 0; i_person1 < m_members.size(); i_person1++) {
				// check if member participates in the social contact survey
				// check if member is present today
				if (m_members[i_person1].second && m_members[i_person1].first->IsParticipatingInSurvey()) {
					auto p1 = m_members[i_person1].first;
					const auto age1 = p1->GetAge();
					for (size_t i_person2 = 0; i_person2 < m_members.size(); i_person2++) {
						// check if member is present today
						if ((i_person1 != i_person2) && m_members[i_person2].second) {
							auto p2 = m_members[i_person2].first;
							// check for contact
							if(contact_handler->contact(age1, m_cluster_type, cluster_size)) {
								// TODO ContactHandler doesn't have a separate transmission function anymore to check for transmission when contact has already been checked.
								// check for transmission
								/*bool transmission = contact_handler->transmission(age1, p2->GetAge());
								unsigned int infecter = 0;
								if (transmission) {
									if (p1->IsInfectious() && p2->IsSusceptible()) {
										infecter = 1;
										p2->StartInfection();
									}
									else if (p2->IsInfectious() && p1->IsSusceptible()) {
										infecter = 2;
										p1->StartInfection();
									}
									//TODO log transmission?
								}*/
								p1->LogContact(m_logger, p2, m_cluster_type, sim_state);

							}
						}
					}
				}
			}
		}
	}

private:

	/// Sort members of cluster according to health status
	/// (order: exposed/infected/recovered, susceptible, immune)
	std::tuple<bool, size_t> SortMembers()
	{
		bool infectious_cases = false;
		size_t num_cases = 0;

		for (size_t i_member = 0; i_member < m_index_immune; i_member++) {
			// if immune, move to back
			if (m_members[i_member].first->IsImmune()) {

				bool swapped = false;
				size_t new_place = m_index_immune - 1;
				m_index_immune--;
				while(! swapped && new_place > i_member) {
					if(m_members[new_place].first->IsImmune()) {
						m_index_immune--;
						new_place--;
					}
					else {
						std::swap(m_members[i_member], m_members[new_place]);
						swapped = true;
					}
				}
			}
			// else, if not susceptible, move to front
			else if (!m_members[i_member].first->IsSusceptible()) {
				if (!infectious_cases && m_members[i_member].first->IsInfectious()) {
					infectious_cases = true;
				}
				if (i_member > num_cases) {
					std::swap(m_members[i_member], m_members[num_cases]);
				}
				num_cases++;
			}
		}

		return std::make_tuple(infectious_cases, num_cases);
	}

	/// Check which members are present in the cluster on the current day
	void UpdateMemberPresence()
	{
		for (auto member: m_members) {
			if (member.first->IsInCluster(m_cluster_type)) {
				member.second = true;
			}
			else {
				member.second = false;
			}
		}
	}

private:
	std::size_t                               m_cluster_id;     ///< The ID of the Cluster (for logging purposes).
	std::string                               m_cluster_type;   ///< The type of the Cluster (for logging purposes).
	std::size_t                               m_index_immune;   ///< Index of the first immune member in the Cluster.
	std::shared_ptr<spdlog::logger>           m_logger;         ///< Pointer to the logger used for recording contacts.
	std::vector<std::pair<Person*, bool>>     m_members;        ///< Container with pointers to the members of the Cluster.
};


} // end_of_namespace
} // end_of_namespace

#endif // include-guard
