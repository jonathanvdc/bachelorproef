/**
 * @file
 * Configuration data structures for the simulator, built with multi-region in mind.
 */

#include <string>
#include <vector>
#include "SimulationConfig.h"
#include "sim/run_stride.h"

namespace stride {
namespace multiregion {

void SimulationConfig::run() const
{
	for (const auto& file_name : config_file_names) {
		run_stride(track_index_case, file_name);
	}
}

} // namespace
} // namespace