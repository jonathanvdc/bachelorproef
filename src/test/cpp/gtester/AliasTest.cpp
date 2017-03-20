#include <iostream>
#include <vector>
#include <gtest/gtest.h>
#include <alias/Alias.h>
#include <alias/AliasUtil.h>

using namespace stride::alias;

namespace Tests {

TEST(Alias, Alias)
{	
	ASSERT_THROW(Alias({}),EmptyProbabilityException);
	ASSERT_THROW(Alias({},4),EmptyProbabilityException);
}

TEST(Alias, Next)
{
	std::vector<double> probs = {1.0};
	Alias a = Alias(probs,1);
	ASSERT_TRUE(true);
}

} // Tests