/**
 * @file
 * Implementation of the VisualizerFile class.
 */

#include "VisualizerFile.h"

#include <iostream>
#include <memory>
#include <boost/property_tree/json_parser.hpp>

namespace stride {
namespace output {

using namespace std;

VisualizerFile::VisualizerFile(const std::string& file) { Initialize(file); }

VisualizerFile::~VisualizerFile() { m_fstream.close(); }

void VisualizerFile::Initialize(const std::string& file) { m_fstream.open((file + "_vis.json").c_str()); }

void VisualizerFile::Print(const Atlas::TownMap& townMap, const VisualizerData& visualizer_data)
{
	using boost::property_tree::ptree;
	using boost::property_tree::write_json;

	ptree townsTree;

	for (auto& p : townMap) {
		// append a town node for each town in the map:
		// id : {name, size, latitude, longitude}
		ptree townNode;
		townNode.put("name", p.second.name);
		townNode.put("size", p.second.size);
		townNode.put("lat", p.first.latitude);
		townNode.put("long", p.first.longitude);
		townsTree.add_child(to_string(p.second.id), townNode);
	}

	std::shared_ptr<ptree> daysTree = visualizer_data.ToPtree();

	ptree fileTree;
	fileTree.add_child("towns", townsTree);
	fileTree.add_child("days", *daysTree);

	write_json(m_fstream, fileTree, false);
}

} // end_of_namespace
} // end_of_namespace
