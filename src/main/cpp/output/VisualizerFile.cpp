/**
 * @file
 * Implementation of the VisualizerFile class.
 */

#include "VisualizerFile.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace stride {
namespace output {

using namespace std;

VisualizerFile::VisualizerFile(const std::string& file) { Initialize(file); }

VisualizerFile::~VisualizerFile() { m_fstream.close(); }

void VisualizerFile::Initialize(const std::string& file) { m_fstream.open((file + "_vis.txt").c_str()); }

void VisualizerFile::Print(Atlas::TownMap townMap, const VisualizerData& visualizer_data)
{
	using boost::property_tree::ptree;
	using boost::property_tree::write_json;

	ptree townsTree;

	for (auto& p : townMap)
		townsTree.put(p.second.name, p.second.size);

	std::shared_ptr<ptree> daysTree = visualizer_data.ToPtree();

	ptree fileTree;
	fileTree.add_child("towns", townsTree);
	fileTree.add_child("days", *daysTree);

	write_json(m_fstream, fileTree, false);
}

} // end_of_namespace
} // end_of_namespace
