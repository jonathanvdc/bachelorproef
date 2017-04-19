#ifndef VISITOR_JOURNAL_H_INCLUDED
#define VISITOR_JOURNAL_H_INCLUDED

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "multiregion/TravelModel.h"
#include "pop/Person.h"
#include "util/Errors.h"

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
struct VisitorId final
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
public:
	/// Adds an expatriate to this journal.
	void AddExpatriate(const Person& person)
	{
		if (expatriates.find(person.GetId()) != expatriates.end()) {
			FATAL_ERROR(
			    "person with id " + std::to_string(person.GetId()) +
			    " cannot be added as an expatriate twice.");
		}
		expatriates.emplace(person.GetId(), person);
	}

	/// Extracts the expatriate with the given id.
	Person ExtractExpatriate(PersonId id)
	{
		if (expatriates.find(id) == expatriates.end()) {
			FATAL_ERROR("no expatriate with id " + std::to_string(id) + ".");
		}
		auto result = expatriates.find(id)->second;
		expatriates.erase(id);
		return result;
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
	/// Tests if the person with the given id is a visitor.
	bool IsVisitor(PersonId id) const { return visitor_ids.find(id) != visitor_ids.end(); }

	/// Gets the number of visitors in the journal.
	std::size_t GetVisitorCount() const { return visitor_ids.size(); }

	/// Adds the given visitor to this journal.
	void AddVisitor(VisitorId visitor, RegionId home_region_id, std::size_t return_day)
	{
		if (visitor_ids.find(visitor.visitor_id) != visitor_ids.end()) {
			FATAL_ERROR(
			    "the same visitor id (" + std::to_string(visitor.visitor_id) + ") cannot be added twice.");
		}

		visitors[return_day][home_region_id].push_back(visitor);
		visitor_ids.insert(visitor.visitor_id);
	}

	/// Extracts all visitors that were scheduled to return on the given day.
	std::unordered_map<RegionId, std::vector<VisitorId>> ExtractVisitors(std::size_t return_day)
	{
		auto result = std::move(visitors[return_day]);
		visitors.erase(return_day);
		for (const auto& pair : result) {
			for (const auto& visitor : pair.second) {
				visitor_ids.erase(visitor.visitor_id);
			}
		}
		return result;
	}

private:
	/// A dictionary of visitors, grouped by the day of their return trip and the region
	/// that sent them.
	std::unordered_map<std::size_t, std::unordered_map<RegionId, std::vector<VisitorId>>> visitors;
	std::unordered_set<PersonId> visitor_ids;
};
}
}

#endif