#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include <vector>
#include "multiregion/TravelModel.h"
#include "pop/Person.h"

/**
 * @file Defines data structures that keep track of a region's visitors and expatriates.
 */

namespace stride {
namespace multiregion {

/**
 * Represents an outgoing visitor: a person who visits another region.
 */
struct OutgoingVisitor final
{
	OutgoingVisitor(const Person& person, RegionId visited_region, std::size_t return_day)
	    : person(person), visited_region(visited_region), return_day(return_day)
	{
	}

	/// The person who is visiting another region.
	Person person;

	/// The region this visitor is visiting.
	RegionId visited_region;

	/// The index of the day on which this visitor's return trip is scheduled.
	std::size_t return_day;
};

/**
 * Represents an incoming visitor: a person who is visiting from another region.
 */
struct IncomingVisitor final
{
	IncomingVisitor(const Person& person, RegionId home_region, std::size_t return_day)
	    : person(person), home_region(home_region), return_day(return_day)
	{
	}

	/// The person who is visiting another region.
	Person person;

	/// The region this visitor is visiting from.
	RegionId home_region;

	/// The index of the day on which this visitor's return trip is scheduled.
	std::size_t return_day;
};

/// The input for a single step in the simulation and the result
/// of a pull operation.
struct SimulationStepInput final
{
	/// The list of all incoming visitors.
	std::vector<IncomingVisitor> visitors;

	/// The list of all returning expatriates.
	std::vector<Person> expatriates;
};

/// The output for a single step in the simulation and the result
/// of a push operation.
struct SimulationStepOutput final
{
	/// The list of all outgoing visitors.
	std::vector<OutgoingVisitor> visitors;

	/// The list of all returning expatriates.
	std::vector<OutgoingVisitor> expatriates;
};
}
}

#endif