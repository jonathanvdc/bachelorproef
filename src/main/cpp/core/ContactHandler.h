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

#include "util/Random.h"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace indismo {

/**
 * Processes the contacts between persons and determines whether transmission occurs.
 */
class ContactHandler
{
public:
	/// Constructor sets the transmission rate and random number generator.
	ContactHandler(double transmission_rate, unsigned int seed, unsigned int stream_count, unsigned int id)
			: m_transmission_rate_adult(transmission_rate), m_rng(seed)
	{
		m_rng.Split(stream_count, id);
	}

	void addMeanNumsContacts(std::string cluster_type, std::vector<double> mean_nums)
	{
		m_age_contact_mean_num[cluster_type] = mean_nums;
	}

	void addContactRates(std::string cluster_type, std::vector<double> rates)
	{
		m_age_contact_rates[cluster_type] = rates;
	}

	/// Handle one contact between persons of the given age. Performs a Bernoulli process with a random
	/// number. The given ages determine the transmission rate (=probability for "success").
	bool operator()(unsigned int age, std::string cluster_type, size_t cluster_size)
	{

		if (age > 80) {
			age = 80;
		}

		//double rate = m_age_contact_rates[cluster_type][age] * m_transmission_rate_adult;
		double rate = m_age_contact_mean_num[cluster_type][age]/cluster_size;
		if(cluster_type == "household")
		{
			rate = 1;
		}
		if(cluster_type == "work")
		{
			rate = rate*1.7;
		}
		rate *= m_transmission_rate_adult;
		return m_rng.NextDouble() < rate;
	}

	/// Check if two individuals make contact.
	/// TODO Check how to maintain correct distribution of probability when splitting contact and transmission rates.
	bool contact(unsigned int age, std::string cluster_type, size_t cluster_size)
	{
		if (age > 80) {
			age = 80;
		}

		double rate = (m_age_contact_mean_num[cluster_type][age] / cluster_size);
		if(cluster_type == "household")
		{
			rate = 1;
		}
		if(cluster_type == "work")
		{
					rate = rate*1.7;
		}
		return m_rng.NextDouble() < rate;
	}


private:
	std::map<std::string, std::vector<double>>      m_age_contact_rates;          ///< Cluster types and contact rates per age
	std::map<std::string, std::vector<double>>      m_age_contact_mean_num;       ///< Cluster types and mean number of contacts per age
	double                                          m_transmission_rate_adult;    ///< Transmission rate between adults.
	util::Random                                    m_rng;                        ///< Random number engine.
};

} // end_of_namespace

#endif // include guard
