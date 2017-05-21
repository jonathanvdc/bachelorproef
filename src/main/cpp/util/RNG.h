/*
 * RNG.h
 *
 *  Created on: Apr 20, 2017
 *      Author: elise
 */

#ifndef SRC_MAIN_CPP_UTIL_RNG_H_
#define SRC_MAIN_CPP_UTIL_RNG_H_

#include <trng/mrg2.hpp>
#include <trng/uniform01_dist.hpp>

namespace stride {

class RNG
{
private:
	RNG() { m_uniform_dist = trng::uniform01_dist<double>(); }
	RNG(const RNG&);
	RNG& operator=(const RNG&);

public:
	static RNG& GetInstance()
	{
		static RNG instance;
		return instance;
	}

	double NextDouble() { return m_uniform_dist(m_engine); }

private:
	/// The random number engine.
	trng::mrg2 m_engine;

	/// The random distribution.
	trng::uniform01_dist<double> m_uniform_dist;
};

} /* namespace stride */

#endif /* SRC_MAIN_CPP_UTIL_RNG_H_ */
