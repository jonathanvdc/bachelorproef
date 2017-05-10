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
 * Main program: command line handling.
 */

#include "run_stride.h"

#include <exception>
#include <iostream>
#include <tclap/CmdLine.h>

#ifndef HANDLE_SIGNALS
// Handle signals unless otherwise specified. This is especially useful for debugging
// segfaults that occur on a remote machine.
#define HANDLE_SIGNALS 1
#endif

#if HANDLE_SIGNALS

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>

void handle_segfault(int sig)
{
	// This signal handler is based on the signal handler from this StackOverflow answer:
	//
	// http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
	//
	// by Todd Gamblin, edited by Violet Giraffe.

	void* array[50];
	size_t size;

	// Get void*s for all entries on the stack.
	size = backtrace(array, 50);

	// Print out all the frames to stderr.
	if (sig == SIGSEGV)
	{
		// This branch should always be taken.
		fprintf(stderr, "error: segmentation fault\n");
	}
	else
	{
		fprintf(stderr, "error: signal %d:\n", sig);
	}
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

#endif

using namespace std;
using namespace stride;
using namespace TCLAP;

/// Main program of the stride simulator.
int main(int argc, char** argv)
{
	#if HANDLE_SIGNALS
	// Set up a signal handler.
	signal(SIGSEGV, handle_segfault);
	#endif

	int exit_status = EXIT_SUCCESS;
	try {
		// -----------------------------------------------------------------------------------------
		// Parse command line.
		// -----------------------------------------------------------------------------------------
		CmdLine cmd("stride", ' ', "1.0", false);
		SwitchArg index_case_Arg("r", "r0", "R0 only", cmd, false);
		ValueArg<string> config_file_Arg(
		    "c", "config", "Config File", false, "./config/run_default.xml", "CONFIGURATION FILE", cmd);
		cmd.parse(argc, argv);

		// -----------------------------------------------------------------------------------------
		// Print output to command line.
		// -----------------------------------------------------------------------------------------
		print_execution_environment();

		// -----------------------------------------------------------------------------------------
		// Check execution environment.
		// -----------------------------------------------------------------------------------------
		verify_execution_environment();

		// -----------------------------------------------------------------------------------------
		// Run the Stride simulator.
		// -----------------------------------------------------------------------------------------
		run_stride(index_case_Arg.getValue(), config_file_Arg.getValue());
	} catch (exception& e) {
		exit_status = EXIT_FAILURE;
		cerr << "\nEXCEPION THROWN: " << e.what() << endl;
	} catch (...) {
		exit_status = EXIT_FAILURE;
		cerr << "\nEXCEPION THROWN: "
		     << "Unknown exception." << endl;
	}
	return exit_status;
}
