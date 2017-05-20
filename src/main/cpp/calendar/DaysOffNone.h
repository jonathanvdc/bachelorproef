#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_NONE_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_NONE_H_

#include "DaysOffInterface.h"

namespace stride {

/**
 * No  days off work or school.
 */
class DaysOffNone : public DaysOffInterface
{
public:
	/// Initialize calendar.
	DaysOffNone(std::shared_ptr<Calendar> cal) {}

	/// See DaysOffInterface.
	bool IsWorkOff() override { return false; }

	/// See DaysOffInterface.
	bool IsSchoolOff() override { return false; }
};

} // end_of_namespace

#endif // end of include guard
