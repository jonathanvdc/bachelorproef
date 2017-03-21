/*
 * Alias.cpp
 *
 *  Created on: Mar 11, 2017
 *      Author: cedric
 */

#include "Alias.h"
#include "AliasUtil.h"

#include <exception>
#include <assert.h>
#include <math.h>

namespace stride {
namespace alias {

Alias Alias::CreateDistribution(std::vector<double> probabilities, util::Random& rng)
{
	assert(probabilities.size() > 0);
	if (probabilities.size() <= 0) {
		throw EmptyProbabilityException();
	}
	unsigned int n = probabilities.size();
	std::vector<double> prob(n);
	std::vector<unsigned int> alias(n);
	std::vector<unsigned int> small, large;
	for (auto& i : probabilities) {
		i *= n;
	}
	for (unsigned int i = 0; i < probabilities.size(); i++) {
		if (probabilities[i] < 1.0) {
			small.push_back(i);
		} else {
			large.push_back(i);
		}
	}

	while (!(small.empty() || large.empty())) {
		unsigned int l = large.front();
		large.erase(large.begin());
		unsigned int g = small.front();
		small.erase(small.begin());
		prob[l] = probabilities[l];
		alias[l] = g;

		probabilities[g] = probabilities[g] + probabilities[l] - 1;
		if (probabilities[g] >= 1) {
			large.push_back(g);
		} else {
			small.push_back(g);
		}
	}

	for (auto g : large) {
		prob[g] = 1;
	}

	for (auto l : small) {
		prob[l] = 1;
	}
	Alias a(alias, prob, rng);
	return std::move(a);
}

unsigned int Alias::Next()
{
	unsigned int roll = m_random(m_alias.size());
	double flip = m_random.NextDouble();
	if (flip <= m_prob[roll]) {
		return roll;
	} else {
		return m_alias[roll];
	}
}
}
}
