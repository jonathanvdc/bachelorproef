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

	/// No assignment operator needed
	Alias& operator=(const Alias&) = delete;

	/// Move constructor
	Alias(Alias&& other) = default;

	/// Creates an Alias object.
	static Alias CreateDistribution(std::vector<double> probabilities, util::Random& rng);

	/// Normalizes the given list of probabilities: every element is
	/// divided by the sum of the elements in the list.
	static void NormalizeProbabilities(std::vector<double>& probabilities);

	/// Generates a new number.
	std::size_t Next();

private:
	/// Constructor
	Alias(std::vector<std::size_t>&& alias, std::vector<double>&& prob, util::Random& rng)
	    : m_random{rng}, m_alias{alias}, m_prob{prob}
	{
	}

	/// The random number generator
	util::Random& m_random;

	/// The vector of aliases
	std::vector<std::size_t> m_alias;

	/// The vector of probabilities
	std::vector<double> m_prob;
};
}
}

#endif
