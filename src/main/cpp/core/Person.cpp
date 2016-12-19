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

#include "core/ClusterType.h"
#include "sim/WorldEnvironment.h"

#include <memory>

namespace stride {

using namespace std;

unsigned int Person::GetClusterId(ClusterType cluster_type) const
{
        switch (cluster_type) {
                case ClusterType::Household:
                        return m_household;
                case ClusterType::School:
                case ClusterType::Work:
                        return m_day_cluster;
                case ClusterType::HomeDistrict:
                        return m_home_district;
                case ClusterType::DayDistrict:
                        return m_day_district;
                default:
                        return -1;
        }
}

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

void Person::Update(shared_ptr<const WorldEnvironment> world_environ)
{
        m_health.Update();

        // update presence in clusters
        if (m_age > 18) { // adult
                if (world_environ->IsHoliday() || world_environ->IsWeekend()) {
                        m_in_day_cluster   = false;
                        m_in_day_district  = false;
                        m_in_home_district = true;
                } else {
                        m_in_day_cluster   = true;
                        m_in_day_district  = true;
                        m_in_home_district = false;
                }
        } else { // kid, so look at school holidays too
                if (world_environ->IsHoliday() || world_environ->IsSchoolHoliday() || world_environ->IsWeekend()) {
                        m_in_day_cluster   = false;
                        m_in_day_district  = false;
                        m_in_home_district = true;
                } else {
                        m_in_day_cluster   = true;
                        m_in_day_district  = true;
                        m_in_home_district = false;
                }
        }
}

} // end_of_namespace
