#ifndef TRAVEL_MODEL_H_INCLUDED
#define TRAVEL_MODEL_H_INCLUDED

/**
 * @file
 * Data structures that describe a model for work travel.
 */

#include <memory>
#include <unordered_set>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace multiregion {

using RegionId = size_t;

struct Airport;
using AirportRef = std::shared_ptr<const Airport>;

/**
 * Represents a single route in the airport network.
 */
struct AirRoute final
{
	/// The fraction of passengers from the source airport which
	/// take this route.
	double passenger_fraction;

	/// The target airport.
	AirportRef target;
};

/**
 * Describes information pertaining to an airport.
 */
struct Airport final
{
	/// The id of the region where this airport is located.
	RegionId region_id;

	/// The fraction of passengers in the region that use this airport.
	double passenger_fraction;

	/// Gets the list of all outgoing routes that start at this airport.
	std::vector<AirRoute> routes;
};

/**
 * Gets the travel model for a single region.
 */
class RegionTravel final
{
public:
	RegionTravel(
	    RegionId region_id, double population_fraction,
	    const std::shared_ptr<const std::vector<AirportRef>>& all_airports);

	/// Gets the region id for the region this data structure represents.
	RegionId GetRegionId() const { return region_id; }

	/// Gets the fraction of people in the region who travel by plane on any given day.
	double GetPopulationFraction() const { return population_fraction; }

	/// Gets a list of all airports.
	const std::vector<AirportRef>& GetAllAirports() const { return *all_airports; }

	/// Gets a list of airports in the current region.
	const std::vector<AirportRef>& GetLocalAirports() const { return local_airports; }

	/// Gets the set of region ids for regions that have at least one air
	/// route which targets a local airport.
	const std::unordered_set<RegionId>& GetRegionsWithIncomingRoutes() const
	{
		return regions_with_incoming_routes;
	}

	/// Parses a ptree that contains a vector of travel information for regions.
	static std::vector<RegionTravel> ParseRegionTravel(boost::property_tree::ptree& ptree);

private:
	RegionId region_id;
	double population_fraction;
	std::shared_ptr<const std::vector<AirportRef>> all_airports;
	std::vector<AirportRef> local_airports;
	std::unordered_set<RegionId> regions_with_incoming_routes;
};

} // namespace
} // namespace

#endif // end-of-include-guard