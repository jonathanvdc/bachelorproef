#include <iostream>
#include <vector>
#include <geo/GeoPosition.h>
#include <gtest/gtest.h>

namespace Tests {

TEST(GeoPosition, SampleCalculation)
{
	using namespace stride::geo;
	const GeoPosition amsterdam{52.37, 4.90};
	const GeoPosition paris{48.86, 2.35};
	ASSERT_NEAR(amsterdam.Distance(paris), 429.7, 1.0);
}

} // Tests