#ifndef UTIL_PARALLEL_H_INCLUDED
#define UTIL_PARALLEL_H_INCLUDED

/**
 * @file
 * A paper-thin abstraction layer over parallelization libraries.
 */

#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#ifdef PARALLELIZATION_LIBRARY_TBB
#include <tbb/parallel_for.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>
#elif !defined PARALLELIZATION_LIBRARY_STL && !defined PARALLELIZATION_LIBRARY_NONE
#include <omp.h>
#endif

namespace stride {
namespace util {
namespace parallel {

/// Gets the number of threads that are available for parallelization.
unsigned int get_number_of_threads();

/// Tries to set the number of threads to the given value. A Boolean flag
/// is returned that specifies if the number of threads could be set
/// successfully.
bool try_set_number_of_threads(unsigned int number_of_threads);

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
/// parameter is a dummy value.
template <typename T, typename TAction>
void serial_for(std::vector<T>& values, const TAction& action)
{
	for (size_t i = 0; i < values.size(); i++) {
		action(values[i], 0);
	}
}

/// Applies the given action to each element in the given map of values.
/// The action is not applied to elements simultaneously.
/// An action is a function object with signature `void(const K&, V&, unsigned int)`
/// where the first parameter is the value that the action takes and the second
/// parameter is a dummy value.
template <typename K, typename V, typename TAction>
void serial_for(std::map<K, V>& values, const TAction& action)
{
	for (auto& pair : values) {
		action(pair.first, pair.second, 0);
	}
}

/// A default integer implementation of an operation that divides a range into chunks.
template <typename T>
struct CreateChunks
{
	std::vector<T> operator()(const T& range_size, std::size_t number_of_chunks) const
	{
		std::vector<T> results;
		auto chunk_size = range_size / number_of_chunks;
		auto values_start_offset = T();
		for (unsigned int i = 0; i < number_of_chunks; i++) {
			auto next_start_offset = values_start_offset + chunk_size;
			auto values_end_offset = i == number_of_chunks - 1 ? range_size : next_start_offset;

			results.push_back(values_end_offset);
			values_start_offset = values_end_offset;
		}
		return results;
	}
};

/// A thread-safe queue.
template <typename T>
struct ConcurrentQueue final
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

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	ConcurrentQueue<unsigned int> thread_id_pool;
	for (unsigned int i = 0; i < num_threads; i++) {
		thread_id_pool.Enqueue(i);
	}

	tbb::task_scheduler_init init(num_threads);

	tbb::parallel_for(
	    tbb::blocked_range<size_t>(0, values.size()),
	    [&action, &values, &thread_id_pool](const tbb::blocked_range<size_t>& r) {
		    auto thread_id = thread_id_pool.Dequeue();
		    for (size_t i = r.begin(); i != r.end(); i++) {
			    action(values[i], thread_id);
		    }
		    thread_id_pool.Enqueue(thread_id);
	    });
}

#elif defined PARALLELIZATION_LIBRARY_STL

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "STL";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = true;

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	if (num_threads <= 1) {
		// Nothing to parallelize.
		serial_for<T, TAction>(values, action);
	} else {
		// Create num_thread threads and divide the workload statically.
		std::vector<std::thread> thread_pool;
		auto chunks = CreateChunks<std::size_t>()(values.size(), num_threads);
		std::size_t values_start_offset = 0;
		for (std::size_t i = 0; i < chunks.size(); i++) {
			auto next_start_offset = chunks[i];
			thread_pool.emplace_back([&values, &action, i, values_start_offset, next_start_offset] {
				for (size_t j = values_start_offset; j < next_start_offset; j++) {
					action(values[j], i);
				}
			});
			values_start_offset = next_start_offset;
		}

		// Wait for the threads to finish.
		for (auto& thread : thread_pool) {
			thread.join();
		}
	}
}

#elif defined _OPENMP && !defined PARALLELIZATION_LIBRARY_NONE

/// The name of the parallelization library that is in use.
const char* const parallelization_library_name = "OpenMP";

/// Tells if a parallelization library is in use.
const bool using_parallelization_library = true;

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

template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action)
{
	serial_for<T, TAction>(values, action);
}

#endif
}
}
}

#endif