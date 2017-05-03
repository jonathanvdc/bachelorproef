#include <vector>
#include "Household.h"

namespace stride {
namespace population {

std::shared_ptr<std::vector<ReferenceHousehold>> ParseReferenceHouseholds(const boost::property_tree::ptree& pt)
{
	auto households = std::make_shared<std::vector<ReferenceHousehold>>();
	for (const auto& h : pt) {
		households->push_back(ReferenceHousehold());
		for (const auto& age : h.second) {
			households->back().ages.push_back(age.second.get_value<int>());
		}
	}
	return households;
}

} // namespace population
} // namespace stride
