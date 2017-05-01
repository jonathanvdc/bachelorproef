#ifndef UTIL_PARALLEL_H_INCLUDED
#define UTIL_PARALLEL_H_INCLUDED

/**
 * @file
 * A paper-thin abstraction layer over parallelization libraries.
 */

#include <mutex>
#include <queue>
#include <vector>

#ifdef PARALLELIZATION_LIBRARY_TBB
#include <tbb/parallel_for.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>
#elif defined PARALLELIZATION_LIBRARY_STL
#include <thread>
#else
#include <omp.h>
#endif

namespace stride {
namespace util {

namespace parallel {

/// Gets the number of threads that are available for parallelization.
unsigned int get_number_of_threads();

/// Applies the given action to each element in the given list of values.
/// The action may be applied to up to num_threads elements simultaneously.
/// An action is a function object with signature `void(T&, unsigned int)`
/// where the first parameter is the value that the action takes and the second
/// parameter is the index of the thread it runs on.
template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action);

/// Applies the given action to each element in the given list of values.
/// The action is not applied to multiple elements simultaneously.
/// An action is a function object with signature `void(T&, unsigned int)`
/// where the first parameter is the value that the action takes and the second
/// parameter is the index of the thread it runs on.
template <typename T, typename TAction>
void serial_for(std::vector<T>& values, const TAction& action)
{
	for (size_t i = 0; i < values.size(); i++) {
		action(values[i], 0);
	}
}

/// A thread-safe queue.
template <typename T>
struct ConcurrentQueue
{
	/// Dequeues an item from this concurrent queue.
	T Dequeue()
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto result = queue.front();
		queue.pop();
		return result;
	}

	/// Enqueues an item in this concurrent queue.
	void Enqueue(const T& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(value);
	}

private:
	std::queue<T> queue;
	std::mutex mutex;
};

#ifdef PARALLELIZATION_LIBRARY_TBB

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "TBB";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = true;

inline unsigned int get_number_of_threads() { return tbb::task_scheduler_init::default_num_threads(); }

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	ConcurrentQueue<unsigned int> thread_ids;
	for (unsigned int i = 0; i < num_threads; i++) {
		thread_ids.Enqueue(i);
	}

	tbb::task_scheduler_init init(num_threads);

	tbb::parallel_for(
	    tbb::blocked_range<size_t>(0, values.size()),
	    [&action, &values, &thread_ids](const tbb::blocked_range<size_t>& r) {
		    auto thread_id = thread_ids.Dequeue();
		    for (size_t i = r.begin(); i != r.end(); i++) {
			    action(values[i], thread_id);
		    }
		    thread_ids.Enqueue(thread_id);
	    });
}

#elif defined PARALLELIZATION_LIBRARY_STL

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "STL";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = true;

inline unsigned int get_number_of_threads()
{
	unsigned int num_threads = std::thread::hardware_concurrency();
	if (num_threads == 0) {
		// std::thread::hardware_concurrency() can return zero if it can't
		// detect the number of cores in the machine. If so, then we'll
		// conservatively use one thread.
		return 1;
	} else {
		return num_threads;
	}
}

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	if (num_threads <= 1) {
		// Nothing to parallelize.
		serial_for(values, action);
	} else {
		// Create num_thread threads and divide the workload statically.
		std::vector<std::thread> thread_pool;
		size_t workload_per_thread = values.size() / num_threads;
		size_t values_start_offset = 0;
		for (unsigned int i = 0; i < num_threads; i++) {
			size_t next_start_offset = values_start_offset + workload_per_thread;
			size_t values_end_offset = i == num_threads - 1 ? values.size() : next_start_offset;
			if (values_end_offset == next_start_offset) {
				continue;
			}

			thread_pool.emplace_back([&values, &action, i, values_start_offset, values_end_offset] {
				for (size_t j = values_start_offset; j < values_end_offset; j++) {
					action(values[j], i);
				}
			});
		}

		// Wait for the threads to finish.
		for (auto& thread : thread_pool) {
			thread.join();
		}
	}
}

#elif defined _OPENMP

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "OpenMP";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = true;

inline unsigned int get_number_of_threads()
{
	unsigned int num_threads;
#pragma omp parallel
	{
		num_threads = omp_get_num_threads();
	}
	return num_threads;
}

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
#pragma omp parallel for num_threads(num_threads) schedule(runtime)
	for (size_t i = 0; i < values.size(); i++) {
		const unsigned int thread_id = omp_get_thread_num();
		action(values[i], thread_id);
	}
}

#else

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "serial";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = false;

inline unsigned int get_number_of_threads() { return 1; }

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	for (size_t i = 0; i < values.size(); i++) {
		action(values[i], 0);
	}
}

#endif
}
}
}

#endif