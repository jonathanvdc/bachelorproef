#ifndef POPULATION_H_INCLUDED
#define POPULATION_H_INCLUDED
/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S, Broeckhove J
 *  Aerts S, De Haes C, Van der Cruysse J & Van Hauwe L
 */

/**
 * @file
 * Header file for the core Population class
 */

#include "Person.h"
#include "core/Health.h"
#include "util/Random.h"

#include <numeric>
#include <memory>
#include <unordered_set>

namespace stride {

/**
 * Container for persons in population.
 */
class Population
{
private:
	std::vector<Person> people;

public:
	/// Inserts a new element into the container constructed in-place with the given args.
	template <typename... TArgs>
	auto emplace(TArgs&&... args)
	{
		return people.emplace_back(args...);
	}

	/// Gets the number of people in this population.
	auto size() const -> decltype(people.size())
	{
		return people.size();
	}

	/// Creates a mutable iterator positioned at the first person in this population.
	auto begin() -> decltype(people.begin())
	{
		return people.begin();
	}

	/// Creates a constant iterator positioned at the first person in this population.
	auto begin() const -> decltype(people.begin())
	{
		return people.begin();
	}

	/// Creates a mutable iterator positioned just past the last person in this population.
	auto end() -> decltype(people.end())
	{
		return people.end();
	}

	/// Creates a constant iterator positioned just past the last person in this population.
	auto end() const -> decltype(people.end())
	{
		return people.end();
	}

	/// Gets a mutable reference to a random person in the population.
	Person& get_random_person(util::Random& rng)
	{
		auto max_population_index = size() - 1;
		return people[rng(max_population_index)];
	}

	/// Gets a constant reference to a random person in the population.
	const Person& get_random_person(util::Random& rng) const
	{
		auto max_population_index = size() - 1;
		return people[rng(max_population_index)];
	}

	/// Get the cumulative number of cases.
	unsigned int GetInfectedCount() const
	{
		unsigned int total {0U};
		for (const auto& p : *this) {
			const auto& h = p.GetHealth();
			total += h.IsInfected() || h.IsRecovered();
		}
		return total;
	}
};

using PopulationRef = std::shared_ptr<const Population>;

} // end_of_namespace

#endif // end of include guard
