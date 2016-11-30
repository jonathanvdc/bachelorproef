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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Header file for the core Population class
 */

#include <assert.h>
#include <numeric>
#include <vector>

#include "core/Person.h"

namespace indismo {
namespace core {

/**
 * Container with Person objects.
 */
class Population : public std::vector<Person>
{
public:
	/// Add a Person with given data to the Population.
	void AddPerson(
		unsigned int id, double age, unsigned int household_id,
		unsigned int home_district_id, unsigned int day_cluster_id,
		unsigned int day_district_id, unsigned int start_infectiousness,
		unsigned int start_symptomatic, unsigned int end_infectiousness,
		unsigned int end_symptomatic)
	{
		auto p = Person(id, age, household_id, home_district_id,
				day_cluster_id, day_district_id,start_infectiousness,
				start_symptomatic,end_infectiousness,end_symptomatic);
		push_back(p);
	}

	/// Get the cumulative number of cases.
	unsigned int GetInfectedCount() const
	{
		const auto counter  = [](unsigned int total, const Person& p) {
						return total + (p.IsInfected() || p.IsRecovered());
					};
		return std::accumulate(this->begin(), this->end(), 0U, counter);
	}

	/// Get the Population size.
	size_t GetSize() const
	{
		return this->size();
	}

	/// Start an infection of the Person with given index in the container.
	void SetIndexCase(const unsigned int index)
	{
		(*this)[index].SetIndexCase();
	}

	/// Start an infection of the Person with given index in the container.
	void SetImmune(const unsigned int index)
	{
		(*this)[index].SetImmune();
	}

	/// Subscribe the given Person to the social contact survey.
	void SetParticipant(const unsigned int index, std::shared_ptr<spdlog::logger> logger)
	{
		(*this)[index].ParticipateInSurvey(logger);
	}

	const Person& GetPerson(const unsigned int index) const
	{
		return (*this)[index];
	}
};


} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
