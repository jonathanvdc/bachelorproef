#include <fstream>
#include <boost/tokenizer.hpp>
#include "Profile.h"
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

	while (getline(csv_file, line)) {
		boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
		tokens.assign(tok.begin(), tok.end());
		cities.push_back(
		    {
			util::StringUtils::FromString<CityID>(tokens[0]),     // city_id
			util::StringUtils::Trim(tokens[1]),		      // city_name
			util::StringUtils::FromString<ProvinceID>(tokens[2]), // province
			util::StringUtils::FromString<double>(tokens[3]),     // population
			util::StringUtils::FromString<double>(tokens[4]),     // x_coord
			util::StringUtils::FromString<double>(tokens[5]),     // y_coord
			util::StringUtils::FromString<double>(tokens[6]),     // latitude
			util::StringUtils::FromString<double>(tokens[7]),     // longitude
		    });
	}

	return std::make_shared<Profile>(cities);
}

} // namespace
} // namespace
