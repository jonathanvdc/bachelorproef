#include "DiseaseProfile.h"
#include "sim/SimulationConfig.h"

namespace stride {

using namespace std;
using namespace boost::property_tree;

void DiseaseProfile::Initialize(const SingleSimulationConfig& config, const ptree& pt_disease)
{
	// Use linear model fitted to simulation data: Expected(R0) = (b0+b1*transm_rate).
	const double r0 = config.common_config->r0;
	const double b0 = pt_disease.get<double>("disease.transmission.b0");
	const double b1 = pt_disease.get<double>("disease.transmission.b1");
	m_transmission_rate = (r0 - b0) / b1;
}

} // namespace
