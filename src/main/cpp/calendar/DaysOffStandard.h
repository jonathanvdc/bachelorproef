#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_STANDARD_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_STANDARD_H_

#include "Calendar.h"
#include "DaysOffInterface.h"

#include <memory>

namespace stride {

/**
 * Standard situation for days off from work and school.
 */
class DaysOffStandard : public DaysOffInterface
{
public:
	/// Initialize calendar.
	DaysOffStandard(std::shared_ptr<Calendar> cal) : m_calendar(cal) {}

	/// See DaysOffInterface.
	bool IsWorkOff() override { return m_calendar->IsWeekend() || m_calendar->IsHoliday(); }

	/// See DaysOffInterface.
	virtual bool IsSchoolOff() override
	{
		return m_calendar->IsWeekend() || m_calendar->IsHoliday() || m_calendar->IsSchoolHoliday();
	}

private:
	std::shared_ptr<Calendar> m_calendar;
};

} // end_of_namespace

#endif // end of include guard
