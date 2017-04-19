#ifndef GEO_GEOPOSITION_H_INCLUDED
#define GEO_GEOPOSITION_H_INCLUDED

#include <string>

namespace stride {
namespace geo {

/// A geodesic position (latitude-longitude point).
struct GeoPosition final
{
	/// The geographic latitude of this position.
	double latitude;

	/// The geographic longitude of this position.
	double longitude;
};

} // namespace
} // namespace

#endif // end-of-include-guard
