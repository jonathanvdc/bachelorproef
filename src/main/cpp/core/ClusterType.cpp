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
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of ClusterType.
 */

#include "ClusterType.h"

#include <boost/algorithm/string.hpp>
#include <map>
#include <string>
#include <iostream>

using namespace std;
using boost::to_upper;

namespace {

using stride::ClusterType;

map<ClusterType, string> g_cluster_type_name {
                make_pair(ClusterType::Household,      "household"),
                make_pair(ClusterType::School,         "school"),
                make_pair(ClusterType::Work,           "work"),
                make_pair(ClusterType::HomeDistrict,   "home_district"),
                make_pair(ClusterType::DayDistrict,    "day_district"),
                make_pair(ClusterType::Null,           "null")
};

map<string, ClusterType> g_name_cluster_type {
                make_pair("HOUSEHOLD",            ClusterType::Household),
                make_pair("SCHOOL",               ClusterType::School),
                make_pair("WORK",                 ClusterType::Work),
                make_pair("HOME_DISTRICT",        ClusterType::HomeDistrict),
                make_pair("DAY_DISTRICT",         ClusterType::DayDistrict),
                make_pair("NULL",                 ClusterType::Null)
};

}

namespace stride {

string ToString(ClusterType c)
{
	return (g_cluster_type_name.count(c) == 1)  ? g_cluster_type_name[c] : "Null";
}

bool IsClusterType(const string& s)
{
        std::string t {s};
        to_upper(t);
        return (g_name_cluster_type.count(t) == 1);
}

ClusterType ToClusterType(const string& s)
{
        std::string t {s};
        to_upper(t);
        return (g_name_cluster_type.count(s) == 1) ? g_name_cluster_type[s] : ClusterType::Null;
}

} // namespace
