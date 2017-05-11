/**
 * @file
 * Implementation of the VisualizerFile class.
 */

#include "VisualizerFile.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace stride {
namespace output {

using namespace std;

VisualizerFile::VisualizerFile(const std::string& file)
{
	Initialize(file);
}

VisualizerFile::~VisualizerFile()
{
	m_fstream.close();
}

void VisualizerFile::Initialize(const std::string& file)
{
	m_fstream.open((file + "_vis.txt").c_str());
}

void VisualizerFile::Print(int x)
{
	m_fstream << x << endl;
}

} // end_of_namespace
} // end_of_namespace
