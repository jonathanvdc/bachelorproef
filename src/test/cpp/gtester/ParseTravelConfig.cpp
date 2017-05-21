#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gtest/gtest.h>
#include "core/LogMode.h"
#include "multiregion/TravelModel.h"

namespace Tests {

void assert_default_travel_config(const std::vector<stride::multiregion::RegionTravelRef>& regions)
{
	ASSERT_EQ(regions.size(), 2u);
	ASSERT_EQ(regions[0]->GetRegionPopulationPath(), "pop_nassau.csv");
	ASSERT_EQ(regions[1]->GetRegionPopulationPath(), "pop_oklahoma.csv");
	ASSERT_DOUBLE_EQ(regions[0]->GetTravelFraction(), 0.01);
	ASSERT_DOUBLE_EQ(regions[1]->GetTravelFraction(), 0.01);
	ASSERT_EQ(regions[0]->GetLocalAirports().size(), 1u);
	ASSERT_EQ(regions[1]->GetLocalAirports().size(), 1u);
	ASSERT_DOUBLE_EQ(regions[0]->GetLocalAirports()[0]->passenger_fraction, 2);
	ASSERT_DOUBLE_EQ(regions[1]->GetLocalAirports()[0]->passenger_fraction, 1);
	ASSERT_EQ(regions[0]->GetLocalAirports()[0]->routes.size(), 1u);
	ASSERT_EQ(regions[1]->GetLocalAirports()[0]->routes.size(), 1u);
	ASSERT_DOUBLE_EQ(regions[0]->GetLocalAirports()[0]->routes[0].passenger_fraction, 3.0);
	ASSERT_EQ(regions[0]->GetLocalAirports()[0]->routes[0].target, regions[1]->GetLocalAirports()[0].get());
	ASSERT_DOUBLE_EQ(regions[1]->GetLocalAirports()[0]->routes[0].passenger_fraction, 1.0);
	ASSERT_EQ(regions[1]->GetLocalAirports()[0]->routes[0].target, regions[0]->GetLocalAirports()[0].get());
}

TEST(ParseTravelModel, ParseDefaultTravelModel)
{
	// <travel_model>
	//     <region population_file="pop_nassau.csv" travel_fraction="0.01">
	//         <airport name="JFK" passenger_fraction="2">
	//             <route passenger_fraction="3">FDR</route>
	//         </airport>
	//     </region>
	//     <region population_file="pop_oklahoma.csv" travel_fraction="0.01">
	//         <airport name="FDR">
	//             <route>JFK</route>
	//         </airport>
	//     </region>
	// </travel_model>

	std::ifstream pop_file{"../data/travel_test.xml"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	auto travel_model = stride::multiregion::RegionTravel::ParseRegionTravel(pt.get_child("travel_model"));

	assert_default_travel_config(travel_model);
}

} // Tests
