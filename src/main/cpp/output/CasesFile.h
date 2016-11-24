#ifndef CASES_FILE_H_INCLUDED
#define CASES_FILE_H_INCLUDED
/*
 *  This file is part of the indismo software.
 *  It is free software: you can redistribute it and/or modify it
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
 *  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and
 *  Broeckhove J. (2015) Optimizing agent-based transmission models for
 *  infectious diseases, BMC Bioinformatics.
 *
 *  Copyright 2015, Willem L, Stijven S & Broeckhove J
 */
/**
 * @file
 * Header for the CasesFile class.
 */

#include <fstream>
#include <string>
#include <vector>

namespace indismo {
namespace output {

/**
 * Produces a file with daily cases count.
 */
class CasesFile
{
public:
	/// Constructor: initialize.
	CasesFile(const std::string& file = "indismo_cases");

	/// Destructor: close the file stream.
	~CasesFile();

	/// Print the given cases with corresponding tag.
	void Print(const std::vector<unsigned int>& cases);

private:
	/// Generate file name and open the file stream.
	void Initialize(const std::string& file = "indismo_cases");

private:
	std::ofstream 	m_fstream;  ///< The file stream.
};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
