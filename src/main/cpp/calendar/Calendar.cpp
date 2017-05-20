#include "Calendar.h"

#include "util/Errors.h"
#include "util/InstallDirs.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace stride {

using namespace std;
using namespace boost::filesystem;
using namespace stride::util;

void Calendar::AdvanceDay()
{
	m_day++;
	m_date = m_date + boost::gregorian::date_duration(1);
}

void Calendar::Initialize(const boost::gregorian::date& start_date, const string& holidays_file)
{
	// Load the json file
	boost::property_tree::ptree pt_holidays;
	const auto file_path{InstallDirs::GetDataDir() /= holidays_file};
	if (!is_regular_file(file_path)) {
		FATAL_ERROR("Holidays file " + file_path.string() + " not present.");
	}
	read_json(file_path.string(), pt_holidays);
	Initialize(start_date, pt_holidays);
}

void Calendar::Initialize(const boost::gregorian::date& start_date, const boost::property_tree::ptree& holidays_ptree)
{
	// Set the start date.
	m_date = start_date;

	// Read in holidays.
	for (int i = 1; i < 13; i++) {
		const string month{to_string(i)};
		const string year{holidays_ptree.get<string>("year", "2016")};

		// Read in general holidays.
		const string general_key{"general." + month};
		for (auto& date : holidays_ptree.get_child(general_key)) {
			const string date_string{year + "-" + month + "-" + date.second.get_value<string>()};
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			m_holidays.push_back(new_holiday);
		}

		// Read in school holidays.
		const string school_key{"school." + month};
		for (auto& date : holidays_ptree.get_child(school_key)) {
			const string date_string{year + "-" + month + "-" + date.second.get_value<string>()};
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			m_school_holidays.push_back(new_holiday);
		}
	}
}

} // end_of_namespace
