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

#include "Infector.h"
#include "LogMode.h"
#include "Person.h"
#include "sim/Calendar.h"

#include "spdlog/spdlog.h"
#include <cstddef>
#include <memory>
#include <vector>

namespace stride {

using namespace std;

Cluster::Cluster(std::size_t cluster_id, ClusterType cluster_type)
        : m_cluster_id(cluster_id), m_cluster_type(cluster_type), m_index_immune(0)
{
}

/// Add the given Person to the Cluster.
void Cluster::AddPerson(Person* p)
{
        m_members.emplace_back(std::make_pair(p, true));
        m_index_immune++;

        if ((m_cluster_type == ClusterType::School) && (p->GetAge() > 24U)) {
                m_cluster_type = ClusterType::Work;
        }
}

tuple<bool, size_t> Cluster::SortMembers()
{
        bool infectious_cases = false;
        size_t num_cases = 0;

        for (size_t i_member = 0; i_member < m_index_immune; i_member++) {
                // if immune, move to back
                if (m_members[i_member].first->GetHealth().IsImmune()) {
                        bool swapped = false;
                        size_t new_place = m_index_immune - 1;
                        m_index_immune--;
                        while(! swapped && new_place > i_member) {
                                if (m_members[new_place].first->GetHealth().IsImmune()) {
                                        m_index_immune--;
                                        new_place--;
                                } else {
                                        swap(m_members[i_member], m_members[new_place]);
                                        swapped = true;
                                }
                        }
                }
                // else, if not susceptible, move to front
                else if (!m_members[i_member].first->GetHealth().IsSusceptible()) {
                        if (!infectious_cases && m_members[i_member].first->GetHealth().IsInfectious()) {
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

void Cluster::Update(shared_ptr<ContactHandler> contact_handler, shared_ptr<const Calendar> sim_state, LogMode log_mode, bool index_case)
{
        switch (log_mode) {
                case LogMode::Contacts:
                        if (index_case) {
                                Infector<LogMode::Contacts, true>::Execute(*this, contact_handler, sim_state);
                        } else {
                                Infector<LogMode::Contacts, false>::Execute(*this, contact_handler, sim_state);
                        }
                        break;
                case LogMode::Transmissions:
                        if (index_case ) {
                                Infector<LogMode::Transmissions, true>::Execute(*this, contact_handler, sim_state);
                        } else {
                                Infector<LogMode::Transmissions, false>::Execute(*this, contact_handler, sim_state);
                        }
                        break;
                case LogMode::None:
                        if (index_case) {
                                Infector<LogMode::None, true>::Execute(*this, contact_handler, sim_state);
                        } else {
                                Infector<LogMode::None, false>::Execute(*this, contact_handler, sim_state);
                        }
                        break;
                default: throw runtime_error(std::string(__func__) + "Logging screwed up!");
        }


}

void Cluster::UpdateMemberPresence()
{
        for (auto& member: m_members) {
                member.second = member.first->IsInCluster(m_cluster_type);
        }
}

} // end_of_namespace
