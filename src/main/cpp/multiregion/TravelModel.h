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

class RegionTravel;
using RegionTravelRef = std::shared_ptr<const RegionTravel>;

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
	/// Creates a travel model for a single region. This constructor will build
	/// a model where no inter-region travel whatsoever occurs.
	RegionTravel(
	    RegionId region_id, const std::string& region_population_path,
	    const std::string& region_geodistribution_profile_path);

	/// Creates a travel model for a single region.
	RegionTravel(
	    RegionId region_id, const std::string& region_population_path,
	    const std::string& region_geodistribution_profile_path, double travel_fraction,
	    const std::shared_ptr<const std::vector<AirportRef>>& all_airports);

	/// Gets the region id for the region this data structure represents.
	RegionId GetRegionId() const { return region_id; }

	/// Gets the path of the population file for this region.
	std::string GetRegionPopulationPath() const { return region_population_path; }

	/// Gets the path of the geodistribution profile file for this region.
	std::string GetRegionGeodistributionProfilePath() const { return region_geodistribution_profile_path; }

	/// Gets the fraction of people in the region who travel by plane on any given day.
	double GetTravelFraction() const { return travel_fraction; }

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
	/// The identifier for the first region can be specified. All regions are assigned
	/// unique identifiers in the [first_region_id, first_region_id + number_of_regions)
	/// range.
	static std::vector<RegionTravelRef> ParseRegionTravel(
	    const boost::property_tree::ptree& ptree, RegionId first_region_id = 0);

private:
	RegionId region_id;
	std::string region_population_path;
	std::string region_geodistribution_profile_path;
	double travel_fraction;
	std::shared_ptr<const std::vector<AirportRef>> all_airports;
	std::vector<AirportRef> local_airports;
	std::unordered_set<RegionId> regions_with_incoming_routes;
};

} // namespace
} // namespace

#endif // end-of-include-guard