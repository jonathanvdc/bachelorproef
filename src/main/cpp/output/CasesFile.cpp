/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of the CasesFile class.
 */

#include "CasesFile.h"

#include <ctime>
#include <iostream>
#include <math.h>
#include <sstream>
#include <vector>


namespace stride {
namespace output {

using namespace std;

CasesFile::CasesFile(const std::string& file)
{
	Initialize(file);
}

CasesFile::~CasesFile()
{
	m_fstream.close();
}

void CasesFile::Initialize(const std::string& file)
{
	const string file_name = file + "_log.csv";
	m_fstream.open(file_name.c_str());
}

void CasesFile::Print(const vector<unsigned int>& cases)
{
	for(unsigned int i = 0; i < (cases.size()-1); i++) {
		m_fstream << cases[i] << ";" ;
	}
	m_fstream << cases[cases.size()-1] << endl;
}

} // end_of_namespace
} // end_of_namespace
