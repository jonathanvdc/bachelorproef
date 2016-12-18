#ifndef CONTACTHANDLER_H_INCLUDED
#define CONTACTHANDLER_H_INCLUDED
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
 * Header for the ContactHandler class.
 */

#include "core/ClusterType.h"
#include "util/Random.h"

#include <map>
#include <vector>

namespace stride {

/**
 * Processes the contacts between persons and determines whether transmission occurs.
 */
class ContactHandler
{
public:
	/// Constructor sets the transmission rate and random number generator.
	ContactHandler(double transmission_rate, unsigned int seed, unsigned int stream_count, unsigned int id)
			: m_rng(seed), m_transmission_rate_adult(transmission_rate)
	{
		m_rng.Split(stream_count, id);
	}

	void AddMeanNumsContacts(ClusterType cluster_type, const std::vector<double>& mean_nums)
	{
		// TODO remove this function? I don't think m_age_contact_mean_num is ever used...
		m_age_contact_mean_num[cluster_type] = mean_nums;
	}

	void AddContactRates(ClusterType cluster_type, const std::vector<double>& rates)
	{
		m_age_contact_rates[cluster_type] = rates;
	}

	/// Handle one contact between persons of the given age. Performs a Bernoulli process with a random
	/// number. The given ages determine the transmission rate (=probability for "success").
	bool operator()(unsigned int age, ClusterType cluster_type, size_t cluster_size)
	{
	        // Top off age at 80.
	        age = (age <= 80U) ? age : 80U;

		//double rate = m_age_contact_rates[cluster_type][age] * m_transmission_rate_adult;
		double rate = m_age_contact_mean_num[cluster_type][age]/cluster_size;
		if (cluster_type == ClusterType::Household) {
			rate = 1;
		}
		if (cluster_type == ClusterType::Work) {
			rate = rate * 1.7;
		}
		rate *= m_transmission_rate_adult;
		return m_rng.NextDouble() < rate;
	}

	/// Check if two individuals make contact.
	/// TODO Check how to maintain correct distribution of probability when splitting contact and transmission rates.
	bool Contact(unsigned int age, ClusterType cluster_type, size_t cluster_size)
	{
                // Top off age at 80.
                age = (age <= 80U) ? age : 80U;

		double rate = m_age_contact_mean_num[cluster_type][age] / cluster_size;
		if (cluster_type == ClusterType::Household) {
			rate = 1;
		}
		if (cluster_type == ClusterType::Work) {
			rate = rate * 1.7;
		}
		return m_rng.NextDouble() < rate;
	}

	//TODO add function to get transmission chance separately

private:
	std::map<ClusterType, std::vector<double>>      m_age_contact_rates;          ///< Cluster types and contact rates per age
	std::map<ClusterType, std::vector<double>>      m_age_contact_mean_num;       ///< Cluster types and mean number of contacts per age

	util::Random                                    m_rng;                        ///< Random number engine.
        double                                          m_transmission_rate_adult;    ///< Transmission rate between adults.
};

} // end_of_namespace

#endif // include guard
