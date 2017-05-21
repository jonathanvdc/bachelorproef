#ifndef SRC_MAIN_CALENDAR_DAYS_OFF_INTERFACE_H_
#define SRC_MAIN_CALENDAR_DAYS_OFF_INTERFACE_H_

namespace stride {

/**
 * Days-off interface definition.
 */
class DaysOffInterface
{
public:
	/// Whether today is a work day.
	virtual bool IsWorkOff() = 0;

	/// Whether today is school day.
	virtual bool IsSchoolOff() = 0;

	/// Virtual destructor.
	virtual ~DaysOffInterface() {}
};

} // end_of_namespace

#endif // end of include guard
