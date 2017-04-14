#ifndef POPULATION_GENERATOR_H_INCLUDED
#define POPULATION_GENERATOR_H_INCLUDED

/**
 * @file
 * Header file for the population generator class
 */

#include <cstddef>
#include <memory>
#include <vector>
#include "Person.h"
#include "PopulationModel.h"
#include "core/Disease.h"
#include "geo/Profile.h"
#include "util/Random.h"

namespace stride {
namespace population_model {

class Generator
{
public:
	Generator(const ModelRef m, const geo::ProfileRef g, const disease::Disease& d, util::Random& r)
	    : model(m), geo_profile(g), disease(d), random(r)
	{
	}

	/// Generate a random population.
	Population Generate();

	/// Check if a population fits the model.
	/// If verbose is true, log the checks performed.
	bool FitsModel(const Population& population, bool verbose = false);

private:
	const ModelRef model;
	const geo::ProfileRef geo_profile;
	const disease::Disease& disease;
	util::Random& random;

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

} // namespace population_model
} // namespace stride

#endif
