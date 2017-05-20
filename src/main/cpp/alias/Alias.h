#ifndef ALIAS_H_INCLUDED
#define ALIAS_H_INCLUDED

#include <unordered_map>
#include <vector>
#include "util/Random.h"

namespace stride {
namespace alias {

/// Normalizes the given list of probabilities: every element is
/// divided by the sum of the elements in the list.
void NormalizeProbabilities(std::vector<double>& probabilities);

/**
 * Class used to generate random numbers according to Vose's Alias Method.
 * Used the algorithm on http://keithschwarz.com/darts-dice-coins/
 * It uses an strides Random as random number generator.
 */
class Alias
{
public:
	Alias() = delete;
	Alias(const Alias&) = delete;
	Alias& operator=(const Alias&) = delete;

	Alias(Alias&& other) = default;
	Alias& operator=(Alias&& other) = default;

	/// Creates an Alias object.
	static Alias CreateDistribution(std::vector<double> probabilities, util::Random& rng);

	/// Generates a new number.
	std::size_t Next();

private:
	/// Constructor
	Alias(std::vector<std::size_t>&& alias, std::vector<double>&& prob, util::Random& rng)
	    : m_random(&rng), m_alias(std::move(alias)), m_prob(std::move(prob))
	{
	}

	/// The random number generator
	util::Random* m_random;

	/// The vector of aliases
	std::vector<std::size_t> m_alias;

	/// The vector of probabilities
	std::vector<double> m_prob;
};

/// A type that generates random values. The odds of each random value occurring
/// are controlled by a probability.
template <typename T>
class BiasedRandomValueGenerator
{
public:
	BiasedRandomValueGenerator() = delete;
	BiasedRandomValueGenerator(const BiasedRandomValueGenerator&) = delete;
	BiasedRandomValueGenerator& operator=(const BiasedRandomValueGenerator&) = delete;

	BiasedRandomValueGenerator(BiasedRandomValueGenerator&& other) = default;
	BiasedRandomValueGenerator& operator=(BiasedRandomValueGenerator&& other) = default;

	/// Creates a biased random value generator from the given value-to-probability map and
	/// random number generator.
	template <typename TProbabilityMap>
	static BiasedRandomValueGenerator<T> CreateDistribution(const TProbabilityMap& probabilities, util::Random& rng)
	{
		std::vector<T> value_map;
		std::vector<double> prob_vector;
		for (const auto& pair : probabilities) {
			value_map.push_back(pair.first);
			prob_vector.push_back(pair.second);
		}
		auto inner = Alias::CreateDistribution(prob_vector, rng);
		return {std::move(inner), std::move(value_map)};
	}

	/// Generates a new value.
	T Next() { return value_map[inner_alias.Next()]; }

private:
	BiasedRandomValueGenerator(Alias&& inner_alias, std::vector<T>&& value_map)
	    : inner_alias(std::move(inner_alias)), value_map(std::move(value_map))
	{
	}
	Alias inner_alias;
	std::vector<T> value_map;
};
} // end-of-namespace
} // end-of-namespace

#endif
