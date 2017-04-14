#include <iostream>
#include "PopulationModel.h"

namespace stride {
namespace population_model {

ModelRef Model::Parse(const boost::property_tree::ptree& pt)
{
	auto root = pt.get_child("population_model");
	std::vector<TownPivot> town_pivots;
	for (const auto& p : root.get_child("town_pivots")) {
		town_pivots.push_back({util::InclusiveRange<int>::Parse(p.second), p.second.get<double>("probability")});
	}

	return std::make_shared<Model>(
		root.get<int>("school_size"),
		root.get<int>("school_cluster_size"),
		root.get<int>("college_size"),
		root.get<int>("college_cluster_size"),
		root.get<int>("workplace_size"),
		root.get<int>("community_size"),
		root.get<double>("school_radius"),
		root.get<int>("population_size"),
		root.get<double>("city_ratio"),
		town_pivots,
		util::InclusiveRange<int>::Parse(root.get_child("school_age")),
		util::InclusiveRange<int>::Parse(root.get_child("college_age")),
		root.get<double>("college_ratio"),
		root.get<double>("college_commute_ratio"),
		util::InclusiveRange<int>::Parse(root.get_child("employable_age")),
		root.get<double>("employed_ratio"),
		root.get<double>("work_commute_ratio")
	);
}

} // namespace population_model
} // namespace stride
