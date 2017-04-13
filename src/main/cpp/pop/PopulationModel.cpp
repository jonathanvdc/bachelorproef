#include <iostream>
#include "PopulationModel.h"

namespace stride {
namespace population_model {

void Model::parse(const boost::property_tree::ptree& pt)
{
	auto root = pt.get_child("population_model");

	school_size = root.get<int>("school_size");
	school_cluster_size = root.get<int>("school_cluster_size");
	college_size = root.get<int>("college_size");
	college_cluster_size = root.get<int>("college_cluster_size");
	workplace_size = root.get<int>("workplace_size");
	community_size = root.get<int>("community_size");
	school_radius = root.get<double>("school_radius");
}

} // namespace population_model
} // namespace stride
