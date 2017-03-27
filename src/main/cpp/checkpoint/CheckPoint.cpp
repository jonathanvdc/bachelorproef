/*
 * CheckPoint.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#include "CheckPoint.h"
#include "hdf5.h"


#include <sstream>
#include <string>

namespace stride {
namespace checkpoint {

CheckPoint::CheckPoint(std::string filename, bool create_mode)
{
	if (create_mode) {
		CreateFile(filename);
	} else {
		OpenFile(filename);
	}
}

void CheckPoint::CreateFile(std::string filename)
{
	m_file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	// TODO: add a lot of preknown data such as Holidays, info about disease,...
}

void CheckPoint::OpenFile(std::string filename) { m_file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); }

void CheckPoint::WriteDate(const Calendar& cal)
{
	std::size_t day = cal.GetDay();
	std::size_t month = cal.GetMonth();
	std::size_t year = cal.GetYear();

	std::stringstream ss;
	ss << year << '/' << month << '/' <<day;
	std::string date = ss.str();
	// TODO: write as attribute to dataset
}

void CheckPoint::WriteHolidays(const boost::property_tree::ptree& holidays_ptree)
{
	std::vector<boost::gregorian::date> holidays;
	std::vector<boost::gregorian::date> school_holidays;
	// Read in holidays.
	for (int i = 1; i < 13; i++) {
		const std::string month = std::to_string(i);
		const std::string year = holidays_ptree.get<std::string>("year", "2016");

		// Read in general holidays.
		const std::string general_key = "general." + month;
		for (auto& date : holidays_ptree.get_child(general_key)) {
			const std::string date_string = year + "-" + month + "-" + date.second.get_value<std::string>();
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			holidays.push_back(new_holiday);
		}

		// Read in school holidays.
		const std::string school_key = "school." + month;
		for (auto& date : holidays_ptree.get_child(school_key)) {
			const std::string date_string = year + "-" + month + "-" + date.second.get_value<std::string>();
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			school_holidays.push_back(new_holiday);
		}
	}
	// TODO: write the holidays to the checkpoint file
}

} /* namespace checkpoint */
} /* namespace stride */
