#ifndef GEO_CITY_H_INCLUDED
#define GEO_CITY_H_INCLUDED

#include <string>

namespace stride {
namespace geo {

using CityID = int;
using ProvinceID = int;

/// A city, as part of a geodistribution profile.
struct City final
{
	/// The city's ID.
	CityID id;

	/// The city's name.
	std::string name;

	/// The ID of the province this city is in.
	ProvinceID province_id;

	/// The *model* population count of this city.
	int population;

	/// The Cartesian x-coordinate of this city.
	double x_coord;

	/// The Cartesian y-coordinate of this city.
	double y_coord;

	/// The geographic latitude of this city.
	double latitude;

	/// The geographic longitude of this city.
	double longitude;
};

} // namespace
} // namespace

#endif // end-of-include-guard
