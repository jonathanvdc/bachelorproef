#include <iostream>
#include <mutex>
#include <vector>
#include <gtest/gtest.h>
#include "util/Parallel.h"
#include "util/Random.h"

namespace Tests {

template <typename T>
using VectorActionType = std::function<void(T& val, unsigned int thread_number)>;

template <typename T>
using VectorForType = std::function<void(std::vector<T>& values, const VectorActionType<T>& action)>;

void vector_test(const VectorForType<int>& run_for)
{
	std::vector<int> values(200u, 0);
	ASSERT_EQ(values.size(), 200u);
	run_for(values, [](int& val, unsigned int) { val++; });
	for (int val : values) {
		ASSERT_EQ(val, 1);
	}

	std::vector<int> empty_vector;
	ASSERT_EQ(empty_vector.size(), 0u);
	run_for(empty_vector, [](int& val, unsigned int) { val++; });
	ASSERT_EQ(empty_vector.size(), 0u);
}

TEST(Parallel, VectorSerial)
{
	vector_test([](std::vector<int>& values, const VectorActionType<int>& action) {
		stride::util::parallel::serial_for(values, action);
	});
}

TEST(Parallel, VectorParallel)
{
	vector_test([](std::vector<int>& values, const VectorActionType<int>& action) {
		stride::util::parallel::parallel_for(values, stride::util::parallel::get_number_of_threads(), action);
	});
}

TEST(Parallel, VectorPseudoParallel)
{
	vector_test([](std::vector<int>& values, const VectorActionType<int>& action) {
		stride::util::parallel::parallel_for(values, 1, action);
	});
}

template <typename K, typename V>
using MapActionType = std::function<void(const K& key, V& val, unsigned int thread_number)>;

template <typename K, typename V>
using MapForType = std::function<void(std::map<K, V>& values, const MapActionType<K, V>& action)>;

void map_test(const MapForType<int, int>& run_for)
{
	std::map<int, int> values;
	for (int i = 0; i < 200; i++) {
		values[i] = 0;
	}
	ASSERT_EQ(values.size(), 200u);
	std::mutex mutex;
	run_for(values, [&mutex](const int& key, int& val, unsigned int) {
		{
			std::lock_guard<std::mutex> guard(mutex);
			std::cout << "visiting map key " << key << std::endl;
		}
		val = key;
	});
	for (auto pair : values) {
		ASSERT_EQ(pair.first, pair.second);
	}

	std::map<int, int> empty_map;
	ASSERT_EQ(empty_map.size(), 0u);
	run_for(empty_map, [](const int& key, int& val, unsigned int) { val = key; });
	ASSERT_EQ(empty_map.size(), 0u);
}

TEST(Parallel, MapSerial)
{
	map_test([](std::map<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::serial_for(values, action);
	});
}

TEST(Parallel, MapParallel)
{
	map_test([](std::map<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::parallel_for(values, stride::util::parallel::get_number_of_threads(), action);
	});
}

TEST(Parallel, MapPseudoParallel)
{
	map_test([](std::map<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::parallel_for(values, 1, action);
	});
}

} // Tests