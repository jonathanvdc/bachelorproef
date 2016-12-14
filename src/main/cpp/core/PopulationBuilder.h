#ifndef POPULATION_BUILDER_H_INCLUDED
#define POPULATION_BUILDER_H_INCLUDED
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
 * Initialize populations.
 */

#include "core/Population.h"
#include "util/Random.h"

#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace stride {

/**
 * Initializes Population objects.
 */
class PopulationBuilder
{
public:
	/**
	 * Initializes a Population: add persons, set immunity, seed infection.
	 *
	 * @param pop             Pointer to the initialized population.
	 * @param pt_config       The property_tree with configuration settings.
	 */
	static bool Build(std::shared_ptr<Population> pop ,
	        const boost::property_tree::ptree& pt_config, const boost::property_tree::ptree& pt_disease);

private:
	/**
	 *
	 */
	static std::vector<double> GetDistribution(const boost::property_tree::ptree& pt_root, const std::string& xml_tag);

	/**
	 *
	 */
	static unsigned int SampleFromDistribution(util::Random& rng, const std::vector<double>& distribution);
};

} // end_of_namespace

#endif // include guard
