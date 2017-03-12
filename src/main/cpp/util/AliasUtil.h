/*
 * AliasUtil.h
 *
 *  Created on: Mar 12, 2017
 *      Author: cedric
 */

#ifndef ALIASUTIL_H_
#define ALIASUTIL_H_

#include <exception>

/**
 * An exception to throw when an empty vector is given to the Alias constructor
 */

namespace stride{
namespace util{

class EmptyProbabilityException: public std::exception {
	public:
		///Returns the problem
		virtual const char* what() const throw(){return "Tried to do Alias without chances.";}
};

}
}

#endif /* ALIASUTIL_H_ */
