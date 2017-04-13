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

/**
 * Parameters for population generation.
 */
struct Model final
{
	// Read a population model from an ptree.
	// Throws `boost::property_tree::ptree_error` on invalid inputs.
	// See data/population_model_default.xml for the structure `pt` should have.
	void parse(const boost::property_tree::ptree& pt);

	int school_size;
	int school_cluster_size;
	int college_size;
	int college_cluster_size;
	int workplace_size;
	int community_size;
	double school_radius;
};

} // namespace population_model
} // namespace stride

#endif
