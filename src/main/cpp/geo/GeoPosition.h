#ifndef GEO_GEOPOSITION_H_INCLUDED
#define GEO_GEOPOSITION_H_INCLUDED

// If this flag is enabled, distances between geodesic positions are
// calculated using a faster, but only approximately correct, method.
// #define EQUIRECTANGULAR_APPROXIMATION

#include <cmath>
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

	// Calculate the distance (in kilometres) between two geodesic positions.
	double Distance(const GeoPosition& rhs) const
	{
		// Earth's radius in kilometres.
		const double R = 6371.0;
		const double deg_to_rad = 3.14159265359 / 180.0;
		const double f1 = latitude * deg_to_rad;
		const double f2 = rhs.latitude * deg_to_rad;
		const double l1 = longitude * deg_to_rad;
		const double l2 = rhs.longitude * deg_to_rad;
		const double df = f2 - f1;
		const double dl = l2 - l1;

#ifdef EQUIRECTANGULAR_APPROXIMATION
		const double x = dl * std::cos((f1 + f2) / 2.0);
		const double y = df;
		const double c = std::sqrt(x * x + y * y);
#else
		const double sf = std::sin(df / 2.0);
		const double sl = std::sin(dl / 2.0);
		const double a = sf * sf + std::sin(f1) * std::sin(f2) + sl * sl;
		const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
#endif
		return R * c;
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
