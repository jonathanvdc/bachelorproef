#include "TravelModel.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace multiregion {

RegionTravel::RegionTravel(
    RegionId region_id, const std::string& region_population_path,
    const std::string& region_geodistribution_profile_path)
    : region_id(region_id), region_population_path(region_population_path),
      region_geodistribution_profile_path(region_geodistribution_profile_path), travel_fraction(0.0),
      all_airports(std::make_shared<std::vector<AirportRef>>())
{
}

RegionTravel::RegionTravel(
    RegionId region_id, const std::string& region_population_path,
    const std::string& region_geodistribution_profile_path, double travel_fraction,
    const std::shared_ptr<const std::vector<AirportRef>>& all_airports)
    : region_id(region_id), region_population_path(region_population_path),
      region_geodistribution_profile_path(region_geodistribution_profile_path), travel_fraction(travel_fraction),
      all_airports(all_airports)
{
	for (const auto& airport : *all_airports) {
		if (airport->region_id == region_id) {
			local_airports.push_back(airport);
		} else {
			for (const auto& route : airport->routes) {
				if (route.target->region_id == region_id)
					regions_with_incoming_routes.insert(airport->region_id);
			}
		}
	}
}

std::vector<RegionTravelRef> RegionTravel::ParseRegionTravel(
    const boost::property_tree::ptree& ptree, RegionId first_region_id)
{
	// A region travel model contains:
	//   * regions, which contain
	//     - a fraction of the region's population, and
	//     - airports, which contain
	//       + a name,
	//       + a fraction of the passengers in the region, and
	//       + routes, which contain
	//         * a fraction of the passengers departing from the airport, and
	//         * the name of the target airport.
	//
	// This function parses lists of regions.

	// Create a dictionary that maps airport names to strings.
	std::unordered_map<std::string, std::shared_ptr<Airport>> airport_map;

	// Create a shared list of references to airports.
	auto airport_list = std::make_shared<std::vector<AirportRef>>();

	// Our first order of business is to parse the airports. and the routes that
	// connect them.
	RegionId region_id = first_region_id;
	for (const auto& region_pair : ptree) {
		if (region_pair.first != "region")
			continue;

		const auto& region = region_pair.second;

		for (const auto& airport_pair : region) {
			if (airport_pair.first != "airport")
				continue;

			const auto& airport = airport_pair.second;

			auto airport_name = airport.get<std::string>("<xmlattr>.name");
			auto airport_instance = std::make_shared<Airport>();
			airport_instance->region_id = region_id;
			airport_instance->passenger_fraction = airport.get<double>("<xmlattr>.passenger_fraction", 1.0);
			airport_list->push_back(airport_instance);
			airport_map[airport_name] = airport_instance;
		}
		region_id++;
	}

	// Next, let's parse the routes that connect them.
	for (const auto& region_pair : ptree) {
		if (region_pair.first != "region")
			continue;

		const auto& region = region_pair.second;

		for (const auto& airport_pair : region) {
			if (airport_pair.first != "airport")
				continue;

			const auto& airport = airport_pair.second;

			auto airport_name = airport.get<std::string>("<xmlattr>.name");
			for (const auto& route_pair : airport) {
				if (route_pair.first != "route")
					continue;

				const auto& route = route_pair.second;

				auto route_passenger_fraction = route.get<double>("<xmlattr>.passenger_fraction", 1.0);
				auto route_target_airport = airport_map[route.get_value<std::string>()];
				airport_map[airport_name]->routes.push_back(
				    {route_passenger_fraction, route_target_airport});
			}
		}
	}

	// Create the list of results.
	std::vector<RegionTravelRef> results;

	// Finally, we'll create data structures for the regions.
	region_id = first_region_id;
	for (const auto& region_pair : ptree) {
		if (region_pair.first != "region")
			continue;

		const auto& region = region_pair.second;
		auto region_population_path = region.get<std::string>("<xmlattr>.population_file");
		auto region_geodistribution_profile_path = region.get<std::string>("<xmlattr>.geodistribution_profile", "");
		auto region_travel_fraction = region.get<double>("<xmlattr>.travel_fraction");
		results.push_back(
		    std::make_shared<RegionTravel>(
			region_id, region_population_path, region_geodistribution_profile_path, region_travel_fraction,
			airport_list));
		region_id++;
	}
	return std::move(results);
}

} // namespace
} // namespace