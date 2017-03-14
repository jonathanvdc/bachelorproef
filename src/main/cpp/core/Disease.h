#ifndef DISEASE_H_INCLUDED
#define DISEASE_H_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include "util/Random.h"

namespace stride {
namespace disease {

using boost::property_tree::ptree;
using boost::property_tree::ptree_error;
using stride::util::Random;

// A Fate records how many days into the simulation a Person will become
// infectious/symptomatic, and for how long. It is assigned to each Person
// when the simulation starts, hence the name.
struct Fate
{
	unsigned int start_infectiousness;
	unsigned int start_symptomatic;
	unsigned int time_infectious;
	unsigned int time_symptomatic;
};

// A Distribution is, essentially, a list `v` of n doubles that sum to 1, representing
// a probability distribution over {0, 1, ..., n-1}. See the Sample method.
class Distribution
{
public:
	Distribution(const std::vector<double>& p) : probabilities(p) {}
	Distribution(std::vector<double>&& p) : probabilities(std::move(p)) {}

	// Return a random index into the `probabilities` vector, yielding `i` with
	// probability `v[i]`.
	unsigned int Sample(Random& rng);

	static std::unique_ptr<Distribution> Parse(const ptree& pt_probability_list);

private:
	std::vector<double> probabilities;
};

// Describes the behavior of a disease, as a record of Distributions
// that are sampled to produce Fates.
class Disease
{
public:
	Disease(Distribution start_infectiousness, Distribution start_symptomatic, Distribution time_infectious,
			Distribution time_symptomatic)
	    : start_infectiousness(start_infectiousness), start_symptomatic(start_symptomatic),
	      time_infectious(time_infectious), time_symptomatic(time_symptomatic)
	{
	}

	Fate Sample(Random& rng);

	static std::unique_ptr<Disease> Parse(const ptree& pt_disease);

private:
	Distribution start_infectiousness;
	Distribution start_symptomatic;
	Distribution time_infectious;
	Distribution time_symptomatic;
};

} // namespace disease
} // namespace stride

#endif
