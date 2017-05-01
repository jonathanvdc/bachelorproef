#ifndef UTIL_PARALLEL_H_INCLUDED
#define UTIL_PARALLEL_H_INCLUDED

/**
 * @file
 * A paper-thin abstraction layer over parallelization technologies.
 */

#include <omp.h>

namespace stride {
namespace util {
namespace parallel {
/// Gets the number of threads that are available for parallelization.
inline unsigned int get_number_of_threads()
{
	unsigned int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	return num_threads;
}
}
}
}

#endif