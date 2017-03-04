#include "PopulationModel.h"
#include <iostream>

namespace stride {

PopulationModel::PopulationModel(const boost::property_tree::ptree& pt) {
	maximum_age = pt.get<int>("popgen.age.maximum");
	elbow_age = pt.get<int>("popgen.age.elbow");

	live_alone_minimum_age = pt.get<int>("popgen.family.age.live_alone_minimum");
	child_maximum_age = pt.get<int>("popgen.family.age.child_maximum");
	parent_minimum_age = pt.get<int>("popgen.family.age.parent_minimum");
	parent_maximum_age = pt.get<int>("popgen.family.age.parent_maximum");
	child_parent_minimum_age_gap = pt.get<int>("popgen.family.age_gap.child_parent_minimum");
	parent_minimum_age_gap = pt.get<int>("popgen.family.age_gap.parent_minimum");
	parent_maximum_age_gap = pt.get<int>("popgen.family.age_gap.parent_maximum");
	child_minimum_age_gap = pt.get<int>("popgen.family.age_gap.child_minimum");
	child_maximum_age_gap = pt.get<int>("popgen.family.age_gap.child_maximum");

	for (const auto &i : pt.get_child("popgen.family.size_distribution")) {
		if (i.first == "p") {
			family_size_distribution.emplace_back(i.second.get_value<int>());
		}
	}

	kindergarten_age = pt.get<int>("popgen.school.age.kindergarten");
	primary_school_age = pt.get<int>("popgen.school.age.primary_school");
	secondary_school_age = pt.get<int>("popgen.school.age.secondary_school");
	higher_eduation_age = pt.get<int>("popgen.school.age.higher_education");
	graduation_age = pt.get<int>("popgen.school.age.graduation");
	p_higher_education = pt.get<double>("popgen.school.p_higher_education");
	school_minimum_size = pt.get<int>("popgen.school.size.minimum");
	school_maximum_size = pt.get<int>("popgen.school.size.maximum");

	work_minimum_age = pt.get<int>("popgen.work.age.minimum");
	work_maximum_age = pt.get<int>("popgen.work.age.maximum");
	p_employed = pt.get<double>("popgen.work.p_employed");
	work_minimum_size = pt.get<int>("popgen.work.size.minimum");
	work_maximum_size = pt.get<int>("popgen.work.size.maximum");

	community_minimum_size = pt.get<int>("popgen.community.size.minimum");
	community_maximum_size = pt.get<int>("popgen.community.size.maximum");
}

}
