#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gtest/gtest.h>
#include "pop/PopulationModel.h"

namespace Tests {

TEST(ParsePopulationModel, ParseDefaultPopulationModel)
{
	std::ifstream pop_file{"../data/population_model_default.xml"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::population_model::Model model;
	model.parse(pt);

	EXPECT_EQ(model.school_size, 500);
	EXPECT_DOUBLE_EQ(model.school_radius, 10.0);
}

TEST(ParsePopulationModel, ExceptionOnInvalidFile)
{
	std::istringstream pop_file{"<a>123</a>"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::population_model::Model model;
	EXPECT_THROW(model.parse(pt), boost::property_tree::ptree_error);
}

} // Tests
