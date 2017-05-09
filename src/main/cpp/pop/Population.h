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

#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <vector>
#include "Person.h"
#include "core/Atlas.h"
#include "core/Health.h"
#include "geo/GeoPosition.h"
#include "util/Random.h"

namespace stride {

/**
 * Stores Persons, along with an Atlas mapping their clusters to GeoPositions.
 */
class Population
{
private:
	std::vector<Person> people;
	PersonId max_person_id;
	Atlas atlas;

public:
	typedef decltype(people)::const_iterator const_iterator;
	typedef const_iterator iterator;

	/// Inserts a new element into the container constructed in-place with the given args.
	template <typename... TArgs>
	const_iterator emplace(TArgs&&... args)
	{
		Person value(args...);
		if (value.GetId() > max_person_id)
			max_person_id = value.GetId();

		people.emplace_back(value.GetId(), value.GetData());
		return end() - 1;
	}

	/// Extracts the person with the given id from this population.
	Person extract(PersonId id)
	{
		throw std::runtime_error("'extract' is not supported");
	}

	/// Gets the number of people in this population.
	auto size() const -> decltype(people.size()) { return people.size(); }

	/// Creates a constant iterator positioned at the first person in this population.
	const_iterator begin() const { return const_iterator(people.begin()); }

	/// Creates a constant iterator positioned just past the last person in this population.
	const_iterator end() const { return const_iterator(people.end()); }

	/// Gets the largest id for any person that has ever been in this population.
	PersonId get_max_id() const { return max_person_id; }

	/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population.
	std::vector<Person> get_random_persons(util::Random& rng, std::size_t count);

	/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population
	/// which satisfy the given predicate.
	std::vector<Person> get_random_persons(
	    util::Random& rng, std::size_t count, std::function<bool(const Person&)> matches);

	/// Get the cumulative number of cases.
	unsigned int get_infected_count() const;

	/// Store a GeoPosition in the population's atlas.
	auto AtlasEmplace(const Atlas::Key& key, const geo::GeoPosition& pos) -> decltype(atlas.Emplace(key, pos))
	{
		return atlas.Emplace(key, pos);
	}
};

using PopulationRef = std::shared_ptr<const Population>;

} // end_of_namespace

#endif // end of include guard
