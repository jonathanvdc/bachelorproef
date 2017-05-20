#ifndef VISUALIZERDATA_H_INCLUDED
#define VISUALIZERDATA_H_INCLUDED

/**
 * @file
 * Data structure for the visualiser.
 */

#include <memory>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "pop/Population.h"

namespace stride {

/**
 * Data structure for the visualiser.
 * Stores the day-by-day infected count per Town.
 */
class VisualizerData
{
private:
	/// Vector of days: Each day is a map from Town names to the amount of infected.
	std::vector<std::map<std::size_t, unsigned int>> days;

public:
	/// Register a new day from the given population's current state.
	void AddDay(const Population& pop);

	/// Get a reference to the vector of days.
	std::vector<std::map<std::size_t, unsigned int>>& GetDays() const;

	/// Convert all stored data into a boost property tree, so that it can be converted to JSON.
	std::shared_ptr<boost::property_tree::ptree> ToPtree() const;
};

} // end of namespace

#endif // end-of-include-guard