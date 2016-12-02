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
 * Main program of the simulator.
 */

#include "sim/Simulator.h"
#include "output/CasesFile.h"
#include "output/PersonFile.h"
#include "output/SummaryFile.h"
#include "util/InstallDirs.h"
#include "util/Stopwatch.h"
#include "util/TimeStamp.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include "spdlog/spdlog.h"
#include "tclap/CmdLine.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <omp.h>

using namespace std;
using namespace std::chrono;
using namespace TCLAP;
using namespace indismo;
using namespace output;
using namespace util;

/// Main program of the indismo2 simulator.
int main(int argc, char** argv)
{
	int exit_status = EXIT_SUCCESS;
	try {
                // -----------------------------------------------------------------------------------------
                // Print output to command line.
                // -----------------------------------------------------------------------------------------
                cout << "\n*************************************************************" << endl;
                cout << "Starting up at:      " << TimeStamp().ToString() << endl;
                cout << "Executing:           " << InstallDirs::GetExecName().string() << endl;
                cout << "Current directory:   " << InstallDirs::GetCurrentDir().string() << endl;
                cout << "Install directory:   " << InstallDirs::GetRootDir().string() << endl;
                cout << "Config  directory:   " << InstallDirs::GetConfigDir().string() << endl;
                cout << "Data    directory:   " << InstallDirs::GetConfigDir().string() << endl;

                // -----------------------------------------------------------------------------------------
                // Check execution environment.
                // -----------------------------------------------------------------------------------------
	        if ( InstallDirs::GetCurrentDir().compare(InstallDirs::GetRootDir()) != 0 ) {
	                throw runtime_error("Current directory is not install root! Aborting.");
	        }
                if ( InstallDirs::GetConfigDir().empty() ) {
                        throw runtime_error("Config dir not present! Aborting.");
                }
                if ( InstallDirs::GetDataDir().empty() ) {
                        throw runtime_error("Data dir not present! Aborting.");
                }

		// -----------------------------------------------------------------------------------------
		// Parse command line.
		// -----------------------------------------------------------------------------------------
		CmdLine cmd("indismo", ' ', "3.0", false);

		ValueArg<string> 	config_file_Arg(
		"c", "config_file", "Config File", false, "run_config_default.xml", "CONFIGURATION FILE", cmd);

		cmd.parse(argc, argv);

		// -----------------------------------------------------------------------------------------
		// Parse configuration file.
		// -----------------------------------------------------------------------------------------
		boost::property_tree::ptree pt_config;
		const string config_file = (InstallDirs::GetConfigDir() /= config_file_Arg.getValue()).string();
		read_xml(config_file, pt_config);

		// -----------------------------------------------------------------------------------------
		// Set output path.
		// -----------------------------------------------------------------------------------------
		string output_prefix = pt_config.get<string>("run.output_prefix");
		if (output_prefix.length() == 0) {
			output_prefix = TimeStamp().ToTag();
		}

		// -----------------------------------------------------------------------------------------
		// Set additional run configurations.
		// -----------------------------------------------------------------------------------------
		if (pt_config.get_optional<bool>("run.num_participants_survey") == false) {
			pt_config.put("run.num_participants_survey", 1);
		}

		// -----------------------------------------------------------------------------------------
		// Create logger
		// Transmissions: 		[TRANSMISSION] <infecterID> <infectedID> <clusterID> <day>
		// General contacts: 	[CNT] <person1ID> <person1AGE> <person2AGE>  <at_home> <at_work> <at_school> <at_other>
		// -----------------------------------------------------------------------------------------
		spdlog::set_async_mode(1048576);
		auto file_logger = spdlog::rotating_logger_mt("contact_logger", output_prefix + "_logfile",
		                        std::numeric_limits<size_t>::max(),  std::numeric_limits<size_t>::max());
		file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

		// -----------------------------------------------------------------------------------------
		// Print output to command line.
		// -----------------------------------------------------------------------------------------
		cout << "Project output tag:  " << output_prefix << endl;
		cout << "Configuration file:  " << config_file << endl;
		cout << endl << endl;

		// -----------------------------------------------------------------------------------------
		// Run the simulation.
		// -----------------------------------------------------------------------------------------
		// Create simulator
		Stopwatch<> total_clock("total_clock", true);
		Simulator sim(pt_config);

		// Run
		Stopwatch<> run_clock("run_clock");
		unsigned int num_days = pt_config.get<unsigned int>("run.num_days");
		vector<unsigned int> cases(num_days);
		for (unsigned int i = 0; i < num_days; i++) {
			run_clock.Start();
			sim.RunTimeStep();
			run_clock.Stop();
			cases[i] = sim.GetInfectedCount();
		}

		// -----------------------------------------------------------------------------------------
		// Generate output files
		// -----------------------------------------------------------------------------------------
		// Cases
		CasesFile    cases_file(output_prefix);
		cases_file.Print(cases);

		// Summary
		SummaryFile  summary_file(output_prefix);
		summary_file.Print(pt_config,
			sim.GetPopulationSize(), sim.GetInfectedCount(),
			duration_cast<milliseconds>(run_clock.Get()).count(),
			duration_cast<milliseconds>(total_clock.Get()).count());

		// Person
		if (pt_config.get<double>("run.generate_person_file") == 1) {
			PersonFile	 person_file(output_prefix);
			person_file.Print(sim.GetPopulation());
		}

		// -----------------------------------------------------------------------------------------
		// Print final message to command line.
		// -----------------------------------------------------------------------------------------
                cerr << "  run_time: " << run_clock.ToString()
                        << "  -- total time: " << total_clock.ToString() << endl << endl;
		cout << "Exiting at:         " << TimeStamp().ToString() << endl << endl;
	}
	catch (exception& e) {
		exit_status = EXIT_FAILURE;
		cerr << e.what() << endl;
	}
	catch (...) {
		exit_status = EXIT_FAILURE;
		cerr << "Unknown exception." << endl;
	}
	return exit_status;
}
