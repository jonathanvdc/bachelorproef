#ifndef TIMEKEEPER_STOPWATCH_H_INCLUDED
#define TIMEKEEPER_STOPWATCH_H_INCLUDED

#include <chrono>
#include <string>
#include "TimeToString.h"

namespace stride {
namespace util {

/**
 * Provides a stopwatch interface to time: it accumulates time between start/stop pairs.
 */
template <typename T = std::chrono::system_clock>
class Stopwatch
{
public:
	typedef T TClock;
	typedef typename T::duration TDuration;

	/// Constructor initializes stopwatch.
	Stopwatch(std::string name = "stopwatch", bool running = false)
	    : m_accumulated(T::duration::zero()), m_name(name), m_running(running)
	{
		if (m_running) {
			m_last_start = T::now();
		}
	}

	/// Starts stopwatch if it was stopped.
	Stopwatch& Start()
	{
		if (!m_running) {
			m_running = true;
			m_last_start = T::now();
		}
		return *this;
	}

	/// Stops the stopwatch if it was running.
	Stopwatch& Stop()
	{
		if (m_running) {
			m_accumulated += (T::now() - m_last_start);
			m_running = false;
		}
		return *this;
	}

	/// Resets stopwatch i.e. stopwatch is stopped and time accumulator is cleared.
	Stopwatch& Reset()
	{
		m_accumulated = T::duration::zero();
		m_running = false;
		return *this;
	}

	/// Reports whether stopwatch has been started.
	bool IsRunning() const { return (m_running); }

	/// Return name of this stopwatch
	std::string GetName() const { return m_name; }

	/// Returns the accumulated value without altering the stopwatch state.
	TDuration Get() const
	{
		auto fTemp = m_accumulated;
		if (m_running) {
			fTemp += (T::now() - m_last_start);
		}
		return fTemp;
	}

	/// Returns string representation of readout
	std::string ToString() const { return DurationToString(Get()); }

	/// Converts the given duration to a string. The string's formatting is defined by the clock period.
	static std::string DurationToString(TDuration duration)
	{
		typedef typename TClock::period TPeriod;
		if (std::ratio_less_equal<TPeriod, std::micro>::value) {
			return TimeToString::ToColonString(
			    std::chrono::duration_cast<std::chrono::microseconds>(duration));
		} else if (std::ratio_less_equal<TPeriod, std::milli>::value) {
			return TimeToString::ToColonString(
			    std::chrono::duration_cast<std::chrono::milliseconds>(duration));
		} else {
			return TimeToString::ToColonString(std::chrono::duration_cast<std::chrono::seconds>(duration));
		}
	}

private:
	TDuration m_accumulated;
	typename T::time_point m_last_start;
	std::string m_name;
	bool m_running;
};

/**
 * Insert accumulated time into output stream without altering stopwatch state.
 */
template <typename T>
std::ostream& operator<<(std::ostream& oss, Stopwatch<T> const& stopwatch)
{
	return (oss << stopwatch.ToString());
}

} // end namespace
} // end namespace

#endif // end of include guard
