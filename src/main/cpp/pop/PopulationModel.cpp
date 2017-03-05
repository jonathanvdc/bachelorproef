#include "PopulationModel.h"
#include <iostream>

namespace stride {
namespace population_model {

void Age::parse(const boost::property_tree::ptree& pt) {
	maximum = pt.get<int>("maximum");
	elbow = pt.get<int>("elbow");
}

void FamilyAge::parse(const boost::property_tree::ptree& pt) {
	live_alone_minimum = pt.get<int>("live_alone_minimum");
	child_maximum = pt.get<int>("child_maximum");
	parent.parse(pt.get_child("parent"));
}

void FamilyAgeGap::parse(const boost::property_tree::ptree& pt) {
	child_parent_minimum = pt.get<int>("child_parent_minimum");
	parent.parse(pt.get_child("parent"));
	child.parse(pt.get_child("child"));
}

void Family::parse(const boost::property_tree::ptree& pt) {
	age.parse(pt.get_child("age"));
	age_gap.parse(pt.get_child("age_gap"));

	// XML doesn't really have lists, so this is stored kinda hackily:
	//
	//     <size_distribution>
	//         <p>12</p> <p>27</p> <p>20</p> ...
	//     </size_distribution
	//
	for (const auto &i : pt.get_child("size_distribution")) {
		if (i.first == "p") {
			size_distribution.emplace_back(i.second.get_value<int>());
		}
	}
}

void SchoolAge::parse(const boost::property_tree::ptree& pt) {
	kindergarten = pt.get<int>("kindergarten");
	primary_school = pt.get<int>("primary_school");
	secondary_school = pt.get<int>("secondary_school");
	higher_eduation = pt.get<int>("higher_education");
	graduation = pt.get<int>("graduation");
}

void School::parse(const boost::property_tree::ptree& pt) {
	age.parse(pt.get_child("age"));
	p_higher_education = pt.get<double>("p_higher_education");
	size.parse(pt.get_child("size"));
}

void Work::parse(const boost::property_tree::ptree& pt) {
	age.parse(pt.get_child("age"));
	p_employed = pt.get<double>("p_employed");
	size.parse(pt.get_child("size"));
}

void Community::parse(const boost::property_tree::ptree& pt) {
	size.parse(pt.get_child("size"));
}

void Model::parse(const boost::property_tree::ptree& pt) {
	auto root = pt.get_child("population_model");

	age.parse(root.get_child("age"));
	family.parse(root.get_child("family"));
	school.parse(root.get_child("school"));
	work.parse(root.get_child("work"));
	community.parse(root.get_child("community"));
	size.parse(root.get_child("size"));
}

} // namespace population_model
} // namespace stride
