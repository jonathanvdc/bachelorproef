#ifndef ATLAS_H_INCLUDED
#define ATLAS_H_INCLUDED

/**
 * @file
 * Header for the Atlas class.
 */

#include <cstddef>
#include <map>
#include <utility>
#include "core/ClusterType.h"
#include "geo/GeoPosition.h"

namespace stride {

/**
 * A map from clusters, identified by their ID and type, to geopositions.
 */
class Atlas
{
public:
	using Key = std::pair<std::size_t, ClusterType>;
	using Map = std::map<Key, geo::GeoPosition>;

	Atlas() {}
	Atlas(const Map& map) : map(map) {}

	/// Look up a cluster's position. If it isn't found, throw std::out_of_range.
	const geo::GeoPosition& Lookup(const Key& key) const { return map.at(key); }

	/// Store a GeoPosition in the atlas.
	auto Emplace(const Atlas::Key& key, const geo::GeoPosition& pos) -> decltype(Map().emplace(key, pos))
	{
		return map.emplace(key, pos);
	}

	Map map;
};

} // end_of_namespace

#endif // include-guard
