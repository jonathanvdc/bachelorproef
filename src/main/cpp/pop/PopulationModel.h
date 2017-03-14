#ifndef POPULATION_MODEL_H_INCLUDED
#define POPULATION_MODEL_H_INCLUDED

/**
 * @file
 * Header file for the population model class
 */

#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "util/InclusiveRange.h"

namespace stride {
namespace population_model {

using stride::util::InclusiveRange;

struct Age
{
	// Read the age model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// The age distribution is a piecewise linear curve connecting
	// (0, h), (elbow_age, h), and (maximum_age, 0), where h is computed
	// so that the area under the curve is 1.
	int maximum;
	int elbow;
};

struct HouseholdAge
{
	// Read the household age model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// Minimum age for members of households of size <= 2.
	int live_alone_minimum;

	// Maximum age for children in households of size >= 3.
	int child_maximum;

	// Age range for parents, in households of size >= 3.
	InclusiveRange<int> parent;
};

struct HouseholdAgeGap
{
	// Read the household age gap model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// Minimum age gap between children and parents.
	int child_parent_minimum;

	// Inter-parent age gap range.
	InclusiveRange<int> parent;

	// Inter-child age gap range.
	InclusiveRange<int> child;
};

struct Household
{
	// Read the household model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	HouseholdAge age;
	HouseholdAgeGap age_gap;

	// Household size probabilities (relative to sum of vector).
	// size_distribution[k] is the relative probability of a household having (k + 1) members.
	std::vector<int> size_distribution;
};

struct SchoolAge
{
	// Read the school age model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// The age at which people start going to kindergarten.
	int kindergarten;

	// The age at which people start going to primary school.
	int primary_school;

	// The age at which people start going to secondary school.
	int secondary_school;

	// The age at which people start higher education (or leave school).
	int higher_education;

	// The age at which people graduate from higher education.
	int graduation;
};

struct School
{
	// Read the school model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	SchoolAge age;

	// The percentage of people who go into higher education.
	double p_higher_education;

	// The size range for schools (in # of students).
	InclusiveRange<int> size;
};

struct Work
{
	// Read the work model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// The age range for employees.
	InclusiveRange<int> age;

	// The percentage of people, within this age range, that is employed.
	double p_employed;

	// The size range for work sites (in # of employees). The distribution is linear.
	InclusiveRange<int> size;
};

struct Community
{
	// Read the community model values from a ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	void parse(const boost::property_tree::ptree& pt);

	// The size range for communities (in # of members). The distribution is linear.
	InclusiveRange<int> size;
};

/**
 * Data from which populations may be generated.
 */
struct Model
{
	// Read a population model from an ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	// See data/population_model_default.xml for the structure `pt` should have.
	void parse(const boost::property_tree::ptree& pt);

	Age age;
	Household household;
	School school;
	Work work;
	Community community;

	// Size range for the entire population.
	InclusiveRange<int> size;
};

} // namespace population_model
} // namespace stride

#endif
