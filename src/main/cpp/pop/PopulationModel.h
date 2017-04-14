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
namespace population_model {

struct TownPivot final
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
	    std::vector<TownPivot> town_pivots, util::InclusiveRange<int> school_age,
	    util::InclusiveRange<int> college_age, double college_ratio, double college_commute_ratio,
	    util::InclusiveRange<int> employable_age, double employed_ratio, double work_commute_ratio)
	    : school_size(school_size), school_cluster_size(school_cluster_size), college_size(college_size),
	      college_cluster_size(college_cluster_size), workplace_size(workplace_size),
	      community_size(community_size), school_radius(school_radius), population_size(population_size),
	      city_ratio(city_ratio), town_pivots(town_pivots), school_age(school_age), college_age(college_age),
	      college_ratio(college_ratio), college_commute_ratio(college_commute_ratio),
	      employable_age(employable_age), employed_ratio(employed_ratio), work_commute_ratio(work_commute_ratio)
	{
	}

	// Generation parameters
	int school_size;
	int school_cluster_size;
	int college_size;
	int college_cluster_size;
	int workplace_size;
	int community_size;
	double school_radius;

	// Population/city/town parameters
	int population_size;
	int city_ratio;
	std::vector<TownPivot> town_pivots;

	// School-work profile
	util::InclusiveRange<int> school_age;
	util::InclusiveRange<int> college_age;
	double college_ratio;
	double college_commute_ratio;
	util::InclusiveRange<int> employable_age;
	double employed_ratio;
	double work_commute_ratio;
};

} // namespace population_model
} // namespace stride

#endif
