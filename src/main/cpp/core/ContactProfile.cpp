#include "ContactProfile.h"

namespace stride {

using namespace std;
using namespace boost::property_tree;

ContactProfile::ContactProfile(ClusterType cluster_type, const ptree& pt_contacts)
{
	const string key = "matrices." + ToString(cluster_type);
	ContactProfile mean_nums;
	unsigned int i = 0U;
	for (const auto& participant : pt_contacts.get_child(key)) {
		double total_contacts = 0;
		for (const auto& contact : participant.second.get_child("contacts")) {
			total_contacts += contact.second.get<double>("rate");
		}
		(*this)[i++] = total_contacts;
	}
}

} // namespace
