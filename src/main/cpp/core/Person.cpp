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

#include "Age.h"
#include "Person.h"

#include "core/ClusterType.h"
#include "sim/Calendar.h"

#include <stdexcept>
#include <string>
#include <memory>

namespace stride {

using namespace std;

unsigned int Person::GetClusterId(ClusterType cluster_type) const
{
        switch (cluster_type) {
                case ClusterType::Household:        return m_household_id;
                case ClusterType::School:			return m_school_id;
                case ClusterType::Work:             return m_work_id;
                case ClusterType::HomeDistrict:     return m_home_district_id;
                case ClusterType::DayDistrict:      return m_day_district_id;
                default: throw runtime_error(string(__func__)  + "> Should not reach default.");
        }
}

bool Person::IsInCluster(ClusterType c) const
{
        switch(c) {
                case ClusterType::Household:         return m_in_household;
                case ClusterType::School:			 return m_in_day_cluster;
                case ClusterType::Work:              return m_in_day_cluster;
                case ClusterType::HomeDistrict:      return m_in_home_district;
                case ClusterType::DayDistrict:       return m_in_day_district;
                default: throw runtime_error(string(__func__)  + "> Should not reach default.");
        }
}

void Person::Update(shared_ptr<const Calendar> calendar)
{
        m_health.Update();

        // Update presence in clusters.
        if (calendar->IsWeekend() || (m_age <= MinAdultAge() && calendar->IsSchoolHoliday()) || calendar->IsHoliday()) {
                m_in_day_cluster   = false;
                m_in_day_district  = false;
                m_in_home_district = true;
        } else {
                m_in_day_cluster   = true;
                m_in_day_district  = true;
                m_in_home_district = false;
        }
}

} // end_of_namespace
