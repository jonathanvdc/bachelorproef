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
 * Implementation file for the WorldEnvironment class.
 */

#include "WorldEnvironment.h"

#include <boost/property_tree/json_parser.hpp>

namespace indismo {

using namespace std;

WorldEnvironment::WorldEnvironment(const boost::property_tree::ptree& pt_config) : m_day(0)
{
	// Set start date
	const string start_date = pt_config.get<string>("run.start_date", "2016-01-01");
	boost::gregorian::date d(boost::gregorian::from_simple_string(start_date));
	m_date = d;

	// Set holidays & school holidays
	const string holidays_file = pt_config.get<string>("run.holidays_file", "./config/holidays_flanders_2016.json") ;
	InitializeHolidays(holidays_file);
}

WorldEnvironment::~WorldEnvironment() {}

void WorldEnvironment::AdvanceDay()
{
	m_day++;     // Advance simulation day
	boost::gregorian::date_duration dd(1);
	m_date = m_date + boost::gregorian::date_duration(1);
}

void WorldEnvironment::InitializeHolidays(std::string holidays_file)
{
	// load json file
	boost::property_tree::ptree pt_holidays;
	read_json(holidays_file, pt_holidays);

	// read in holidays
	for (int i = 1; i < 13; i++) {
		const string month = std::to_string(i);
		const string year = pt_holidays.get<std::string>("year", "2016");

		// read in general holidays
		const string general_key = "general." + month;
		for (auto& date: pt_holidays.get_child(general_key)) {
			std::string day = date.second.get_value<std::string>();
			std::string date_string = year + "-" + month + "-" + day;
			boost::gregorian::date new_holiday = boost::gregorian::from_simple_string(date_string);
			m_holidays.push_back(new_holiday);
		}

		// read in school holidays
		const string school_key = "school." + month;
		for (auto& date: pt_holidays.get_child(school_key)) {
			std::string day = date.second.get_value<std::string>();
			std::string date_string = year + "-" + month + "-" + day;
			boost::gregorian::date new_holiday = boost::gregorian::from_simple_string(date_string);
			m_school_holidays.push_back(new_holiday);
		}
	}
}

} // end_of_namespace
