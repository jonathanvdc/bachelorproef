#ifndef VISITOR_JOURNAL_H_INCLUDED
#define VISITOR_JOURNAL_H_INCLUDED

#include <unordered_map>
#include "multiregion/TravelModel.h"
#include "pop/Person.h"

/**
 * @file Defines data structures that keep track of a region's visitors and expatriates.
 */

namespace stride {
namespace multiregion {

/**
 * Represents a visitor to a region: a pair of ids of which the first represents the
 * person's id in their home region and the second is their id in the region they're
 * visiting.
 */
struct Visitor final
{
	/// The visitor's id in their home region.
	PersonId home_id;

	/// The visitor's id in the region they're visiting.
	PersonId visitor_id;
};

/**
 * Keeps track of people who are currently outside of their home region.
 */
class ExpatriateJournal final
{
	/// Adds an expatriate to this journal.
	void add_expatriate(Person&& person) { expatriates.emplace(person.GetId(), person); }

	/// Extracts the expatriate with the given id.
	Person extract_expatriate(PersonId id)
	{
		auto result = std::move(expatriates.find(id)->second);
		expatriates.erase(id);
		return std::move(result);
	}

private:
	/// A dictionary that maps expatriate person ids to personal information.
	std::unordered_map<PersonId, Person> expatriates;
};

/**
 * Records all visitors to a region.
 */
class VisitorJournal final
{
public:
	/// Adds the given visitor to this journal.
	void add_visitor(Visitor visitor, RegionId home_region_id, std::size_t return_day)
	{
		visitors[return_day][home_region_id].push_back(visitor);
	}

	/// Extracts all visitors that were scheduled to return on the given day.
	std::unordered_map<RegionId, std::vector<Visitor>> extract_visitors(std::size_t return_day)
	{
		auto result = std::move(visitors[return_day]);
		visitors.erase(return_day);
		return std::move(result);
	}

private:
	/// A dictionary of visitors, grouped by the day of their return trip and the region
	/// that sent them.
	std::unordered_map<std::size_t, std::unordered_map<RegionId, std::vector<Visitor>>> visitors;
};
}
}

#endif