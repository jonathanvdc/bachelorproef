#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <alias/Alias.h>
#include <alias/AliasUtil.h>
#include <gtest/gtest.h>
#include <util/Random.h>

using namespace stride::alias;

namespace Tests {

TEST(Alias, Alias)
{
	stride::util::Random rng(0);
	ASSERT_THROW(Alias::CreateDistribution({}, rng), EmptyProbabilityException);
}

TEST(Alias, Next)
{
	stride::util::Random rng(0);
	Alias::CreateDistribution({1.0}, rng).Next();
}

TEST(Alias, Generate)
{
	stride::util::Random rng(0);
	std::vector<double> probabilities = {0.9, 0.05, 0.05};
	auto distribution = Alias::CreateDistribution(probabilities, rng);
	std::unordered_map<std::size_t, int> frequencies;
	int sample_size = 10000;
	for (int i = 0; i < sample_size; i++) {
		frequencies[distribution.Next()]++;
	}
	for (const auto& pair : frequencies) {
		ASSERT_NEAR(
		    static_cast<double>(pair.second) / static_cast<double>(sample_size), probabilities[pair.first], 0.1)
		    << "key: " << pair.first;
	}
}

TEST(Alias, BiasedRandomValueGenerator)
{
	stride::util::Random rng(0);
	enum Color
	{
		Red,
		Green,
		Blue
	};
	std::unordered_map<Color, double> probabilities;
	probabilities[Red] = 0.9;
	probabilities[Green] = 0.05;
	probabilities[Blue] = 0.05;
	auto random_val_generator = BiasedRandomValueGenerator<Color>::CreateDistribution(probabilities, rng);
	std::unordered_map<Color, int> frequencies;
	int sample_size = 10000;
	for (int i = 0; i < sample_size; i++) {
		frequencies[random_val_generator.Next()]++;
	}
	double cumulative_probability = probabilities[Red] + probabilities[Green] + probabilities[Blue];
	for (const auto& pair : frequencies) {
		ASSERT_NEAR(
		    static_cast<double>(pair.second) / static_cast<double>(sample_size),
		    probabilities[pair.first] / cumulative_probability, 0.1)
		    << "color: " << pair.first;
	}
}

} // Tests