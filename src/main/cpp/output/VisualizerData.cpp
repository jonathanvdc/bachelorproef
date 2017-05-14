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
			days.back()[pop.Hometown(p).name]++;
		}
	}
}

const vector<map<string, unsigned int>>& VisualizerData::GetDays() const { return days; }

const shared_ptr<ptree> VisualizerData::ToPtree() const
{
	auto daysTree = make_shared<ptree>();

	for (const auto& day : days) {
		ptree data;
		for (const auto& p : day)
			data.put(p.first, p.second);
		daysTree->push_back(make_pair("", data));
	}

	return daysTree;
}

} // end of namespace