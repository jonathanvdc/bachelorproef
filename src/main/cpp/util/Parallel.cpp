#include "Parallel.h"

namespace stride {
namespace util {
namespace parallel {

#ifdef PARALLELIZATION_LIBRARY_TBB
namespace {
unsigned int tbb_number_of_threads = tbb::task_scheduler_init::default_num_threads();
}
unsigned int get_number_of_threads() { return tbb_number_of_threads; }

bool try_set_number_of_threads(unsigned int number_of_threads)
{
	if (number_of_threads == 0) {
		return false;
	}
	tbb_number_of_threads = number_of_threads;
	return true;
}

#elif defined PARALLELIZATION_LIBRARY_STL

namespace {
unsigned int stl_number_of_threads = std::thread::hardware_concurrency();
}
unsigned int get_number_of_threads()
{
	auto num_threads = stl_number_of_threads;
	if (num_threads == 0) {
		// std::thread::hardware_concurrency() can return zero if it can't
		// detect the number of cores in the machine. If so, then we'll
		// conservatively use one thread.
		return 1;
	} else {
		return num_threads;
	}
}

bool try_set_number_of_threads(unsigned int number_of_threads)
{
	if (number_of_threads == 0) {
		return false;
	}
	stl_number_of_threads = number_of_threads;
	return true;
}

#elif defined _OPENMP && !defined PARALLELIZATION_LIBRARY_NONE

unsigned int get_number_of_threads()
{
	unsigned int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	return num_threads;
}

bool try_set_number_of_threads(unsigned int number_of_threads)
{
	if (number_of_threads == 0) {
		return false;
	}
	omp_set_num_threads(number_of_threads);
	return true;
}

#else

unsigned int get_number_of_threads() { return 1; }

bool try_set_number_of_threads(unsigned int) { return false; }

#endif
}
}
}