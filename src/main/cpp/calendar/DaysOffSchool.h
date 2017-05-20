#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_SCHOOL_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_ALL_H_

#include "DaysOffInterface.h"

namespace stride {

/**
 * Schools closed.
 */
class DaysOffSchool : public DaysOffInterface
{
public:
	/// Initialize calendar.
	DaysOffSchool(std::shared_ptr<Calendar> cal) : m_calendar(cal) {}

	/// See DaysOffInterface.
	bool IsWorkOff() override { return m_calendar->IsWeekend() || m_calendar->IsHoliday(); }

	/// See DaysOffInterface.
	virtual bool IsSchoolOff() override { return true; }

private:
	std::shared_ptr<Calendar> m_calendar;
};

} // end_of_namespace

#endif // end of include guard
