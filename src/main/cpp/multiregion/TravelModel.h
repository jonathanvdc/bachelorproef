#ifndef TRAVEL_MODEL_H_INCLUDED
#define TRAVEL_MODEL_H_INCLUDED

/**
 * @file
 * Data structures that describe a model for work travel.
 */

#include <memory>
#include <unordered_set>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace multiregion {

using RegionId = std::size_t;

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
	const Airport* target;
};

/**
 * Describes an airport's region ID and passenger fraction.
 */
struct AirportDescription final
{
	/// The id of the region where this airport is located.
	RegionId region_id;

	/// The fraction of passengers in the region that use this airport.
	double passenger_fraction;
};

/**
 * Describes an airport's region ID, passenger fraction and routes.
 */
struct Airport final
{
	/// The id of the region where this airport is located.
	RegionId region_id;

	/// The fraction of passengers in the region that use this airport.
	double passenger_fraction;

	/// Gets this airport's description.
	AirportDescription GetDescription() const { return {region_id, passenger_fraction}; }

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
	    const std::string& region_geodistribution_profile_path,
	    const std::string& region_reference_households_path);

	/// Creates a travel model for a single region.
	RegionTravel(
	    RegionId region_id, const std::string& region_population_path,
	    const std::string& region_geodistribution_profile_path, const std::string& region_reference_households_path,
	    double travel_fraction, std::size_t min_travel_duration, std::size_t max_travel_duration,
	    const std::shared_ptr<const std::vector<AirportRef>>& all_airports);

	/// Gets the region id for the region this data structure represents.
	RegionId GetRegionId() const { return region_id; }

	/// Gets the path of the population file for this region.
	std::string GetRegionPopulationPath() const { return region_population_path; }

	/// Gets the path of the geodistribution profile file for this region.
	std::string GetRegionGeodistributionProfilePath() const { return region_geodistribution_profile_path; }

	/// Gets the path of the geodistribution profile file for this region.
	std::string GetRegionReferenceHouseholdsPath() const { return region_reference_households_path; }

	/// Gets the fraction of people in the region who travel by plane on any given day.
	double GetTravelFraction() const { return travel_fraction; }

	/// Gets the minimal duration of a trip abroad, in days.
	std::size_t GetMinTravelDuration() const { return min_travel_duration; }

	/// Gets the maximal duration of a trip abroad, in days.
	std::size_t GetMaxTravelDuration() const { return max_travel_duration; }

	/// Gets a list of all airports.
	const std::vector<AirportRef>& GetAllAirports() const { return *all_airports; }

	/// Gets a list of airports in the current region.
	const std::vector<AirportRef>& GetLocalAirports() const { return local_airports; }

	/// Gets the set of region ids for regions that are connected with this region
	/// by an air route.
	const std::unordered_set<RegionId>& GetConnectedRegions() const { return connected_regions; }

	/// Parses a ptree that contains a vector of travel information for regions.
	/// The identifier for the first region can be specified. All regions are assigned
	/// unique identifiers in the [first_region_id, first_region_id + number_of_regions)
	/// range.
	static std::vector<RegionTravelRef> ParseRegionTravel(
	    const boost::property_tree::ptree& ptree, RegionId first_region_id = 0);

	using BoostEdgeWeightProperty = boost::property<boost::edge_weight_t, double>;
	using BoostGraph = boost::adjacency_list<
	    boost::vecS, boost::vecS, boost::directedS, AirportDescription, BoostEdgeWeightProperty>;

	/// Creates a boost graph that is equivalent to the graph defined by this travel model.
	BoostGraph ToBoostGraph() const;

private:
	RegionId region_id;
	std::string region_population_path;
	std::string region_geodistribution_profile_path;
	std::string region_reference_households_path;
	double travel_fraction;
	std::size_t min_travel_duration;
	std::size_t max_travel_duration;
	std::shared_ptr<const std::vector<AirportRef>> all_airports;
	std::vector<AirportRef> local_airports;
	std::unordered_set<RegionId> connected_regions;
};

} // namespace
} // namespace

#endif // end-of-include-guard