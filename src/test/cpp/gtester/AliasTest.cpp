#include <iostream>
#include <vector>
#include <gtest/gtest.h>
#include <alias/Alias.h>

using namespace stride::alias;

namespace Tests {

TEST(AliasDeathTest, Alias)
{
	ASSERT_DEATH(Alias({}),"probabilities.size()!= 0");
	ASSERT_DEATH(Alias({},4),"probabilities.size()!= 0");
}

TEST(Alias, Next)
{
	std::vector<double> probs = {1.0};
	Alias a = Alias(probs,1);
}

} // Tests