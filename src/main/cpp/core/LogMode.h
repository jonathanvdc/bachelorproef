#ifndef LOGMODE_H_INCLUDED
#define LOGMODE_H_INCLUDED

#include <string>

namespace stride {

/**
* Enum specifiying the level of logging required:
* \li none at all
* \li only contacts where transimission occurs
* \li all contacts.
*/
enum class LogMode
{
	None = 0U,
	Transmissions = 1U,
	Contacts = 2U,
	Null
};

/// Converts a LogMode value to corresponding name.
std::string ToString(LogMode w);

/// Check whether string is name of LogMode value.
bool IsLogMode(const std::string& s);

/// Converts a string with name to LogMode value.
LogMode ToLogMode(const std::string& s);

} // end_of_namespace

#endif // include-guard
