/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Actually run the simulator.
 */

#include "run_stride.h"

#include "multiregion/SequentialSimulationManager.h"
#include "multiregion/SimulationManager.h"
#include "output/CasesFile.h"
#include "output/PersonFile.h"
#include "output/SummaryFile.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "util/ConfigInfo.h"
#include "util/InstallDirs.h"
#include "util/Stopwatch.h"
#include "util/TimeStamp.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <omp.h>
#include <spdlog/spdlog.h>

#include <cmath>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace stride {

using namespace output;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;
using namespace std::chrono;

/// Gets the number of threads provided by OpenMP.
unsigned int get_number_of_omp_threads()
{
	unsigned int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	return num_threads;
}

/// Prints information about the current execution environment.
void print_execution_environment()
{
	cout << "\n*************************************************************" << endl;
	cout << "Starting up at:      " << TimeStamp().ToString() << endl;
	cout << "Executing:           " << InstallDirs::GetExecPath().string() << endl;
	cout << "Current directory:   " << InstallDirs::GetCurrentDir().string() << endl;
	cout << "Install directory:   " << InstallDirs::GetRootDir().string() << endl;
	cout << "Data    directory:   " << InstallDirs::GetDataDir().string() << endl;
}

/// Verifies that Stride is being run in the right execution environment.
void verify_execution_environment()
{
	if (InstallDirs::GetCurrentDir().compare(InstallDirs::GetRootDir()) != 0) {
		throw runtime_error(string(__func__) + "> Current directory is not install root! Aborting.");
	}
	if (InstallDirs::GetDataDir().empty()) {
		throw runtime_error(string(__func__) + "> Data directory not present! Aborting.");
	}
}

/// Run the stride simulator.
void run_stride(bool track_index_case, const string& config_file_name)
{
	// -----------------------------------------------------------------------------------------
	// Print output to command line.
	// -----------------------------------------------------------------------------------------
	print_execution_environment();

	// -----------------------------------------------------------------------------------------
	// Check execution environment.
	// NOTE: the statement below is commented out because it inhibits our ability to write
	// tests for this function.
	// -----------------------------------------------------------------------------------------
	// verify_execution_environment();

	// -----------------------------------------------------------------------------------------
	// Configuration.
	// -----------------------------------------------------------------------------------------
	ptree pt_config;
	const auto file_path = canonical(system_complete(config_file_name));
	if (!is_regular_file(file_path)) {
		throw runtime_error(string(__func__) + ">Config file " + file_path.string() +
				    " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);
	cout << "Configuration file:  " << file_path.string() << endl;

	SingleSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = track_index_case;

	// -----------------------------------------------------------------------------------------
	// OpenMP.
	// -----------------------------------------------------------------------------------------
	unsigned int num_threads = get_number_of_omp_threads();
	if (ConfigInfo::HaveOpenMP()) {
		cout << "Using OpenMP threads:  " << num_threads << endl;
	} else {
		cout << "Not using OpenMP threads." << endl;
	}
	// -----------------------------------------------------------------------------------------
	// Set output path prefix.
	// -----------------------------------------------------------------------------------------
	auto output_prefix = config.log_config->output_prefix;
	if (output_prefix.length() == 0) {
		output_prefix = TimeStamp().ToTag();
	}
	cout << "Project output tag:  " << output_prefix << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Track index case setting.
	// -----------------------------------------------------------------------------------------
	cout << "Setting for track_index_case:  " << boolalpha << track_index_case << endl;

	// -----------------------------------------------------------------------------------------
	// Create logger
	// Transmissions:     [TRANSMISSION] <infecterID> <infectedID> <clusterID> <day>
	// General contacts:  [CNT] <person1ID> <person1AGE> <person2AGE>  <at_home> <at_work> <at_school> <at_other>
	// -----------------------------------------------------------------------------------------
	spdlog::set_async_mode(1048576);
	auto file_logger =
	    spdlog::rotating_logger_mt("contact_logger", output_prefix + "_logfile", std::numeric_limits<size_t>::max(),
				       std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	// -----------------------------------------------------------------------------------------
	// Create simulator.
	// -----------------------------------------------------------------------------------------
	Stopwatch<> total_clock("total_clock", true);
	struct SimulatorResult
	{
		SimulatorResult() : run_clock("run_clock", false), day()
		{
		}

		Stopwatch<> run_clock;
		vector<unsigned int> cases;
		int day;

		static void PreSimStep(SimulatorResult& result, const Population&)
		{
			cout << "Simulating day: " << setw(5) << result.day;
			result.run_clock.Start();
		}

		static void PostSimStep(SimulatorResult& result, const Population& pop)
		{
			result.run_clock.Stop();
			cout << "     Done, infected count: ";
			result.cases.push_back(pop.GetInfectedCount());
			cout << setw(10) << result.cases[result.cases.size() - 1] << endl;
			result.day++;
		}
	};

	multiregion::SequentialSimulationManager<SimulatorResult> sim_manager{num_threads};

	cout << "Building the simulator. " << endl;
	auto sim_task = sim_manager.StartSimulation(config, SimulatorResult(), SimulatorResult::PreSimStep,
						    SimulatorResult::PostSimStep);
	cout << "Done building the simulator. " << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Run the simulation.
	// -----------------------------------------------------------------------------------------
	sim_task->Wait();

	// -----------------------------------------------------------------------------------------
	// Generate output files
	// -----------------------------------------------------------------------------------------
	// Cases
	auto sim_result = sim_task->GetResult();
	CasesFile cases_file(output_prefix);
	cases_file.Print(sim_result.cases);

	// Summary
	SummaryFile summary_file(output_prefix);
	summary_file.Print(config, sim_task->GetPopulationSize(), sim_task->GetInfectedCount(),
			   duration_cast<milliseconds>(sim_result.run_clock.Get()).count(),
			   duration_cast<milliseconds>(total_clock.Get()).count());

	// Persons
	if (config.log_config->generate_person_file) {
		auto pop = std::make_shared<Population>(sim_task->GetPopulation());
		PersonFile person_file(output_prefix);
		person_file.Print(pop);
	}

	// -----------------------------------------------------------------------------------------
	// Print final message to command line.
	// -----------------------------------------------------------------------------------------
	cout << endl << endl;
	cout << "  run_time: " << sim_result.run_clock.ToString() << "  -- total time: " << total_clock.ToString()
	     << endl
	     << endl;
	cout << "Exiting at:         " << TimeStamp().ToString() << endl << endl;

	// Release the log.
	spdlog::drop_all();
}

} // end_of_namespace
