#ifndef POPULATION_GENERATOR_H_INCLUDED
#define POPULATION_GENERATOR_H_INCLUDED

/**
 * @file
 * Header file for the population generator class
 */

#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include "Household.h"
#include "Model.h"
#include "Person.h"
#include "alias/Alias.h"
#include "core/Disease.h"
#include "geo/Profile.h"
#include "pop/Population.h"
#include "sim/SimulationConfig.h"
#include "util/InclusiveRange.h"
#include "util/Random.h"

namespace stride {
namespace population {

class Generator
{
	using TownBRNG = alias::BiasedRandomValueGenerator<util::InclusiveRange<int>>;

public:
	Generator(
	    const stride::population::ModelRef& m, const geo::ProfileRef& g,
	    const std::shared_ptr<std::vector<ReferenceHousehold>>& h, const disease::Disease& d, util::Random& r)
	    : model(m), geo_profile(g), reference_households(h), disease(d), random(r),
	      town_brng(TownBRNG::CreateDistribution(m->town_distribution, r)), verbose(false)
	{
		std::cout << "Constructed generator" << std::endl;
	}

	// "Constructor" that reads the model, geoprofile and households from a configuration file.
	static std::unique_ptr<Generator> FromConfig(
	    const SingleSimulationConfig& config, const disease::Disease& disease, util::Random& rng);

	/// Generate a random population.
	Population Generate();

	/// Check if a population fits the model.
	/// If verbose is true, log the checks performed.
	bool FitsModel(const Population& population);

	/// Log a debug message.
	template <typename... Args>
	void Debug(const char* fmt, const Args&... args)
	{
		if (!verbose)
			return;
		auto console = spdlog::get("popgen");
		if (!console) {
			console = spdlog::stderr_logger_st("popgen");
			console->set_level(spdlog::level::debug);
			console->set_pattern("\x1b[36;1m[popgen] %v\x1b[0m");
		}
		console->debug(fmt, args...);
	}

	/// Enable/disable logging.
	void Verbose(bool v) { verbose = v; }

private:
	ModelRef model;
	geo::ProfileRef geo_profile;
	std::shared_ptr<std::vector<ReferenceHousehold>> reference_households;
	const disease::Disease& disease;
	util::Random& random;

	TownBRNG town_brng;

	bool verbose;

	/// Get a random reference household.
	const ReferenceHousehold& GetRandomReferenceHousehold() { return random.Sample(*reference_households); }

	/// Sample a random Town size from the model.
	int GetRandomTownSize() { return random(town_brng.Next()); }

	/// Generate a random GeoPosition in the simulation area.
	geo::GeoPosition GetRandomGeoPosition() { return geo_profile->GetRandomGeoPosition(random); }

	/// Find a random GeoPosition map value close to the given origin point.
	template <typename T>
	T& FindLocal(const geo::GeoPosition& origin, std::map<geo::GeoPosition, T>& map, int tries = 5)
	{
		if (map.empty()) {
			FATAL_ERROR("Generator::FindLocal called on empty map.");
		}

		double r = model->search_radius;
		std::vector<std::reference_wrapper<T>> hits;
		for (int i = 0; i < tries; i++, r *= 2.0) {
			for (auto& p : map) {
				if (p.first.Distance(origin) < r) {
					hits.emplace_back(p.second);
				}
			}
			if (!hits.empty()) {
				return random.Sample(hits).get();
			}
		}

		Debug("FindLocal: giving up after {} radius expansions", tries);
		auto it = map.begin();
		std::advance(it, random(map.size()));
		const double distance = it->first.Distance(origin);
		Debug("Settling on distance {} between {} and {}", distance, it->first.ToString(), origin.ToString());
		return it->second;
	}
};

} // namespace population
} // namespace stride

#endif
