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

#include "Cluster.h"
#include "LogMode.h"
#include "Person.h"
#include "sim/WorldEnvironment.h"
#include "util/TrackIndexCase.h"


#include "spdlog/spdlog.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>


namespace indismo {

using namespace std;

Cluster::Cluster(std::size_t cluster_id, std::string cluster_type)
        : m_cluster_id(cluster_id), m_index_immune(0)
{
        m_cluster_type = IsClusterType(cluster_type) ?
                ToClusterType(cluster_type)
                : throw std::runtime_error(std::string(__func__) + "> Problem with cluster_type" + cluster_type);
        m_logger = spdlog::get("contact_logger");
}

tuple<bool, size_t> Cluster::SortMembers()
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
                                } else {
                                        swap(m_members[i_member], m_members[new_place]);
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
                                swap(m_members[i_member], m_members[num_cases]);
                        }
                        num_cases++;
                }
        }

        return make_tuple(infectious_cases, num_cases);
}

void Cluster::UpdateMemberPresence()
{
        for (auto& member: m_members) {
                member.second = member.first->IsInCluster(m_cluster_type);
        }
}

template<>
void Cluster::Update<LogMode::None>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state)
{
        // check if the cluster has infected members and sort
        bool infectious_cases;
        size_t num_cases;
        tie(infectious_cases, num_cases) = SortMembers();

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
                                                        if ((*contact_handler)(age1, ToString(m_cluster_type), cluster_size)) {
                                                                p2->StartInfection();
                                                                if (TRACK_INDEX_CASE) {
                                                                        p2->StopInfection();
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

template<>
void Cluster::Update<LogMode::Transmissions>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state)
{
        // check if the cluster has infected members and sort
        bool infectious_cases;
        size_t num_cases;
        tie(infectious_cases, num_cases) = SortMembers();

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
                                                        if ((*contact_handler)(age1, ToString(m_cluster_type), cluster_size)) {
                                                                // log transmission
                                                                m_members[i_infected].first->LogTransmission(m_logger, p2, m_cluster_type, sim_state);
                                                                p2->StartInfection();
                                                                if (TRACK_INDEX_CASE) {
                                                                        p2->StopInfection();
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

template<>
void Cluster::Update<LogMode::Contacts>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state)
{
        UpdateMemberPresence();
        size_t cluster_size = GetSize();

        // check all contacts
        for (size_t i_person1 = 0; i_person1 < m_members.size(); i_person1++) {
                // check if member participates in the social contact survey && member is present today
                if (m_members[i_person1].second && m_members[i_person1].first->IsParticipatingInSurvey()) {
                        auto p1 = m_members[i_person1].first;
                        const auto age1 = p1->GetAge();
                        for (size_t i_person2 = 0; i_person2 < m_members.size(); i_person2++) {
                                // check if member is present today
                                if ((i_person1 != i_person2) && m_members[i_person2].second) {
                                        auto p2 = m_members[i_person2].first;
                                        // check for contact
                                        if (contact_handler->contact(age1, ToString(m_cluster_type), cluster_size)) {
                                                // TODO ContactHandler doesn't have a separate transmission function anymore to
                                                // check for transmission when contact has already been checked.
                                                // check for transmission
                                                /*bool transmission = contact_handler->transmission(age1, p2->GetAge());
                                                unsigned int infecter = 0;
                                                if (transmission) {
                                                        if (p1->IsInfectious() && p2->IsSusceptible()) {
                                                                infecter = 1;
                                                                p2->StartInfection();
                                                                if (TRACK_INDEX_CASE) {
                                                                        p2->StopInfection();
                                                                }
                                                        }
                                                        else if (p2->IsInfectious() && p1->IsSusceptible()) {
                                                                infecter = 2;
                                                                p1->StartInfection();
                                                                if (TRACK_INDEX_CASE) {
                                                                        p1->StopInfection();
                                                                }
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


template void Cluster::Update<LogMode::None>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state);

template void Cluster::Update<LogMode::Transmissions>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state);

template void Cluster::Update<LogMode::Contacts>(shared_ptr<ContactHandler> contact_handler, shared_ptr<const WorldEnvironment> sim_state);

} // end_of_namespace
