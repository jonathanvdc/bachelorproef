#include "CasesFile.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace stride {
namespace output {

using namespace std;

CasesFile::CasesFile(const std::string& file) { Initialize(file); }

CasesFile::~CasesFile() { m_fstream.close(); }

void CasesFile::Initialize(const std::string& file) { m_fstream.open((file + "_cases.csv").c_str()); }

void CasesFile::Print(const vector<unsigned int>& cases)
{
	for (unsigned int i = 0; i < (cases.size() - 1); i++) {
		m_fstream << cases[i] << ",";
	}
	m_fstream << cases[cases.size() - 1] << endl;
}

} // end_of_namespace
} // end_of_namespace
