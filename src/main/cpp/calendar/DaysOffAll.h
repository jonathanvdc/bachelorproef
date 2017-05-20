#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_ALL_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_ALL_H_

#include "DaysOffInterface.h"

namespace stride {

/**
 * All days off work or school.
 */
class DaysOffAll : public DaysOffInterface
{
public:
	/// Initialize calendar.
	DaysOffAll(std::shared_ptr<Calendar> cal) {}

	/// See DaysOffInterface.
	bool IsWorkOff() override { return true; }

	/// See DaysOffInterface.
	bool IsSchoolOff() override { return true; }
};

} // end_of_namespace

#endif // end of include guard
