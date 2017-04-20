#include <algorithm>
#include <fstream>
#include <limits>
#include <boost/tokenizer.hpp>
#include "Profile.h"
#include "geo/GeoPosition.h"
#include "util/StringUtils.h"

namespace stride {
namespace geo {

ProfileRef Profile::Parse(std::ifstream& csv_file)
{
	std::vector<City> cities;
	std::vector<std::string> tokens;
	std::string line;

	// Skip the header line:
	// "city_id","city_name","province","population","x_coord","y_coord","latitude","longitude"
	getline(csv_file, line);

	double min_latitude = +std::numeric_limits<double>::infinity();
	double max_latitude = -std::numeric_limits<double>::infinity();
	double min_longitude = +std::numeric_limits<double>::infinity();
	double max_longitude = -std::numeric_limits<double>::infinity();

	while (getline(csv_file, line)) {
		boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
		tokens.assign(tok.begin(), tok.end());
		double latitude = util::StringUtils::FromString<double>(tokens[6]);
		double longitude = util::StringUtils::FromString<double>(tokens[7]);
		cities.push_back(
		    {util::StringUtils::FromString<CityID>(tokens[0]),     // city_id
		     util::StringUtils::Trim(tokens[1]),		   // city_name
		     util::StringUtils::FromString<ProvinceID>(tokens[2]), // province
		     util::StringUtils::FromString<double>(tokens[3]),     // population
		     util::StringUtils::FromString<double>(tokens[4]),     // x_coord
		     util::StringUtils::FromString<double>(tokens[5]),     // y_coord
		     {latitude, longitude}});

		min_latitude = std::min(min_latitude, latitude);
		max_latitude = std::max(max_latitude, latitude);
		min_longitude = std::min(min_longitude, longitude);
		max_longitude = std::max(max_longitude, longitude);
	}

	return std::make_shared<Profile>(
	    cities, GeoRectangle{min_latitude, max_latitude, min_longitude, max_longitude});
}

} // namespace
} // namespace
