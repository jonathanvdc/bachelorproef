#ifndef PERSON_FILE_H_INCLUDED
#define PERSON_FILE_H_INCLUDED

#include "pop/Population.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace stride {
namespace output {

/**
 * Produces a file with daily cases count.
 */
class PersonFile
{
public:
	/// Constructor: initialize.
	PersonFile(const std::string& file = "stride_person");

	/// Destructor: close the file stream.
	~PersonFile();

	/// Print the given cases with corresponding tag.
	void Print(const PopulationRef& population);

private:
	/// Generate file name and open the file stream.
	void Initialize(const std::string& file);

private:
	std::ofstream m_fstream;
};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
