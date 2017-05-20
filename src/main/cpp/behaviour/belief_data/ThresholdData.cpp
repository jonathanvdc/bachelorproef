/*
 * ThresholdData.cpp
 *
 *  Created on: May 8, 2017
 *      Author: elise
 */

#include "ThresholdData.h"
#include "pop/Person.h"

namespace stride {

template <typename BehaviourPolicy, typename BeliefPolicy>
void ThresholdData::Contact(std::shared_ptr<const GenericPersonData<BehaviourPolicy, BeliefPolicy>> p) {
	m_num_contacts++;
	if (p->GetHealth().IsSymptomatic()) {
		m_num_contacts_infected++;
	}
	const auto other_belief_data = p->GetBeliefData();
	if (BeliefPolicy::HasAdopted(other_belief_data)) {
		m_num_contacts_adopted++;
	}
}

template void ThresholdData::Contact<AlwaysFollowBeliefs, Threshold<true, false> >(std::shared_ptr<const GenericPersonData<AlwaysFollowBeliefs, Threshold<true, false> >> p);
template void ThresholdData::Contact<AlwaysFollowBeliefs, Threshold<false, true> >(std::shared_ptr<const GenericPersonData<AlwaysFollowBeliefs, Threshold<false, true> >> p);
template void ThresholdData::Contact<AlwaysFollowBeliefs, Threshold<true, true> >(std::shared_ptr<const GenericPersonData<AlwaysFollowBeliefs, Threshold<true, true> >> p);


} /* namespace stride */
