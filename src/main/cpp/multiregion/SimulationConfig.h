#ifndef MULTIREGION_CONFIG_H_INCLUDED
#define MULTIREGION_CONFIG_H_INCLUDED

/**
 * @file
 * Configuration data structures for the simulator built, with multi-region in mind.
 */

#include <string>
#include <vector>

namespace stride {
namespace multiregion {

/**
 * Defines a simulation configuration, possibly for multiple regions.
 */
struct SimulationConfig final
{
    // <rng_seed>1</rng_seed>
    // <r0>11</r0>
    // <seeding_rate>0.002</seeding_rate>
    // <immunity_rate>0.8</immunity_rate>
    // <population_file>pop_nassau.csv</population_file>
    // <num_days>50</num_days>
    // <output_prefix></output_prefix>
    // <disease_config_file>disease_measles.xml</disease_config_file>
    // <generate_person_file>1</generate_person_file>
    // <num_participants_survey>10</num_participants_survey>
    // <start_date>2017-01-01</start_date>
    // <holidays_file>holidays_none.json</holidays_file>
    // <age_contact_matrix_file>contact_matrix_average.xml</age_contact_matrix_file>
    // <log_level>Transmissions</log_level>

	bool track_index_case;

	/// The names of the single-region configuration files.
	std::vector<std::string> config_file_names;

	/// Runs the simulator for this configuration.
	void run() const;
};

} // namespace
} // namespace

#endif // end-of-include-guard
