#ifndef TIMEKEEPER_TIMESTAMP_H_INCLUDED
#define TIMEKEEPER_TIMESTAMP_H_INCLUDED

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace stride {
namespace util {

/**
 * Provides wall-clock time stamp using the time call.
 * The time is that of the constructor call.
 */
class TimeStamp
{
public:
	/// Constructor marks the time for the time stamp.
	TimeStamp() : m_tp(std::chrono::system_clock::now()) {}

	/// Returns string with the time stamp after eliminating newline.
	std::string ToString() const
	{
		std::time_t t = std::chrono::system_clock::to_time_t(m_tp);
		std::string str = std::ctime(&t);
		// str[str.length() - 1] = ' ';
		return str.substr(0, str.length() - 1);
	}

	/// Returns string with the time stamp after eliminating newline.
	std::string ToTag() const
	{
		// This is the C++11 implementation but gcc (at least up to 4.9)
		// does not implement std::put_time.
		// auto now = std::chrono::system_clock::now();
		// auto in_time_t = std::chrono::system_clock::to_time_t(now);
		// std::stringstream ss;
		// ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");
		// return ss.str();

		time_t now = time(NULL);
		struct tm tstruct;
		char buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
		return buf;
	}

	/// Returns time stamp as a time_t.
	std::time_t ToTimeT() const { return std::chrono::system_clock::to_time_t(m_tp); }

private:
	std::chrono::system_clock::time_point m_tp;
};

/**
 * TimeStamp helper inserts string representation in output stream.
 */
inline std::ostream& operator<<(std::ostream& os, TimeStamp t) { return (os << t.ToString()); }

} // end namespace
} // end namespace

#endif // include guard
