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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S, Broeckhove J
 *  Aerts S, De Haes C, Van der Cruysse J & Van Hauwe L
 */

/**
 * @file
 * Actually run the simulator.
 */

#include "run_stride.h"

#include "checkpoint/CheckPoint.h"
#include "multiregion/ParallelSimulationManager.h"
#include "multiregion/SimulationManager.h"
#include "multiregion/TravelModel.h"
#include "output/CasesFile.h"
#include "output/PersonFile.h"
#include "output/SummaryFile.h"
#include "sim/Simulator.h"
#include "sim/SimulatorBuilder.h"
#include "util/Errors.h"
#include "util/InstallDirs.h"
#include "util/Parallel.h"
#include "util/Stopwatch.h"
#include "util/TimeStamp.h"
#include "util/ExternalVars.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <spdlog/spdlog.h>

#include <cmath>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

std::atomic<bool> stride::util::INTERRUPT(false);

namespace stride {

using namespace output;
using namespace util;
using namespace boost::filesystem;
using namespace boost::property_tree;
using namespace std;
using namespace std::chrono;
using namespace checkpoint;

std::mutex StrideSimulatorResult::io_mutex;

// temporary "global" for demo and testing purposes
//CheckPoint* cp;

/// Performs an action just before a simulator step is performed.
void StrideSimulatorResult::BeforeSimulatorStep(
    const Simulator& sim)
{
	run_clock.Start();
	// saves the start configuration
	/*
	if (day == 0) {
		cp->OpenFile();
		cp->SaveCheckPoint(sim);
		cp->CloseFile();
	}
	*/

	if (util::INTERRUPT){
		exit(-1);
	}
}

/// Performs an action just after a simulator step has been performed.
void StrideSimulatorResult::AfterSimulatorStep(
    const Simulator& sim)
{	
	/*
	if (day != 0) {
		cp->OpenFile();
		cp->SaveCheckPoint(sim);
		cp->CloseFile();
	}
	*/
	run_clock.Stop();
	auto infected_count = sim.GetPopulation()->get_infected_count();
	cases.push_back(infected_count);
	day++;

	lock_guard<mutex> lock(io_mutex);
	cout << "Simulation " << setw(3) << id << ": simulated day: " << setw(5) << (day - 1)
	     << "     Done, infected count: " << setw(10) << infected_count << endl;

	if (util::INTERRUPT){
		exit(-1);
	}
}

/// Prints and returns the number of threads.
unsigned int print_number_of_threads()
{
	unsigned int num_threads = stride::util::parallel::get_number_of_threads();
	if (stride::util::parallel::using_parallelization_library) {
		cout << "Using " << stride::util::parallel::parallelization_library_name << " threads: " << num_threads
		     << endl;
	} else {
		cout << "Not using threads for parallelization." << endl;
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
		FATAL_ERROR("Current directory is not install root! Aborting.");
	}
	if (InstallDirs::GetDataDir().empty()) {
		FATAL_ERROR("Data directory not present! Aborting.");
	}
}

/// Run the stride simulator.
void run_stride(const MultiSimulationConfig& config)
{
	// -----------------------------------------------------------------------------------------
	// OpenMP.
	// -----------------------------------------------------------------------------------------
	unsigned int num_threads = print_number_of_threads();

	// -----------------------------------------------------------------------------------------
	// Set output path prefix.
	// -----------------------------------------------------------------------------------------

	if (config.log_config->output_prefix.length() == 0) {
		config.log_config->output_prefix = TimeStamp().ToTag();
	}
	auto output_prefix = config.log_config->output_prefix;

	/*
	cp->OpenFile();
	cp->WriteConfig(config);
	cp->CloseFile();
	cp->OpenFile();
	SingleSimulationConfig foo = cp->LoadSingleConfig();
	std::cout<<"prefix: "<<foo.log_config->output_prefix<<std::endl;
	cp->CloseFile();
	*/

	cout << "Project output tag:  " << output_prefix << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Track index case setting.
	// -----------------------------------------------------------------------------------------
	cout << "Setting for track_index_case:  " << boolalpha << config.common_config->track_index_case << endl;

	// Set the log queue size.
	spdlog::set_async_mode(1048576);

	// -----------------------------------------------------------------------------------------
	// Create simulator.
	// -----------------------------------------------------------------------------------------
	Stopwatch<> total_clock("total_clock", true);
	// multiregion::SequentialSimulationManager<StrideSimulatorResult, multiregion::RegionId> sim_manager{
	//     num_threads};
	multiregion::ParallelSimulationManager<StrideSimulatorResult, multiregion::RegionId> sim_manager{
	    config.region_models.size(), num_threads};

	// Build all the simulations.
	struct SimulationTuple
	{
		std::string log_name;
		std::string sim_output_prefix;
		SingleSimulationConfig sim_config;
		std::shared_ptr<multiregion::SimulationTask<StrideSimulatorResult>> sim_task;
	};
	std::vector<SimulationTuple> tasks;
	for (const auto& single_config : config.GetSingleConfigs()) {
		multiregion::RegionId region_id = single_config.GetId();
		cout << "Building simulator #" << region_id << endl;
		auto sim_output_prefix = output_prefix + "_sim" + std::to_string(region_id);

		// -----------------------------------------------------------------------------------------
		// Create logger
		// Transmissions:     [TRANSMISSION] <infecterID> <infectedID> <clusterID> <day>
		// General contacts:  [CNT] <person1ID> <person1AGE> <person2AGE>  <at_home> <at_work> <at_school>
		// <at_other>
		// -----------------------------------------------------------------------------------------
		auto log_name = std::string("contact_logger_") + sim_output_prefix;
		auto file_logger = spdlog::rotating_logger_mt(
		    log_name, sim_output_prefix + "_logfile", std::numeric_limits<size_t>::max(),
		    std::numeric_limits<size_t>::max());
		file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

		tasks.push_back(
		    {log_name, sim_output_prefix, single_config,
		     sim_manager.CreateSimulation(single_config, file_logger, region_id)});
	}
	cout << "Done building simulators. " << endl << endl;

	// -----------------------------------------------------------------------------------------
	// Run the simulation.
	// -----------------------------------------------------------------------------------------
	sim_manager.WaitAll();

	// Generate output files for the simulations.
	for (const auto& sim_tuple : tasks) {
		// -----------------------------------------------------------------------------------------
		// Generate output files
		// -----------------------------------------------------------------------------------------
		// Cases
		auto sim_result = sim_tuple.sim_task->GetResult();
		CasesFile cases_file(sim_tuple.sim_output_prefix);
		cases_file.Print(sim_result.cases);

		// Summary
		SummaryFile summary_file(sim_tuple.sim_output_prefix);
		summary_file.Print(
		    sim_tuple.sim_config, sim_tuple.sim_task->GetPopulationSize(),
		    sim_tuple.sim_task->GetInfectedCount(),
		    duration_cast<milliseconds>(sim_result.GetRuntime()).count(),
		    duration_cast<milliseconds>(total_clock.Get()).count());

		// Persons
		if (sim_tuple.sim_config.log_config->generate_person_file) {
			auto pop = sim_tuple.sim_task->GetPopulation();
			PersonFile person_file(sim_tuple.sim_output_prefix);
			person_file.Print(pop);
		}

		cout << endl << endl;
		cout << "  run_time: " << sim_result.GetRuntimeString() << "  -- total time: " << total_clock.ToString()
		     << endl
		     << endl;

		spdlog::drop(sim_tuple.log_name);
	}

	// -----------------------------------------------------------------------------------------
	// Print final message to command line.
	// -----------------------------------------------------------------------------------------
	cout << "Exiting at:         " << TimeStamp().ToString() << endl << endl;
}

/// Run the stride simulator.
void run_stride(const SingleSimulationConfig& config) { run_stride(config.AsMultiConfig()); }

/// Run the stride simulator.
void run_stride(bool track_index_case, const string& config_file_name)
{
	// Parse the configuration.
	ptree pt_config;
	const auto file_path = canonical(system_complete(config_file_name));
	if (!is_regular_file(file_path)) {
		throw runtime_error(
		    string(__func__) + ">Config file " + file_path.string() + " not present. Aborting.");
	}
	read_xml(file_path.string(), pt_config);
	cout << "Configuration file:  " << file_path.string() << endl;

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = track_index_case;

	/*
	cp = new CheckPoint("foo.h5");

	cp->CreateFile();
	cp->OpenFile();
	cp->WriteHolidays(pt_config.get_child("run").get<std::string>("holidays_file", "holidays_flanders_2016.json"));
	cp->CloseFile();
	*/

	// Run Stride.
	run_stride(config);
}

} // end_of_namespace
