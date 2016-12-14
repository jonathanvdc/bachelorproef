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

#include "core/Person.h"

#include <numeric>
#include <vector>

namespace stride {

/**
 * Container for persons in population.
 */
class Population : public std::vector<Person>
{
public:

	/// Get the cumulative number of cases.
	unsigned int GetInfectedCount() const
	{
		const auto counter  = [](unsigned int total, const Person& p) {
						return total + (p.IsInfected() || p.IsRecovered());
		                        };
		return std::accumulate(this->begin(), this->end(), 0U, counter);
	}

};

} // end_of_namespace

#endif // end of include guard
