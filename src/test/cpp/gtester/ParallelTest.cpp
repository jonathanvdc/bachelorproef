#include <iostream>
#include <vector>
#include <gtest/gtest.h>
#include "util/Parallel.h"
#include "util/ParallelMap.h"
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
using MapForType =
    std::function<void(stride::util::parallel::ParallelMap<K, V>& values, const MapActionType<K, V>& action)>;

void map_test(const MapForType<int, int>& run_for)
{
	stride::util::parallel::ParallelMap<int, int> values;
	for (int i = 0; i < 200; i++) {
		values[i] = 0;
	}
	ASSERT_EQ(values.size(), 200u);
	run_for(values, [](const int& key, int& val, unsigned int) { val = key; });
	ASSERT_EQ(values.size(), 200u);
	for (auto pair : values) {
		ASSERT_EQ(pair.first, pair.second);
	}

	stride::util::parallel::ParallelMap<int, int> empty_map;
	ASSERT_EQ(empty_map.size(), 0u);
	run_for(empty_map, [](const int& key, int& val, unsigned int) { val = key; });
	ASSERT_EQ(empty_map.size(), 0u);

	stride::util::parallel::ParallelMap<int, int> sparse_map;
	sparse_map[100] = 0;
	for (int i = 1000; i < 1010; i++) {
		sparse_map[i] = 0;
	}
	ASSERT_EQ(sparse_map.size(), 11u);
	run_for(sparse_map, [](const int& key, int& val, unsigned int) { val++; });
	ASSERT_EQ(sparse_map.size(), 11u);
	for (auto pair : sparse_map) {
		ASSERT_EQ(pair.second, 1) << " (key: " << pair.first << ")";
	}
}

int fac(int n)
{
	if (n <= 0)
		return 1;
	else
		return n * fac(n - 1);
}

static stride::util::parallel::ParallelMap<int, int> create_perf_test_map()
{
	stride::util::parallel::ParallelMap<int, int> values;
	for (std::size_t i = 0; i < 2000000u; i++) {
		values[i] = 0;
	}
	return values;
}

static stride::util::parallel::ParallelMap<int, int> perf_test_map = create_perf_test_map();

void map_perf_test(const MapForType<int, int>& run_for)
{
	for (int i = 0; i < 20; i++) {
		run_for(perf_test_map, [](const int& key, int& val, unsigned int) { val = fac(key % 20); });
	}
}

TEST(Parallel, MapSerial)
{
	map_test([](stride::util::parallel::ParallelMap<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::serial_for(values, action);
	});
}

TEST(Parallel, MapParallel)
{
	map_test([](stride::util::parallel::ParallelMap<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::parallel_for(values, stride::util::parallel::get_number_of_threads(), action);
	});
}

TEST(Parallel, MapPseudoParallel)
{
	map_test([](stride::util::parallel::ParallelMap<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::parallel_for(values, 1u, action);
	});
}

TEST(Parallel, MapPerfSerial)
{
	map_perf_test([](stride::util::parallel::ParallelMap<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::serial_for(values, action);
	});
}

TEST(Parallel, MapPerfParallel)
{
	map_perf_test([](stride::util::parallel::ParallelMap<int, int>& values, const MapActionType<int, int>& action) {
		stride::util::parallel::parallel_for(values, 2u, action);
	});
}

} // Tests