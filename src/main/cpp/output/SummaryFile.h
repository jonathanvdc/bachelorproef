#ifndef SUMMARY_FILE_H_INCLUDED
#define SUMMARY_FILE_H_INCLUDED

#include "pop/Population.h"
#include "sim/SimulationConfig.h"

#include <fstream>
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace stride {
namespace output {

/**
 * Produces a file with simulation summary output.
 */
class SummaryFile
{
public:
	/// Constructor: initialize.
	SummaryFile(const std::string& file = "stride_summary");

	/// Destructor: close the file stream.
	~SummaryFile();

	/// Print the given output with corresponding tag.
	void Print(
	    const SingleSimulationConfig& config, unsigned int population_size, unsigned int num_cases,
	    unsigned int run_time, unsigned int total_time);

private:
	/// Generate file name and open the file stream.
	void Initialize(const std::string& file);

private:
	std::ofstream m_fstream;
};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
