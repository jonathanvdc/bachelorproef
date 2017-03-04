#include "pop/PopulationModel.h"
#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace Tests {

TEST(ParsePopulationModel, ParseDefaultPopulationModel) {
	std::ifstream pop_file{ "data/popgen_default.xml" };
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::PopulationModel model{ pt };
	pop_file.close();

	EXPECT_EQ(model.elbow_age, 65);
	EXPECT_DOUBLE_EQ(model.p_higher_education, 0.25);
	EXPECT_EQ(model.family_size_distribution[0], 12);
	EXPECT_EQ(model.family_size_distribution[5], 9);
	EXPECT_EQ(model.family_size_distribution.size(), std::size_t{ 6 });
}

} // Tests
