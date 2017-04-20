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

    // Lexicographic ordering.
	bool operator<(const GeoPosition& rhs) const
	{
		return latitude < rhs.latitude || (latitude == rhs.latitude && longitude < rhs.longitude);
	}

};

/// A geodesic "rectangle" (latitude range x longitude range.)
struct GeoRectangle final
{
    /// The minimum geographic latitude of this are.
    double min_latitude;

    /// The maximum geographic latitude of this are.
    double max_latitude;

    /// The minimum geographic longitude of this are.
    double min_longitude;

    /// The maximum geographic longitude of this are.
    double max_longitude;
};

} // namespace
} // namespace

#endif // end-of-include-guard
