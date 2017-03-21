/*
 * Alias.h
 *
 *  Created on: Mar 11, 2017
 *      Author: cedric
 */
#ifndef ALIAS_H_INCLUDED
#define ALIAS_H_INCLUDED

#include <vector>
#include <util/Random.h>
namespace stride {
namespace alias {
/**
 * Class used to generate random numbers according to Vose's Alias Method.
 * Used the algorithm on http://keithschwarz.com/darts-dice-coins/
 * It uses an strides Random as random number generator.
 */
class Alias
{
    public:
	/// No standard constructor needed
	Alias() = delete;
	/// No copy constructor needed
	Alias(const Alias&) = delete;
	/// No assignement operator needed
	Alias& operator=(const Alias&) = delete;
	/// Creates an Alias object. The constructor would be way too long.
	static Alias& CreateDistribution(std::vector<double> probabilities, util::Random& rng);
	/// Generates a new number
	unsigned int Next();

    private:
	/// Constructor
	Alias(std::vector<unsigned int> alias, std::vector<double> prob, util::Random& rng)
	    : m_random{rng}, m_alias{alias}, m_prob{prob}
	{
	}
	util::Random& m_random;		   ///< The random number generator
	std::vector<unsigned int> m_alias; ///< The vector of aliases
	std::vector<double> m_prob;	///< The vector of probabilities
};
}
}

#endif
