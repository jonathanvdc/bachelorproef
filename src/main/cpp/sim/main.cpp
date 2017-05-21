#include "run_stride.h"

#include <exception>
#include <iostream>
#include <tclap/CmdLine.h>
#include "util/Signals.h"

using namespace std;
using namespace stride;
using namespace TCLAP;

/// Main program of the stride simulator.
int main(int argc, char** argv)
{
	// Set up a signal handler.
	stride::util::setup_segfault_handler();
	stride::util::setup_interrupt_handler();
	int exit_status = EXIT_SUCCESS;
	try {
		// -----------------------------------------------------------------------------------------
		// Parse command line.
		// -----------------------------------------------------------------------------------------
		CmdLine cmd("stride", ' ', "1.0", false);
		SwitchArg index_case_Arg("r", "r0", "R0 only", cmd, false);
		SwitchArg generate_vis_Arg("V", "no-vis", "Don't generate visualization file", cmd, true);
		ValueArg<string> config_file_Arg("c", "config", "Config File", false, "", "CONFIGURATION FILE", cmd);
		SwitchArg hdf5("h", "no-hdf5", "Run without checkpointig", cmd, false);

		ValueArg<string> h5File("f", "h5file", "HDF5 file to write to", false, "", "CHECKPOINT FILE", cmd);

		ValueArg<string> date(
		    "l", "load", "The date to be loaded from the HDF5 file", false, "", "NON-DELIMITED DATE", cmd);

		ValueArg<unsigned int> interval(
		    "i", "interval", "the amount of days between each checkpoint. The first and last are not counted.",
		    false, -1, "", cmd);

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
		run_stride(
		    index_case_Arg.getValue(), config_file_Arg.getValue(), h5File.getValue(), date.getValue(),
		    generate_vis_Arg.getValue(), !hdf5.getValue(), interval.getValue());
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
