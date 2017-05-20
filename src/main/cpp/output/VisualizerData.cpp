/**
 * @file
 * Implementation of the VisualizerData class.
 */

#include "VisualizerData.h"

namespace stride {

using namespace std;
using boost::property_tree::ptree;

void VisualizerData::AddDay(const Population& pop)
{
	days.push_back({});
	for (const auto& p : pop) {
		if (p.GetHealth().IsInfected()) {
			days.back()[pop.GetHometown(p).id]++;
		}
	}
}

vector<map<size_t, unsigned int>>& VisualizerData::GetDays() const { return days; }

shared_ptr<ptree> VisualizerData::ToPtree() const
{
	auto daysTree = make_shared<ptree>();

	// Create a list: [{townId: infected}]
	for (const auto& day : days) {
		ptree data;
		for (const auto& p : day)
			data.put(to_string(p.first), p.second);
		daysTree->push_back(make_pair("", data));
	}

	return daysTree;
}

} // end of namespace