/*
 * AliasUtil.h
 *
 *  Created on: Mar 12, 2017
 *      Author: cedric
 */
#include <exception>

#ifndef ALIASUTIL_H_INCLUDED
#define ALIASUTIL_H_INCLUDED

/**
 * An exception to throw when an empty vector is given to the Alias constructor
 */
class EmptyProbabilityException : public std::exception
{
    public:
	/// Returns the problem
	virtual const char* what() const throw()
	{
		return "Tried to do Alias without chances.";
	}
};

#endif /* ALIASUTIL_H_ */
