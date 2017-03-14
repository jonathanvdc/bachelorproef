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

	T minimum;
	T maximum;
};

} // namespace util
} // namespace stride

#endif
