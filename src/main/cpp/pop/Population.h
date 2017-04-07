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
#include "core/Health.h"
#include "util/Random.h"

namespace stride {

/**
 * Container for persons in population.
 */
class Population
{
private:
	std::map<PersonId, Person> people;
	PersonId max_person_id;

	template <typename TMapIterator>
	class PopulationIterator;

private:
	/// An iterator implementation for Population containers.
	template <typename TMapIterator>
	class PopulationIterator
	{
	public:
		typedef TMapIterator map_iterator;

		PopulationIterator(const TMapIterator& map_iterator_val) : map_iterator_val(map_iterator_val) {}
		PopulationIterator(const PopulationIterator<TMapIterator>&) = default;
		PopulationIterator<TMapIterator>& operator++()
		{
			++map_iterator_val;
			return *this;
		}
		PopulationIterator<TMapIterator>& operator++(int)
		{
			map_iterator_val++;
			return *this;
		}
		PopulationIterator<TMapIterator>& operator--()
		{
			--map_iterator_val;
			return *this;
		}
		PopulationIterator<TMapIterator>& operator--(int)
		{
			map_iterator_val--;
			return *this;
		}
		auto& operator*() { return map_iterator_val->second; }
		auto& operator*() const { return map_iterator_val->second; }
		auto operator-> () { return &map_iterator_val->second; }
		auto operator-> () const { return &map_iterator_val->second; }
		bool operator==(const PopulationIterator<TMapIterator>& other) const
		{
			return map_iterator_val == other.map_iterator_val;
		}
		bool operator!=(const PopulationIterator<TMapIterator>& other) const
		{
			return map_iterator_val != other.map_iterator_val;
		}

		template <typename T>
		friend void swap(PopulationIterator<T>& lhs, PopulationIterator<T>& rhs);

	private:
		TMapIterator map_iterator_val;
	};

public:
	typedef PopulationIterator<decltype(people)::iterator> iterator;
	typedef PopulationIterator<decltype(people)::const_iterator> const_iterator;

	/// Inserts a new element into the container constructed in-place with the given args.
	template <typename... TArgs>
	auto emplace(TArgs&&... args)
	{
		Person value(args...);
		if (value.GetId() > max_person_id)
			max_person_id = value.GetId();

		return people.emplace(value.GetId(), std::move(value));
	}

	/// Gets the number of people in this population.
	auto size() const -> decltype(people.size()) { return people.size(); }

	/// Creates a mutable iterator positioned at the first person in this population.
	iterator begin() { return iterator(people.begin()); }

	/// Creates a constant iterator positioned at the first person in this population.
	const_iterator begin() const { return const_iterator(people.begin()); }

	/// Creates a mutable iterator positioned just past the last person in this population.
	iterator end() { return iterator(people.end()); }

	/// Creates a constant iterator positioned just past the last person in this population.
	const_iterator end() const { return const_iterator(people.end()); }

	/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population.
	std::vector<Person*> get_random_persons(util::Random& rng, std::size_t count);

	/// Gets a list of pointers to 'count' unique, randomly chosen participants in the population
	/// which satisfy the given predicate.
	std::vector<Person*> get_random_persons(
	    util::Random& rng, std::size_t count, std::function<bool(const Person&)> matches);

	/// Get the cumulative number of cases.
	unsigned int get_infected_count() const;
};

/// Swaps two population iterators.
template <typename TMapIterator>
void swap(
    typename Population::PopulationIterator<TMapIterator>& lhs,
    typename Population::PopulationIterator<TMapIterator>& rhs)
{
	std::swap(lhs.map_iterator, rhs.map_iterator);
}

using PopulationRef = std::shared_ptr<const Population>;

} // end_of_namespace

namespace std {
template <>
struct iterator_traits<stride::Population::iterator>
{
	typedef stride::Person value_type;
	typedef value_type& reference;
	typedef value_type* pointer;
};
template <>
struct iterator_traits<stride::Population::const_iterator>
{
	typedef const stride::Person value_type;
	typedef value_type& reference;
	typedef value_type* pointer;
};
}

#endif // end of include guard
