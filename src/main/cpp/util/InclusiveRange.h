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
	static InclusiveRange Parse(const boost::property_tree::ptree& pt)
	{
		return InclusiveRange(pt.get<T>("minimum"), pt.get<T>("maximum"));
	}

	// Return the average (central) value for this range. (The result will be truncated if T is an integral type.)
	T Average() const { return (minimum + maximum) / 2; }

	// Is the given argument contained in this range?
	bool Includes(T x) const { return minimum <= x && x <= maximum; }

	// Lexicographic ordering.
	bool operator<(const InclusiveRange<T>& rhs) const
	{
		return minimum < rhs.minimum || (minimum == rhs.minimum && maximum < rhs.maximum);
	}

	T minimum;
	T maximum;
};

} // namespace util
} // namespace stride

#endif
