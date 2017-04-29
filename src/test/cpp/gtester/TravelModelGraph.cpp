#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gtest/gtest.h>
#include "core/LogMode.h"
#include "multiregion/TravelModel.h"

namespace Tests {

TEST(TravelModelGraph, VerifyBoostGraph)
{
	// First, parse the default travel model.

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
	auto first_region_travel_model = travel_model[0];

	// Next, convert it to a boost graph.

	auto boost_graph = first_region_travel_model->ToBoostGraph();
	using BoostGraph = decltype(boost_graph);
	ASSERT_EQ(boost_graph[0].region_id, 0u);
	ASSERT_DOUBLE_EQ(boost_graph[0].passenger_fraction, 0.01);
	ASSERT_EQ(boost_graph[1].region_id, 1u);
	ASSERT_DOUBLE_EQ(boost_graph[1].passenger_fraction, 0.01);

	boost::property_map<BoostGraph, boost::edge_weight_t>::type edge_weight_map =
	    get(boost::edge_weight_t(), boost_graph);
	std::pair<BoostGraph::edge_descriptor, bool> route1 = boost::edge(0, 1, boost_graph);
	std::pair<BoostGraph::edge_descriptor, bool> route2 = boost::edge(1, 0, boost_graph);

	ASSERT_TRUE(route1.second);
	ASSERT_TRUE(route2.second);
	ASSERT_DOUBLE_EQ(edge_weight_map[route1.first], 2.0);
	ASSERT_DOUBLE_EQ(edge_weight_map[route2.first], 1.0);
}

} // Tests
