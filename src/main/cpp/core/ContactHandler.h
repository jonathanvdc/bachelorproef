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

#include "core/Age.h"
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
		m_age_contact_mean_num[cluster_type] = mean_nums;
	}

	/// Handle one contact between persons of the given age. Performs a Bernoulli process with a random
	/// number. The given ages determine the transmission rate (=probability for "success").
	bool operator()(unsigned int age, ClusterType cluster_type, size_t cluster_size)
	{
		return m_rng.NextDouble() < m_transmission_rate_adult * ContactRate(age, cluster_type, cluster_size);
	}

	/// Check if two individuals make contact.
	bool Contact(unsigned int age, ClusterType cluster_type, size_t cluster_size)
	{
		return m_rng.NextDouble() < ContactRate(age, cluster_type, cluster_size);
	}

        /// Check if two individuals make contact.
        double ContactRate(unsigned int age, ClusterType cluster_type, size_t cluster_size)
        {
                double rate = 1.0;
                if (cluster_type != ClusterType::Household) {
                        rate = m_age_contact_mean_num[cluster_type][EffectiveAge(age)] / cluster_size;
                        rate = (cluster_type == ClusterType::Work) ? rate*1.7 : rate;
                }
                return rate;
        }

private:
	std::map<ClusterType, std::vector<double>>      m_age_contact_mean_num;       ///< Cluster types and mean number of contacts per age

        //using ContactProfile = std::array<AgeContactProfile, NumOfClusterTypes()>;
	//std::array<std::array<double, MaximumAge() + 1>, NumOfClusterTypes()>   m_age_contact_mean_num;       ///< Cluster types and mean number of contacts per age
	util::Random                                    m_rng;                        ///< Random number engine.
        double                                          m_transmission_rate_adult;    ///< Transmission rate between adults.
};

} // end_of_namespace

#endif // include guard
