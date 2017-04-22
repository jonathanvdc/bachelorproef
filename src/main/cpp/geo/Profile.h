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
	Profile(const std::vector<City>& cities, const GeoRectangle& simulation_area)
	    : m_cities(cities), m_simulation_area(simulation_area)
	{
		std::sort(m_cities.begin(), m_cities.end(), [](const City& a, const City& b) {
			return a.relative_population > b.relative_population;
		});
	}

	/// Return a list of cities in this geodistribution profile.
	/// The result is sorted in descending order of relative population.
	const std::vector<City>& GetCities() const { return m_cities; }

	/// Return the simulation area, which is defined as the the latitude/longitude
	/// bounding geodesic rectangle of all the cities.
	const GeoRectangle& GetSimulationArea() const { return m_simulation_area; }

	static ProfileRef Parse(std::ifstream& csv_file);

private:
	std::vector<City> m_cities;
	GeoRectangle m_simulation_area;
};

} // namespace
} // namespace

#endif // end-of-include-guard
