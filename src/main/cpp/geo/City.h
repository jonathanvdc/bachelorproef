#ifndef GEO_CITY_H_INCLUDED
#define GEO_CITY_H_INCLUDED

#include <string>
#include "GeoPosition.h"

namespace stride {
namespace geo {

using CityId = int;
using ProvinceId = int;

/// A city, as part of a geodistribution profile.
struct City final
{
	/// The city's ID.
	CityId id;

	/// The city's name.
	std::string name;

	/// The ID of the province this city is in.
	ProvinceId province_id;

	/// The relative population count of this city. Profile-wide, these add up to 1.
	double relative_population;

	/// The Cartesian x-coordinate of this city.
	double x_coord;

	/// The Cartesian y-coordinate of this city.
	double y_coord;

	/// The geographic position of this city.
	GeoPosition geo_position;
};

} // namespace
} // namespace

#endif // end-of-include-guard
