#ifndef TRAVEL_MODEL_H_INCLUDED
#define TRAVEL_MODEL_H_INCLUDED

/**
 * @file
 * Data structures that describe a model for work travel.
 */

#include <memory>
#include <unordered_set>
#include <vector>

namespace stride {
namespace multiregion {

using RegionId = size_t;

struct Airport;

/**
 * Represents a single route in the airport network.
 */
struct AirRoute final
{
	/// The fraction of passengers which takes this route from the source
	/// airport.
	double passenger_fraction;

	/// The target airport.
	std::shared_ptr<const Airport> target;
};

/**
 * Describes information pertaining to an airport.
 */
struct Airport final
{
	/// The id of the region where this airport is located.
	RegionId region_id;

	std::vector<AirRoute> routes;
};

/**
 * Gets the travel model for a single region.
 */
class RegionTravel final
{
public:
	RegionTravel(RegionId region_id, const std::vector<std::shared_ptr<const Airport>>& all_airports);

	/// Gets the region id for the region this data structure represents.
	RegionId GetRegionId() const { return region_id; }

	/// Gets a list of all airports.
	const std::vector<std::shared_ptr<const Airport>>& GetAllAirports() const { return all_airports; }

	/// Gets a list of airports in the current region.
	const std::vector<std::shared_ptr<const Airport>>& GetLocalAirports() const { return local_airports; }

	/// Gets the set of region ids for regions that have at least one air
	/// route which targets a local airport.
	const std::unordered_set<RegionId>& GetRegionsWithIncomingRoutes() const
	{
		return regions_with_incoming_routes;
	}

private:
	RegionId region_id;
	std::vector<std::shared_ptr<const Airport>> all_airports;
	std::vector<std::shared_ptr<const Airport>> local_airports;
	std::unordered_set<RegionId> regions_with_incoming_routes;
};

} // namespace
} // namespace

#endif // end-of-include-guard