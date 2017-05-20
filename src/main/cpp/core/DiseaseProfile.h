#ifndef DISEASE_PROFILE_H_INCLUDED
#define DISEASE_PROFILE_H_INCLUDED

#include <boost/property_tree/ptree.hpp>
#include "sim/SimulationConfig.h"

namespace stride {

class DiseaseProfile
{
public:
	/// Initialize.
	DiseaseProfile() : m_transmission_rate(0.0) {}

	/// Return transmission rate.
	double GetTransmissionRate() { return m_transmission_rate; }

	/// Initialize.
	void Initialize(const SingleSimulationConfig& config, const boost::property_tree::ptree& pt_disease);

private:
	double m_transmission_rate;
};

} // namespace

#endif // end-of-include-guard
