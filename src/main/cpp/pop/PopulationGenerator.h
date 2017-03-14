#ifndef POPULATION_GENERATOR_H_INCLUDED
#define POPULATION_GENERATOR_H_INCLUDED

/**
 * @file
 * Header file for the population generator class
 */

#include <memory>
#include <vector>
#include "Person.h"
#include "PopulationModel.h"
#include "util/Random.h"

namespace stride {
namespace population_model {

class Generator
{
public:
	Generator(Model* m, util::Random* r) : model(m), random(r) {}

	/// Generate a random population.
	std::unique_ptr<Population> generate();

private:
	Model* model;
	util::Random* random;
};

} // namespace population_model
} // namespace stride

#endif
