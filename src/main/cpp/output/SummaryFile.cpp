#include "SummaryFile.h"

#include <fstream>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include "util/Parallel.h"

namespace stride {
namespace output {

using namespace std;

SummaryFile::SummaryFile(const std::string& file)
{
	m_fstream.open((file + "_summary.csv").c_str());

	// add header
	m_fstream << "pop_file,num_days,pop_size,seeding_rate,"
		  << "R0,transm_rate,immunity_rate,num_threads,rng_seed,run_time,"
		  << "total_time,num_cases,AR" << endl;
}

SummaryFile::~SummaryFile() { m_fstream.close(); }

void SummaryFile::Print(
    const SingleSimulationConfig& config, unsigned int population_size, unsigned int num_cases, unsigned int run_time,
    unsigned int total_time)
{
	unsigned int num_threads = util::parallel::get_number_of_threads();

	m_fstream << config.GetPopulationPath() << "," << config.common_config->number_of_days << "," << population_size
		  << "," << config.common_config->seeding_rate << "," << config.common_config->r0 << ","
		  << "NA"
		  << "," // << pt_config.get<double>("run.transmission_rate") << ";"
		  << config.common_config->immunity_rate << "," << num_threads << "," << config.common_config->rng_seed
		  << "," << run_time << "," << total_time << "," << num_cases << ","
		  << static_cast<double>(num_cases) / population_size << endl;
}

} // end namespace
} // end namespace
