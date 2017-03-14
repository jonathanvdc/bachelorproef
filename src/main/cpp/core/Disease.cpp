#include <iostream>
#include "Disease.h"

namespace stride {
namespace disease {

unsigned int Distribution::Sample(Random& rng)
{
	const double random_value = rng.NextDouble();
	for (unsigned int i = 0; i < probabilities.size(); i++) {
		if (random_value <= probabilities[i]) {
			return i;
		}
	}

	std::cerr << "WARNING: PROBLEM WITH DISEASE DISTRIBUTION [disease::Distribution]" << std::endl;
	return 0;
}

std::unique_ptr<Distribution> Distribution::Parse(const ptree& pt_probability_list)
{
	std::vector<double> probabilities;

	for (const auto& i : pt_probability_list) {
		if (i.first != "probability") {
			throw ptree_error("Probability list should consist of <probability> tags only");
		}
		probabilities.push_back(i.second.get_value<double>());
	}

	return std::make_unique<Distribution>(probabilities);
}

Fate Disease::Sample(Random& rng)
{
	const unsigned int si = start_infectiousness.Sample(rng);
	const unsigned int ss = start_symptomatic.Sample(rng);
	const unsigned int ti = time_infectious.Sample(rng);
	const unsigned int ts = time_symptomatic.Sample(rng);
	return Fate{si, ss, ti, ts};
}

std::unique_ptr<Disease> Disease::Parse(const ptree& pt_disease)
{
	return std::make_unique<Disease>( //
	    *Distribution::Parse(pt_disease.get_child("disease.start_infectiousness")),
	    *Distribution::Parse(pt_disease.get_child("disease.start_symptomatic")),
	    *Distribution::Parse(pt_disease.get_child("disease.time_infectious")),
	    *Distribution::Parse(pt_disease.get_child("disease.time_symptomatic")));
}

} // namespace disease
} // namespace stride
