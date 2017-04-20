#ifndef ALIAS_UTIL_H_INCLUDED
#define ALIAS_UTIL_H_INCLUDED

#include <exception>

/**
 * An exception to throw when an empty vector is given to the Alias constructor
 */
class EmptyProbabilityException : public std::exception
{
public:
	/// Returns the problem
	virtual const char* what() const throw() { return "Tried to do Alias without chances."; }
};

#endif
