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
	std::vector<GeoPosition> points;
	std::vector<std::string> tokens;
	std::string line;

	// Skip the header line:
	// "city_id","city_name","province","population","x_coord","y_coord","latitude","longitude"
	getline(csv_file, line);

	while (getline(csv_file, line)) {
		boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
		tokens.assign(tok.begin(), tok.end());
		double latitude = util::StringUtils::FromString<double>(tokens[6]);
		double longitude = util::StringUtils::FromString<double>(tokens[7]);
		cities.push_back(
		    {util::StringUtils::FromString<CityId>(tokens[0]),     // city_id
		     util::StringUtils::Trim(tokens[1]),		   // city_name
		     util::StringUtils::FromString<ProvinceId>(tokens[2]), // province
		     util::StringUtils::FromString<double>(tokens[3]),     // population
		     util::StringUtils::FromString<double>(tokens[4]),     // x_coord
		     util::StringUtils::FromString<double>(tokens[5]),     // y_coord
		     {latitude, longitude}});
		points.push_back({latitude, longitude});
	}

	return std::make_shared<Profile>(cities, HullSampler(points));
}

} // namespace
} // namespace
