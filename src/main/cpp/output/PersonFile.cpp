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

#include "PersonFile.h"

#include "core/Health.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace stride {
namespace output {

using namespace std;

PersonFile::PersonFile(const std::string& file) { Initialize(file); }

PersonFile::~PersonFile() { m_fstream.close(); }

void PersonFile::Initialize(const std::string& file)
{
	m_fstream.open((file + "_person.csv").c_str());

	// add header
	m_fstream << "id,is_recovered,is_immune,start_infectiousness;"
		  << "end_infectiousness,start_symptomatic,end_symptomatic" << endl;
}

void PersonFile::Print(const PopulationRef& population)
{
	population->serial_for([this](const Person& p, unsigned int) {
		const auto& h = p.GetHealth();
		if (!h.IsSusceptible()) {
			m_fstream << p.GetId() << "," << h.IsRecovered() << "," << h.IsImmune() << ","
				  << h.GetStartInfectiousness() << "," << h.GetEndInfectiousness() << ","
				  << h.GetStartSymptomatic() << "," << h.GetEndSymptomatic() << endl;
		}
	});
}

} // end_of_namespace
} // end_of_namespace
