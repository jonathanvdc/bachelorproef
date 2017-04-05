#ifndef GEO_PROFILE_H_INCLUDED
#define GEO_PROFILE_H_INCLUDED

#include "City.h"
#include <memory>
#include <fstream>

namespace stride {
namespace geo {

class Profile;
using ProfileRef = std::shared_ptr<const Profile>;

class Profile
{
public:
	Profile(std::vector<City> cities) : cities(cities) {}

	/// Return a list of cities in this geodistribution profile.
	const std::vector<City>& GetCities() const { return cities; }

	/// Find the sum of the population of all cities in the profile.
	int TotalCityPopulation() const
	{
		int sum = 0;
		for (const auto& c : cities) {
			sum += c.population;
		}
		return sum;
	}

	static ProfileRef Parse(std::ifstream& csv_file);

private:
	std::vector<City> cities;
};

} // namespace
} // namespace

#endif // end-of-include-guard
