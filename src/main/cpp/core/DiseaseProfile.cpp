/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Disease profile.
 */

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
