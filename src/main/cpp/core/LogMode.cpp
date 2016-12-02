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

#include "LogMode.h"

#include <boost/algorithm/string.hpp>
#include <map>
#include <string>

using namespace std;
using boost::to_upper;

namespace {

using indismo::LogMode;

map<LogMode, string> g_cluster_type_name {
                make_pair(LogMode::None,             "None"),
                make_pair(LogMode::Transmissions,    "Transmissions"),
                make_pair(LogMode::Contacts,         "Contacts")
};

map<string, LogMode> g_name_cluster_type {
                make_pair("NONE",            LogMode::None),
                make_pair("TRANSMISSIONS",   LogMode::Transmissions),
                make_pair("CONTACTS",        LogMode::Contacts)
};

}

namespace indismo {

string ToString(LogMode l)
{
	string ret = "Unknown";
	if (g_cluster_type_name.count(l) == 1)  {
		ret = g_cluster_type_name[l];
	}
	return ret;
}

bool IsLogMode(string s)
{
        to_upper(s);
        return (g_name_cluster_type.count(s) == 1);
}

LogMode ToLogMode(string s)
{
	return g_name_cluster_type[s];
}

} // namespace
