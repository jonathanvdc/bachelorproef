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

#include "Cluster.h"

#include "Infector.h"
#include "LogMode.h"
#include "calendar/Calendar.h"
#include "pop/Person.h"

#include <spdlog/spdlog.h>
#include <cstddef>
#include <memory>
#include <vector>

namespace stride {

using namespace std;

std::array<ContactProfile, NumOfClusterTypes()> Cluster::g_profiles;

Cluster::Cluster(std::size_t cluster_id, ClusterType cluster_type)
        : m_cluster_id(cluster_id), m_cluster_type(cluster_type),
          m_index_immune(0), m_profile(g_profiles.at(ToSizeType(m_cluster_type)))
{
}

void Cluster::AddContactProfile(ClusterType cluster_type, const ContactProfile& profile)
{
        g_profiles.at(ToSizeType(cluster_type)) = profile;
}


void Cluster::AddPerson(const Person& p)
{
        if (p.GetHealth().IsImmune()) {
                m_members.emplace_back(
                        std::make_pair(p, p.IsInCluster(m_cluster_type)));
        }
        else {
                m_members.emplace(
                        m_members.begin() + m_index_immune,
                        std::make_pair(p, p.IsInCluster(m_cluster_type)));
                m_index_immune++;
        }
}

void Cluster::RemovePerson(const Person& p)
{
        std::size_t index = 0;
        while (index < m_members.size()) {
                if (m_members[index].first == p) {
                        m_members.erase(m_members.begin() + index);
                        if (m_index_immune == index) {
                                m_index_immune++;
                        }
                        if (m_index_immune > m_members.size()) {
                                m_index_immune = m_members.size();
                        }
                        return;
                }
                index++;
        }
}

tuple<bool, std::size_t> Cluster::SortMembers()
{
        bool infectious_cases = false;
        std::size_t num_cases = 0;

        for (size_t i_member = 0; i_member < m_index_immune; i_member++) {
                // if immune, move to back
                if (m_members[i_member].first.GetHealth().IsImmune()) {
                        bool swapped = false;
                        std::size_t new_place = m_index_immune - 1;
                        m_index_immune--;
                        while(! swapped && new_place > i_member) {
                                if (m_members[new_place].first.GetHealth().IsImmune()) {
                                        m_index_immune--;
                                        new_place--;
                                } else {
                                        swap(m_members[i_member], m_members[new_place]);
                                        swapped = true;
                                }
                        }
                }
                // else, if not susceptible, move to front
                else if (!m_members[i_member].first.GetHealth().IsSusceptible()) {
                        if (!infectious_cases && m_members[i_member].first.GetHealth().IsInfectious()) {
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
                member.second = member.first.IsInCluster(m_cluster_type);
        }
}

} // end_of_namespace
