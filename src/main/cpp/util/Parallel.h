#ifndef UTIL_PARALLEL_H_INCLUDED
#define UTIL_PARALLEL_H_INCLUDED

/**
 * @file
 * A paper-thin abstraction layer over parallelization technologies.
 */

#include <mutex>
#include <queue>
#include <vector>

#ifdef USE_TBB
#include <tbb/parallel_for.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>
#else
#include <omp.h>
#endif

namespace stride {
namespace util {
namespace parallel {

/// Gets the number of threads that are available for parallelization.
unsigned int get_number_of_threads();

/// Applies the given action to each element in the given list of values.
template <typename T, typename TAction>
void parallel_for(std::vector<T>& values, unsigned int num_threads, const TAction& action);

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

#ifdef USE_TBB

inline unsigned int get_number_of_threads() { return 4; }

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

#else

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

#endif
}
}
}

#endif