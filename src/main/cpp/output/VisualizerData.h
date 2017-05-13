#ifndef VISUALIZERDATA_H_INCLUDED
#define VISUALIZERDATA_H_INCLUDED

/**
 * @file
 * Data structure for the visualiser.
 */

#include <memory>
#include <string>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "pop/Population.h"

namespace stride {

class VisualizerData
{
private:
	std::vector<std::map<std::string, unsigned int>> days;

public:
	void AddDay(const Population& pop)
	{
		days.push_back({});
		for (const auto& p : pop) {
			if (p.GetHealth().IsInfected()) {
				days.back()[pop.Hometown(p).name]++;
			}
		}
	}

	const std::vector<std::map<std::string, unsigned int>>& GetDays() const { return days; }

	const std::shared_ptr<boost::property_tree::ptree> ToPtree() const
	{
		using boost::property_tree::ptree;

		auto daysTree = std::make_shared<ptree>();

		for (const auto& day : days) {
			ptree data;
			for (const auto& p : day)
				data.put(p.first, p.second);
			daysTree->push_back(std::make_pair("", data));
		}

		return daysTree;
	}
};

} // end of namespace

#endif // end-of-include-guard