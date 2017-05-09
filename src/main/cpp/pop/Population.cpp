#include "Population.h"

#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Person.h"
#include "core/Health.h"
#include "util/Errors.h"
#include "util/Random.h"

namespace stride {
/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population.
std::vector<Person> Population::get_random_persons(util::Random& rng, std::size_t count)
{
	if (count > size()) {
		FATAL_ERROR(
		    "Cannot pick " + std::to_string(count) + " random people from a population of " +
		    std::to_string(size()) + ".");
	}

	auto max_population_index = size() - 1;
	std::unordered_map<size_t, std::size_t> random_pick_indices;
	for (size_t i = 0; i < count; i++) {
		size_t pick_index;
		do {
			pick_index = rng(max_population_index);
		} while (random_pick_indices.find(pick_index) != random_pick_indices.end());
		random_pick_indices[pick_index] = i;
	}

	std::vector<Person> random_picks(count, Person(0, nullptr));
	size_t i = 0;
	for (const auto& person : people) {
		if (random_pick_indices.find(i) != random_pick_indices.end()) {
			random_picks[random_pick_indices[i]] = person;
		}
		i++;
	}

	return random_picks;
}

/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population
/// which satisfy the given predicate.
std::vector<Person> Population::get_random_persons(
    util::Random& rng, std::size_t count, std::function<bool(const Person&)> matches)
{
	// This function tries to search as efficiently as possible for people who match
	// the given predicate without skewing the distribution too much.
	//
	// The core algorithm used by this function boils down to repeatedly grabbing random
	// groups of people from the population and testing if they're a match. Note that
	// there is no bound on how long this can take: if we're SOL, then we'll always end up
	// picking people who don't match the predicate.
	//
	// That issue is compensated for here by growing the sample size exponentially as long as an
	// iteration does not find any suitable persons until either the entire population is
	// sampled and we throw an exception or we find enough matching candidates.

	std::vector<Person> results;
	std::unordered_set<PersonId> result_ids;
	size_t sample_size = count;
	while (count > 0) {
		size_t found = 0;
		for (auto person : get_random_persons(rng, sample_size)) {
			if (result_ids.find(person.GetId()) == result_ids.end() && matches(person)) {
				results.push_back(person);
				result_ids.insert(person.GetId());
				count--;
				found++;

				if (count == 0) {
					return results;
				}
			}
		}

		if (found == 0) {
			if (sample_size == size()) {
				FATAL_ERROR(
				    "Could not find " + std::to_string(count) + " people matching the predicate.");
			}
			sample_size *= 2;
			if (sample_size > size()) {
				sample_size = size();
			}
		} else {
			sample_size = count;
		}
	}
	return results;
}

/// Get the cumulative number of cases.
unsigned int Population::get_infected_count() const
{
	unsigned int total{0U};
	for (const auto& p : *this) {
		const auto& h = p.GetHealth();
		total += h.IsInfected() || h.IsRecovered();
	}
	return total;
}
}