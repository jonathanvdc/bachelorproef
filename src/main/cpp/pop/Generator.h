#ifndef POPULATION_GENERATOR_H_INCLUDED
#define POPULATION_GENERATOR_H_INCLUDED

/**
 * @file
 * Header file for the population generator class
 */

#include <cstddef>
#include <memory>
#include <vector>
#include "Household.h"
#include "Model.h"
#include "Person.h"
#include "alias/Alias.h"
#include "core/Disease.h"
#include "geo/Profile.h"
#include "pop/Population.h"
#include "util/InclusiveRange.h"
#include "util/Random.h"

namespace stride {
namespace population {

class Generator
{
	using TownBRNG = alias::BiasedRandomValueGenerator<util::InclusiveRange<int>>;

public:
	Generator(
	    const stride::population::ModelRef& m, const geo::ProfileRef& g, const std::vector<ReferenceHousehold>& h,
	    const disease::Disease& d, util::Random& r)
	    : model(m), geo_profile(g), reference_households(h), disease(d), random(r),
	      town_brng(TownBRNG::CreateDistribution(m->town_distribution, r))
	{
	}

	/// Get a random reference household.
	const ReferenceHousehold& GetRandomReferenceHousehold() { return random.Sample(reference_households); }

	/// Sample a random Town size from the model.
	int GetRandomTownSize() { return random(town_brng.Next()); }

	/// Generate a random GeoPosition in the simulation area.
	geo::GeoPosition GetRandomGeoPosition()
	{
		const auto area = geo_profile->GetSimulationArea();
		return geo::GeoPosition{random(area.min_latitude, area.max_latitude),
					random(area.min_longitude, area.max_longitude)};
	}

	/// Find a random GeoPosition map value close to the given origin point.
	template <typename T>
	T& FindLocal(const geo::GeoPosition& origin, const std::map<geo::GeoPosition, T>& map);

	/// Generate a random population.
	Population Generate();

	/// Check if a population fits the model.
	/// If verbose is true, log the checks performed.
	bool FitsModel(const Population& population, bool verbose = false);

private:
	ModelRef model;
	geo::ProfileRef geo_profile;
	const std::vector<ReferenceHousehold>& reference_households;
	const disease::Disease& disease;
	util::Random& random;

	TownBRNG town_brng;

	/*
	int num_schools;
	int num_works;
	int num_communities;
	int people_generated;
	unsigned int household_id;

	void GenerateHousehold(Population& population, int size);
	void GeneratePerson(Population& population, int age);
	unsigned int SchoolID(int age);
	unsigned int WorkID(int age);
	unsigned int CommunityID();

	std::vector<int> SampleApart(InclusiveRange<int> range, InclusiveRange<int> gap, std::size_t count);
	*/
};

} // namespace population
} // namespace stride

#endif
