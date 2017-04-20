#ifndef GEO_PROFILE_H_INCLUDED
#define GEO_PROFILE_H_INCLUDED

#include <fstream>
#include <memory>
#include "City.h"

namespace stride {
namespace geo {

class Profile;
using ProfileRef = std::shared_ptr<const Profile>;

class Profile
{
public:
	Profile(const std::vector<City>& cities, const GeoRectangle& simulation_area) : cities(cities), simulation_area(simulation_area) {}

	/// Return a list of cities in this geodistribution profile.
	const std::vector<City>& GetCities() const { return cities; }

	/// Return the simulation area, which is defined as the the latitude/longitude
	/// bounding geodesic rectangle of all the cities.
	const GeoRectangle GetSimulationArea() const { return simulation_area; }

	static ProfileRef Parse(std::ifstream& csv_file);

private:
	std::vector<City> cities;
	GeoRectangle simulation_area;
};

} // namespace
} // namespace

#endif // end-of-include-guard
