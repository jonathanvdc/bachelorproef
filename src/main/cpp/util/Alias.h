/*
 * Alias.h
 *
 *  Created on: Mar 11, 2017
 *      Author: cedric
 */
#ifndef ALIAS_H_
#define ALIAS_H_

#include "Random.h"
#include <vector>
namespace stride{
namespace util{
/**
 * Class used to generate random numbers according to Vose's Alias Method.
 * Used the algorithm on http://keithschwarz.com/darts-dice-coins/
 * It uses an strides Random as random number generator.
 */
class Alias {
public:
	///Constructor. Uses a random seed
	Alias(const std::vector<double> probabilities);
	///Constructor. Uses a given seed
	Alias(std::vector<double> probabilities,const unsigned int seed);
	///Generates a new number
	unsigned int Next();
private:
	stride::util::Random m_random;							///< The random number generator
	std::vector<unsigned int> m_alias;						///< The vector of aliases
	std::vector<double> m_prob;								///< The vector of probabilities
};
}
}

#endif /* ALIAS_H_ */
