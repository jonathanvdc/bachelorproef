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
