#ifndef POPULATION_HOUSEHOLD_H_INCLUDED
#define POPULATION_HOUSEHOLD_H_INCLUDED

/**
 * @file
 * Header file for reference households
 */

#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace population {

// A list of ages.
using Household = std::vector<int>;

// Parse a reference households file.
std::vector<Household> ParseReferenceHouseholds(const boost::property_tree::ptree& pt);

} // namespace population
} // namespace stride

#endif
