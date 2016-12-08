#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED
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
 * Header for the Simulator class.
 */

#include "WorldEnvironment.h"
#include "core/ContactHandler.h"
#include "core/Cluster.h"
#include "core/LogMode.h"
#include "core/Population.h"

#include <boost/property_tree/ptree.hpp>
#include <type_traits>
#include <memory>
#include <string>
#include <vector>

namespace indismo {

/**
 * Main class that contains and direct the virtual world.
 */
class Simulator
{
public:
	/// Constructor: Initialize the Simulator.
	Simulator(const boost::property_tree::ptree& pt_config);

	/// Get the cumulative number of cases.
	unsigned int GetInfectedCount() const;

	/// Get the Population size.
	unsigned int GetPopulationSize() const;

	/// Get the population.
	const std::shared_ptr<const Population> GetPopulation() const;

	/// Run one time step.
	void RunTimeStep();

private:

	/// Get age-based contact rates for a given cluster type
	std::vector<double> GetContactRates(const std::vector<double>& mean_nums, unsigned int avg_cluster_size);

	/// Get mean number of contacts a day per age for given cluster type
	std::vector<double> GetMeanNumbersOfContacts(std::string cluster_type, const boost::property_tree::ptree& pt_contacts);

	/// Initialize the clusters.
	void InitializeClusters();

	/// Get average size of given clusters
	double GetAverageClusterSize(const std::vector<Cluster>& clusters);

	/// Update the contacts in the given clusters.
	void UpdateContacts(std::vector<Cluster>& clusters);

private:
	unsigned int                              m_num_threads;          ///< The number of (OpenMP) threads.
	std::shared_ptr<WorldEnvironment>         m_state;                ///< The current state of the simulated world.

	LogMode                                   m_log_level;            ///< Specifies logging mode.

	std::shared_ptr<Population>               m_population;           ///< Pointer to the Population.
	std::vector<Cluster>                      m_households;           ///< Container with households Clusters.
	std::vector<Cluster>                      m_day_clusters;         ///< Container with day Clusters.
	std::vector<Cluster>                      m_home_districts;       ///< Container with home district Clusters.
	std::vector<Cluster>                      m_day_districts;        ///< Container with day district Clusters.

	std::vector<std::shared_ptr<ContactHandler>>    m_contact_handler;      ///< Pointer to the ContactHandler.
};

} // end_of_namespace

#endif // end-of-include-guard
