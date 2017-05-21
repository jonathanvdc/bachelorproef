#ifndef RNG_HANDLER_H_INCLUDED
#define RNG_HANDLER_H_INCLUDED

#include "math.h"
#include "util/Random.h"

namespace stride {

/**
 * Processes the contacts between persons and determines whether transmission occurs.
 */
class RngHandler
{
public:
	/// Constructor sets the transmission rate and random number generator.
	RngHandler(unsigned int seed, unsigned int stream_count, unsigned int id) : m_rng(seed)
	{
		m_rng.Split(stream_count, id);
	}

	/// Convert rate into probability
	double RateToProbability(double rate) { return 1 - exp(-rate); }

	/// Check if two individuals have transmission.
	bool HasContactAndTransmission(double contact_rate, double transmission_rate)
	{
		return m_rng.NextDouble() < RateToProbability(transmission_rate * contact_rate);
	}

	/// Check if two individuals have contact.
	bool HasContact(double contact_rate) { return m_rng.NextDouble() < RateToProbability(contact_rate); }

	///
	bool HasTransmission(double transmission_rate)
	{
		return m_rng.NextDouble() < RateToProbability(transmission_rate);
	}

private:
	/// Random number engine.
	util::Random m_rng;
};

} // end_of_namespace

#endif // include guard
