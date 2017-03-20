#ifndef INCLUSIVE_RANGE_H_INCLUDED
#define INCLUSIVE_RANGE_H_INCLUDED

#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace util {

template <typename T>
struct InclusiveRange
{
	InclusiveRange() : InclusiveRange(T(), T()) {}
	InclusiveRange(T min, T max) : minimum(min), maximum(max) {}

	// Parse an inclusive range from a ptree with "minimum" and "maximum" keys.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt)
	{
		minimum = pt.get<T>("minimum");
		maximum = pt.get<T>("maximum");
	}

	// Return the average (central) value for this range. (The result will be truncated if T is an integral type.)
	T Average() const { return (minimum + maximum) / 2; }

	// Is the given argument contained in this range?
	bool Includes(T x) const { return minimum <= x && x <= maximum; }

	T minimum;
	T maximum;
};

} // namespace util
} // namespace stride

#endif
