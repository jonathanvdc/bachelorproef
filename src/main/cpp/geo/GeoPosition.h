#ifndef GEO_GEOPOSITION_H_INCLUDED
#define GEO_GEOPOSITION_H_INCLUDED

#include <cmath>
#include <string>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/register/multi_point.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include "util/Random.h"

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

	std::string ToString() const
	{
		std::string ns = latitude > 0 ? std::to_string(latitude) + "N" : std::to_string(-latitude) + "S";
		std::string ew = longitude > 0 ? std::to_string(longitude) + "E" : std::to_string(-longitude) + "W";
		return ns + " " + ew;
	}

	// Calculate the distance (in kilometres) between two geodesic positions on Earth.
	double Distance(const GeoPosition& rhs) const
	{
		using CS = boost::geometry::cs::spherical_equatorial<boost::geometry::degree>;
		using EarthPoint = boost::geometry::model::point<double, 2, CS>;

		// Earth's radius in kilometres.
		const double earth_radius = 6371.0;
		EarthPoint p1(longitude, latitude);
		EarthPoint p2(rhs.longitude, rhs.latitude);
		return earth_radius * boost::geometry::distance(p1, p2);
	}

	double GetLatitude() const { return latitude; }
	double GetLongitude() const { return longitude; }
	void SetLatitude(double in) { latitude = in; }
	void SetLongitude(double in) { longitude = in; }
};
}
}

// Q: Why is GeoPosition registered as boost::geometry::cs::cartesian?
// A: Boost's convex hull algorithm doesn't work for geographic points (in
//    fact, such an algorithm is pretty tricky to write). But pretending
//    (lat, long) pairs are (x, y) is a close enough approximation for the
//    areas Stride users seem to simulate: Belgium and the continental US.

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(
    stride::geo::GeoPosition, double, boost::geometry::cs::cartesian, //
    stride::geo::GeoPosition::GetLatitude, stride::geo::GeoPosition::GetLongitude,
    stride::geo::GeoPosition::SetLatitude, stride::geo::GeoPosition::SetLongitude)

BOOST_GEOMETRY_REGISTER_MULTI_POINT(std::vector<stride::geo::GeoPosition>)

namespace stride {
namespace geo {

using GeoPolygon = boost::geometry::model::polygon<GeoPosition>;
using GeoBox = boost::geometry::model::box<GeoPosition>;

class HullSampler
{
public:
	HullSampler(const std::vector<GeoPosition>& points)
	{
		boost::geometry::convex_hull(points, hull);
		boost::geometry::envelope(hull, box);
	}

	GeoPosition Sample(util::Random& random) const
	{
		GeoPosition point;
		do {
			GeoPosition a = box.min_corner();
			GeoPosition b = box.max_corner();
			double ax = a.latitude, ay = a.longitude;
			double bx = b.latitude, by = b.longitude;
			double t = random.NextDouble();
			double u = random.NextDouble();
			point.latitude = ax * t + bx * (1.0 - t);
			point.longitude = ay * u + by * (1.0 - u);
		} while (!boost::geometry::within(point, hull));

		return point;
	}

private:
	GeoPolygon hull;
	GeoBox box;
};

} // namespace
} // namespace

#endif // end-of-include-guard
