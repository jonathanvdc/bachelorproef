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

#include "Person.h"

#include "ClusterType.h"
#include "sim/WorldEnvironment.h"
#include "util/TrackIndexCase.h"
#include "spdlog/spdlog.h"

#include <cstddef>

namespace indismo {

using namespace std;

bool Person::IsInCluster(ClusterType c) const
{
        switch(c) {
                case ClusterType::Household:
                        return m_in_household; break;
                case ClusterType::HomeDistrict:
                        return m_in_home_district; break;
                case ClusterType::School:
                case ClusterType::Work:
                        return m_in_day_cluster; break;
                case ClusterType::DayDistrict:
                        return m_in_day_district; break;
                default:
                        return false;
        }
}

void Person::LogContact(shared_ptr<spdlog::logger> logger,
                const Person* p2, ClusterType cluster_type,
                shared_ptr<const WorldEnvironment> world_environ)
{
        unsigned int home   = (cluster_type == ClusterType::Household);
        unsigned int work   = (cluster_type == ClusterType::Work);
        unsigned int school = (cluster_type == ClusterType::School);
        unsigned int other  = (cluster_type == ClusterType::HomeDistrict || cluster_type == ClusterType::DayDistrict);

        logger->info("[CONT] {} {} {} {} {} {} {} {}",
                m_id, m_age, p2->GetAge(), home, work, school, other, world_environ->GetSimulationDay());

}

void Person::LogTransmission(shared_ptr<spdlog::logger> logger,
                const Person* p2, ClusterType cluster_type,
                shared_ptr<const WorldEnvironment> world_environ)
{
        logger->info("[TRAN] {} {} {} {}",
                m_id, p2->GetId(), ToString(cluster_type), world_environ->GetSimulationDay());
}

void Person::Update(shared_ptr<const WorldEnvironment> world_environ)
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
                if (GetDiseaseCounter() == m_start_symptomatic) {
                        m_symptomatic = true;
                }
                if (GetDiseaseCounter() == m_end_symptomatic) {
                        m_symptomatic = false;
                        if (!m_infectious){
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

} // end_of_namespace
