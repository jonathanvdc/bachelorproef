#include <iostream>
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

} // Tests