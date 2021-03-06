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
 * Implementation file for the Calendar class.
 */

#include "Calendar.h"

#include "util/InstallDirs.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

Calendar::Calendar(const boost::property_tree::ptree& pt_config)
        : m_day(0)
{
	// Set start date
	const string start_date { pt_config.get<string>("run.start_date", "2016-01-01") };
	m_date = boost::gregorian::from_simple_string(start_date);

	// Set holidays & school holidays
	InitializeHolidays(pt_config);
}

void Calendar::AdvanceDay()
{
	m_day++;
	m_date = m_date + boost::gregorian::date_duration(1);
}

void Calendar::InitializeHolidays(const boost::property_tree::ptree& pt_config)
{
	// Load json file
	boost::property_tree::ptree pt_holidays;
	{
	        const auto file_name { pt_config.get<string>("run.holidays_file", "holidays_flanders_2016.json") };
	        const auto file_path { InstallDirs::GetDataDir() /= file_name };
	        if ( !is_regular_file(file_path) ) {
	                throw runtime_error(string(__func__) + "Holidays file " + file_path.string() + " not present.");
	        }
	        read_json(file_path.string(), pt_holidays);
        }

	// Read in holidays
	for (int i = 1; i < 13; i++) {
		const string month { to_string(i) };
		const string year  { pt_holidays.get<string>("year", "2016") };

		// read in general holidays
		const string general_key { "general." + month };
		for (auto& date: pt_holidays.get_child(general_key)) {
			const string date_string { year + "-" + month + "-" + date.second.get_value<string>() };
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			m_holidays.push_back(new_holiday);
		}

		// read in school holidays
		const string school_key { "school." + month };
		for (auto& date: pt_holidays.get_child(school_key)) {
			const string date_string { year + "-" + month + "-" + date.second.get_value<string>() };
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			m_school_holidays.push_back(new_holiday);
		}
	}
}

} // end_of_namespace
