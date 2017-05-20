#ifndef DISEASE_H_INCLUDED
#define DISEASE_H_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include "util/Random.h"

namespace stride {
namespace disease {

// A Fate records how many days into the simulation a Person will become
// infectious/symptomatic, and for how long. It is assigned to each Person
// when the simulation starts, hence the name.
struct Fate
{
	unsigned int start_infectiousness;
	unsigned int start_symptomatic;
	unsigned int end_infectiousness;
	unsigned int end_symptomatic;
};

// A Distribution is, essentially, the cumulative sum of a list `v` of n positive reals
// that sum to 1, representing a probability distribution over {0, 1, ..., n-1}.
// See the Sample method.
class Distribution
{
public:
	Distribution(const std::vector<double>& p) : probabilities(p) {}
	Distribution(std::vector<double>&& p) : probabilities(std::move(p)) {}

	// Return a random index into the `probabilities` vector, yielding `i` with
	// probability `v[i]`.
	unsigned int Sample(util::Random& rng) const;

	static std::unique_ptr<Distribution> Parse(const boost::property_tree::ptree& pt_probability_list);

private:
	// The cumulative sum of `v`; that is, `probabilities[n]` equals `sum(k=0..n) v[k]`.
	// This means `probabilities` is an ascending list of reals, ending in 1.
	std::vector<double> probabilities;
};

// Describes the behavior of a disease, as a record of Distributions
// that are sampled to produce Fates.
class Disease
{
public:
	Disease(
	    Distribution start_infectiousness, Distribution start_symptomatic, Distribution time_infectious,
	    Distribution time_symptomatic)
	    : start_infectiousness(start_infectiousness), start_symptomatic(start_symptomatic),
	      time_infectious(time_infectious), time_symptomatic(time_symptomatic)
	{
	}

	Fate Sample(util::Random& rng) const;

	static std::unique_ptr<Disease> Parse(const boost::property_tree::ptree& pt_disease);

private:
	Distribution start_infectiousness;
	Distribution start_symptomatic;
	Distribution time_infectious;
	Distribution time_symptomatic;
};

} // namespace disease
} // namespace stride

#endif
