#ifndef RUN_STRIDE_H_INCLUDED
#define RUN_STRIDE_H_INCLUDED
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
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Header for the Simulator class.
 */

#include <string>

namespace stride {

/// Gets the number of threads provided by OpenMP.
unsigned int get_number_of_omp_threads();

/// Prints information about the current execution environment.
void print_execution_environment();

/// Verifies that Stride is being run in the right execution environment.
void verify_execution_environment();

/**
 * Run the simulator with config information provided.
 */
void run_stride(bool track_index_case, const std::string& config_file_name);

} // end_of_namespace

#endif // end-of-include-guard
