#include <memory>
#include <vector>
#include "Population.h"
#include "PopulationGenerator.h"

namespace stride {
namespace population_model {

std::unique_ptr<Population> Generator::generate()
{
	int population_size = (*random)(model->size);
	int people_generated = 0;

	std::unique_ptr<Population> population(new Population);

	while (people_generated < population_size) {
		// population->emplace_back(Person(people_generated++, age, household_id, school_id, work_id,
		//				primary_community_id, secondary_community_id, start_infectiousness,
		//				start_symptomatic, time_infectious, time_symptomatic));
        break; // TODO
	}

	return std::make_unique<Population>();
}

} // namespace population_model
} // namespace stride
