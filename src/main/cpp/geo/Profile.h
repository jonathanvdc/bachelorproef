#ifndef GEO_PROFILE_H_INCLUDED
#define GEO_PROFILE_H_INCLUDED

#include <fstream>
#include <memory>
#include "City.h"
#include "util/Random.h"

namespace stride {
namespace geo {

class Profile;
using ProfileRef = std::shared_ptr<const Profile>;

class Profile
{
public:
	Profile(const std::vector<City>& cities, const HullSampler& hull_sampler)
	    : m_cities(cities), m_hull_sampler(hull_sampler)
	{
		std::sort(m_cities.begin(), m_cities.end(), [](const City& a, const City& b) {
			return a.relative_population > b.relative_population;
		});
	}

	/// Return a list of cities in this geodistribution profile.
	/// The result is sorted in descending order of relative population.
	const std::vector<City>& GetCities() const { return m_cities; }

	/// Get a random geoposition in the simulation area.
	GeoPosition GetRandomGeoPosition(util::Random& random) const { return m_hull_sampler.Sample(random); }

	static ProfileRef Parse(std::ifstream& csv_file);

private:
	std::vector<City> m_cities;
	HullSampler m_hull_sampler;
};

} // namespace
} // namespace

#endif // end-of-include-guard
