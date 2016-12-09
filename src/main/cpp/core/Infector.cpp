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
 * Implementation of Infector algorithms.
 */

#include "Cluster.h"
#include "Infector.h"
#include "LogMode.h"
#include "Person.h"
#include "sim/WorldEnvironment.h"

#include "spdlog/spdlog.h"
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace indismo {

using namespace std;

//--------------------------------------------------------------------------
// Primary R0 policy: do nothing i.e. track all cases.
//--------------------------------------------------------------------------
template<bool track_index_case = false>
class R0_POLICY
{
public:
        static void Execute(indismo::Person* p) {}
};

//--------------------------------------------------------------------------
// Specialised R0 policy: track only the index case.
//--------------------------------------------------------------------------
template<>
class R0_POLICY<true>
{
public:
        static void Execute(indismo::Person* p) { p->StopInfection(); }
};


//--------------------------------------------------------------------------
// Declaration of partial specialization for LogMode::Contacts.
//--------------------------------------------------------------------------
template<bool track_index_case>
class Infector<LogMode::Contacts, track_index_case>
{
public:
        ///
        void operator()(Cluster& cluster,
                std::shared_ptr<ContactHandler> contact_handler,
                std::shared_ptr<const WorldEnvironment> sim_state);
};

//--------------------------------------------------------------------------
// Declaration of partial specialization for LogMode::Transmissions.
//--------------------------------------------------------------------------
template<bool track_index_case>
class Infector<LogMode::Transmissions, track_index_case>
{
public:
        ///
        void operator()(Cluster& cluster,
                std::shared_ptr<ContactHandler> contact_handler,
                std::shared_ptr<const WorldEnvironment> sim_state);
};

//--------------------------------------------------------------------------
// Declaration of partial specialization for LogMode::None.
//--------------------------------------------------------------------------
template<bool track_index_case>
class Infector<LogMode::None, track_index_case>
{
public:
        ///
        void operator()(Cluster& cluster,
                std::shared_ptr<ContactHandler> contact_handler,
                std::shared_ptr<const WorldEnvironment> sim_state);
};






//--------------------------------------------------------------------------
// Definition for primary template (empty).
//--------------------------------------------------------------------------
template<LogMode log_level, bool track_index_case>
void Infector<log_level, track_index_case>::operator()(Cluster& cluster,
                                shared_ptr<ContactHandler> contact_handler,
                                shared_ptr<const WorldEnvironment> sim_state)
{
}

//--------------------------------------------------------------------------
// Definition of partial specialization for LogMode::Contacts.
//--------------------------------------------------------------------------
template<bool track_index_case>
void Infector<LogMode::Contacts, track_index_case>::operator()(Cluster& cluster,
                                shared_ptr<ContactHandler> contact_handler,
                                shared_ptr<const WorldEnvironment> sim_state)
{
        cluster.UpdateMemberPresence();
        size_t cluster_size = cluster.GetSize();

        // check all contacts
        for (size_t i_person1 = 0; i_person1 < cluster.m_members.size(); i_person1++) {
                // check if member participates in the social contact survey && member is present today
                if (cluster.m_members[i_person1].second && cluster.m_members[i_person1].first->IsParticipatingInSurvey()) {
                        auto p1 = cluster.m_members[i_person1].first;
                        const auto age1 = p1->GetAge();
                        for (size_t i_person2 = 0; i_person2 < cluster.m_members.size(); i_person2++) {
                                // check if member is present today
                                if ((i_person1 != i_person2) && cluster.m_members[i_person2].second) {
                                        auto p2 = cluster.m_members[i_person2].first;
                                        // check for contact
                                        if (contact_handler->contact(age1, ToString(cluster.m_cluster_type), cluster_size)) {
                                                // TODO ContactHandler doesn't have a separate transmission function anymore to
                                                // check for transmission when contact has already been checked.
                                                // check for transmission
                                                /*bool transmission = contact_handler->transmission(age1, p2->GetAge());
                                                unsigned int infecter = 0;
                                                if (transmission) {
                                                        if (p1->IsInfectious() && p2->IsSusceptible()) {
                                                                infecter = 1;
                                                                p2->StartInfection();
                                                                R0_POLICY<track_index_case>::Execute(p2);
                                                        }
                                                        else if (p2->IsInfectious() && p1->IsSusceptible()) {
                                                                infecter = 2;
                                                                p1->StartInfection();
                                                                R0_POLICY<track_index_case>::Execute(p1);
                                                        }
                                                        //TODO log transmission?
                                                }*/
                                                p1->LogContact(cluster.m_logger, p2, cluster.m_cluster_type, sim_state);

                                        }
                                }
                        }
                }
        }
}

//--------------------------------------------------------------------------
// Definition of partial specialization for LogMode::Transmissions.
//--------------------------------------------------------------------------
template<bool track_index_case>
void Infector<LogMode::Transmissions, track_index_case>::operator()(Cluster& cluster,
                                        shared_ptr<ContactHandler> contact_handler,
                                        shared_ptr<const WorldEnvironment> sim_state)
{
        // check if the cluster has infected members and sort
        bool infectious_cases;
        size_t num_cases;
        tie(infectious_cases, num_cases) = cluster.SortMembers();

        if (infectious_cases) {
                cluster.UpdateMemberPresence();
                size_t cluster_size = cluster.GetSize();

                // match infectious in first part with susceptible in second part, skip last part (immune)
                for (size_t i_infected = 0; i_infected < num_cases; i_infected++) {
                        // check if member is present today
                        if (cluster.m_members[i_infected].second) {
                                if (cluster.m_members[i_infected].first->IsInfectious()) {
                                        const auto age1 = cluster.m_members[i_infected].first->GetAge();
                                        for (size_t i_contact = num_cases; i_contact < cluster.m_index_immune; i_contact++) {
                                                // check if member is present today
                                                if (cluster.m_members[i_contact].second) {
                                                        auto p2 = cluster.m_members[i_contact].first;
                                                        if ((*contact_handler)(age1, ToString(cluster.m_cluster_type), cluster_size)) {
                                                                // log transmission
                                                                cluster.m_members[i_infected].first->LogTransmission(
                                                                        cluster.m_logger, p2, cluster.m_cluster_type, sim_state);
                                                                p2->StartInfection();
                                                                R0_POLICY<track_index_case>::Execute(p2);
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}

//--------------------------------------------------------------------------
// Definition of partial specialization for LogMode::None.
//--------------------------------------------------------------------------
template<bool track_index_case>
void Infector<LogMode::None, track_index_case>::operator()(Cluster& cluster,
                                shared_ptr<ContactHandler> contact_handler,
                                shared_ptr<const WorldEnvironment> sim_state)
{
        // check if the cluster has infected members and sort
        bool infectious_cases;
        size_t num_cases;
        tie(infectious_cases, num_cases) = cluster.SortMembers();

        if (infectious_cases) {
                cluster.UpdateMemberPresence();
                size_t cluster_size = cluster.GetSize();

                // match infectious in first part with susceptible in second part, skip last part (immune)
                for (size_t i_infected = 0; i_infected < num_cases; i_infected++) {
                        // check if member is present today
                        if (cluster.m_members[i_infected].second) {
                                if (cluster.m_members[i_infected].first->IsInfectious()) {
                                        const auto age1 = cluster.m_members[i_infected].first->GetAge();
                                        for (size_t i_contact = num_cases; i_contact < cluster.m_index_immune; i_contact++) {
                                                // check if member is present today
                                                if (cluster.m_members[i_contact].second) {
                                                        auto p2 = cluster.m_members[i_contact].first;
                                                        if ((*contact_handler)(age1, ToString(cluster.m_cluster_type), cluster_size)) {
                                                                p2->StartInfection();
                                                                R0_POLICY<track_index_case>::Execute(p2);
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
}


//--------------------------------------------------------------------------
// All explicit instantiations.
//--------------------------------------------------------------------------
template class Infector<LogMode::None, false>;

template class Infector<LogMode::None, true>;

template class Infector<LogMode::Transmissions, false>;

template class Infector<LogMode::Transmissions, true>;

template class Infector<LogMode::Contacts, false>;

template class Infector<LogMode::Contacts, true>;

} // end_of_namespace
