/*
 * Alias.h
 *
 *  Created on: Mar 11, 2017
 *      Author: cedric
 */
#ifndef ALIAS_H_INCLUDED
#define ALIAS_H_INCLUDED

#include <util/Random.h>
#include <vector>
namespace stride{
namespace alias{
/**
 * Class used to generate random numbers according to Vose's Alias Method.
 * Used the algorithm on http://keithschwarz.com/darts-dice-coins/
 * It uses an strides Random as random number generator.
 */
class Alias {
public:
	///Constructor. Uses a random seed
	Alias(std::vector<double> probabilities);
	///Constructor. Uses a given seed
	Alias(std::vector<double> probabilities,unsigned int seed);
	///Generates a new number
	unsigned int Next();
private:
	util::Random m_random = util::Random(0);				///< The random number generator
	std::vector<unsigned int> m_alias;						///< The vector of aliases
	std::vector<double> m_prob;								///< The vector of probabilities
};
}
}

#endif
