#ifndef CASES_FILE_H_INCLUDED
#define CASES_FILE_H_INCLUDED

#include <fstream>
#include <string>
#include <vector>

namespace stride {
namespace output {

/**
 * Produces a file with daily cases count.
 */
class CasesFile
{
public:
	/// Constructor: initialize.
	CasesFile(const std::string& file = "stride_cases");

	/// Destructor: close the file stream.
	~CasesFile();

	/// Print the given cases with corresponding tag.
	void Print(const std::vector<unsigned int>& cases);

private:
	/// Generate file name and open the file stream.
	void Initialize(const std::string& file);

private:
	std::ofstream m_fstream;
};

} // end_of_namespace
} // end_of_namespace

#endif // end of include guard
