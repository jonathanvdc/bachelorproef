#ifndef TIMEKEEPER_TIME_TO_STRING_H_INCLUDED
#define TIMEKEEPER_TIME_TO_STRING_H_INCLUDED

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace stride {
namespace util {

/**
 * Utilities to tag clocks and to reformat number of ticks to a string.
 */
struct TimeToString
{
	/// Procude string in hh:mm:ss format.
	static std::string ToColonString(std::chrono::seconds d)
	{
		using namespace std;
		using namespace std::chrono;

		ostringstream oss;
		hours hh = duration_cast<hours>(d);
		minutes mm = duration_cast<minutes>(d % hours(1));
		seconds ss = duration_cast<seconds>(d % minutes(1));

		oss << right << setfill('0') << setw(2) << hh.count() << ":" << setw(2) << mm.count() << ":" << setw(2)
		    << ss.count();
		return oss.str();
	}

	/// Produce string in hh:mm:ss:mus format
	static std::string ToColonString(std::chrono::milliseconds d)
	{
		using namespace std;
		using namespace std::chrono;

		ostringstream oss;
		hours hh = duration_cast<hours>(d);
		minutes mm = duration_cast<minutes>(d % hours(1));
		seconds ss = duration_cast<seconds>(d % minutes(1));
		milliseconds milli = duration_cast<milliseconds>(d % seconds(1));

		oss << right << setfill('0') << setw(2) << hh.count() << ":" << setw(2) << mm.count() << ":" << setw(2)
		    << ss.count() << ":" << setw(3) << milli.count();
		return oss.str();
	}

	/// Produce string in hh:mm:ss:ms:mus format.
	static std::string ToColonString(std::chrono::microseconds d)
	{
		using namespace std;
		using namespace std::chrono;

		ostringstream oss;
		hours hh = duration_cast<hours>(d);
		minutes mm = duration_cast<minutes>(d % hours(1));
		seconds ss = duration_cast<seconds>(d % minutes(1));
		milliseconds milli = duration_cast<milliseconds>(d % seconds(1));
		microseconds micro = duration_cast<microseconds>(d % milliseconds(1));

		oss << right << setfill('0') << setw(2) << hh.count() << ":" << setw(2) << mm.count() << ":" << setw(2)
		    << ss.count() << ":" << setw(3) << milli.count() << ":" << setw(3) << micro.count();
		return oss.str();
	}

	/// Produce string in hh:mm:ss:ms:mus:ns format.
	static std::string ToColonString(std::chrono::nanoseconds d)
	{
		using namespace std;
		using namespace std::chrono;

		ostringstream oss;
		hours hh = duration_cast<hours>(d);
		minutes mm = duration_cast<minutes>(d % hours(1));
		seconds ss = duration_cast<seconds>(d % minutes(1));
		milliseconds milli = duration_cast<milliseconds>(d % seconds(1));
		microseconds micro = duration_cast<microseconds>(d % milliseconds(1));
		nanoseconds nano = duration_cast<nanoseconds>(d % microseconds(1));

		oss << right << setfill('0') << setw(2) << hh.count() << ":" << setw(2) << mm.count() << ":" << setw(2)
		    << ss.count() << ":" << setw(3) << milli.count() << ":" << setw(3) << micro.count() << ":"
		    << setw(3) << nano.count() << endl;
		return oss.str();
	}
};

} // end namespace
} // end namespace

#endif // end of include guard
