#ifndef POPULATION_MODEL_H_INCLUDED
#define POPULATION_MODEL_H_INCLUDED

/**
 * @file
 * Header file for the population model class
 */

#include <memory>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "util/InclusiveRange.h"

namespace stride {
namespace population {

// The proportion of the generated population that *doesn't* get placed in a
// city is randomly allocated among a set of randomly-placed towns. The number
// of inhabitants such a town should have is determined by a list of "town
// ranges". For example, such a list might describe that
//
//     * 30% of the generated towns have 100-1000 inhabitants; and
//     * 70% of the generated towns have 1000-5000 inhabitants.
//
// A TownRange represents one entry in this list: a population range, paired
// with a probability of this range being picked when generating a town.
//
struct TownRange final
{
	util::InclusiveRange<int> population;
	double probability;
};

struct Model;
using ModelRef = std::shared_ptr<const Model>;

/**
 * Parameters for population generation.
 */
struct Model final
{
	// Read a population model from an ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	// See data/population_model_default.xml for the structure `pt` should have.
	static ModelRef Parse(const boost::property_tree::ptree& pt);

	Model(
	    int school_size, int school_cluster_size, int college_size, int college_cluster_size, int workplace_size,
	    int community_size, double school_radius, int population_size, int city_ratio,
	    std::vector<TownRange> town_ranges, util::InclusiveRange<int> school_age,
	    util::InclusiveRange<int> college_age, double college_ratio, double college_commute_ratio,
	    util::InclusiveRange<int> employable_age, double employed_ratio, double work_commute_ratio)
	    : school_size(school_size), school_cluster_size(school_cluster_size), college_size(college_size),
	      college_cluster_size(college_cluster_size), workplace_size(workplace_size),
	      community_size(community_size), school_radius(school_radius), population_size(population_size),
	      city_ratio(city_ratio), town_ranges(town_ranges), school_age(school_age), college_age(college_age),
	      college_ratio(college_ratio), college_commute_ratio(college_commute_ratio),
	      employable_age(employable_age), employed_ratio(employed_ratio), work_commute_ratio(work_commute_ratio)
	{
	}

	// The size of a (non-college) school.
	int school_size;

	// The size of a (non-college) school cluster.
	int school_cluster_size;

	// The size of a college ("hogeschool").
	int college_size;

	// The size of a college cluster.
	int college_cluster_size;

	// The size of a workplace. Workplaces are their own clusters, so there's
	// no separate cluster size field.
	int workplace_size;

	// The size of a primary or secondary community.
	int community_size;

	// The radius to look for (non-college) schools in. (When no schools are
	// found in this radius, it is doubled repeatedly until one is found.)
	double school_radius;

	// The size of the entire population.
	int population_size;

	// The proportion of people who live in cities. (The remaining portion
	// lives in randomly-generated towns.)
	double city_ratio;

	// The ranges for town generation.
	std::vector<TownRange> town_ranges;

	// The age range for compulsory lower education.
	util::InclusiveRange<int> school_age;

	// The age range for college education.
	util::InclusiveRange<int> college_age;

	// The proportion of students in the above age range that actually goes
	// to college, instead of getting a job.
	double college_ratio;

	// The proportion of actual college students that commutes to a distant
	// college campus.
	double college_commute_ratio;

	// The age range for employable citizens.
	util::InclusiveRange<int> employable_age;

	// The proportion of citizens in the above age range that is actually
	// employed.
	double employed_ratio;

	// The proportion of actual employees that commutes to a distant workplace.
	double work_commute_ratio;
};

} // namespace population
} // namespace stride

#endif
