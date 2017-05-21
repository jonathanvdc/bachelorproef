#ifndef CONTACT_PROFILE_H_INCLUDED
#define CONTACT_PROFILE_H_INCLUDED

#include "core/ClusterType.h"
#include "pop/Age.h"

#include <array>
#include <boost/property_tree/ptree.hpp>

namespace stride {

class ContactProfile : public std::array<double, MaximumAge() + 1>
{
public:
	/// Need to keep the default constructor available.
	ContactProfile(){};

	/// Explicitly initialize
	ContactProfile(ClusterType cluster_type, const boost::property_tree::ptree& pt_contacts);
};

} // namespace

#endif // end-of-include-guard
