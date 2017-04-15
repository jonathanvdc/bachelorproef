#include <vector>
#include "Household.h"

namespace stride {
namespace population {

std::vector<Household> ParseReferenceHouseholds(const boost::property_tree::ptree& pt)
{
	std::vector<Household> households;
	for (const auto& h : pt) {
		households.push_back(Household{});
		for (const auto& age : h.second) {
			households.back().push_back(age.second.get_value<int>());
		}
	}
	return households;
}

} // namespace population
} // namespace stride
