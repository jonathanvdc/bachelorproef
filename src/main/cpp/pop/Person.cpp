#include "Age.h"
#include "Person.h"

#include "core/ClusterType.h"
#include "util/Errors.h"

#include <memory>
#include <stdexcept>
#include <string>

namespace stride {

using namespace std;

template <class BehaviourPolicy, class BeliefPolicy>
unsigned int& GenericPersonData<BehaviourPolicy, BeliefPolicy>::GetClusterId(ClusterType cluster_type)
{
	switch (cluster_type) {
	case ClusterType::Household:
		return m_household_id;
	case ClusterType::School:
		return m_school_id;
	case ClusterType::Work:
		return m_work_id;
	case ClusterType::PrimaryCommunity:
		return m_primary_community_id;
	case ClusterType::SecondaryCommunity:
		return m_secondary_community_id;
	default:
		FATAL_ERROR("Should not reach default.");
	}
}

template <class BehaviourPolicy, class BeliefPolicy>
unsigned int GenericPersonData<BehaviourPolicy, BeliefPolicy>::GetClusterId(ClusterType cluster_type) const
{
	switch (cluster_type) {
	case ClusterType::Household:
		return m_household_id;
	case ClusterType::School:
		return m_school_id;
	case ClusterType::Work:
		return m_work_id;
	case ClusterType::PrimaryCommunity:
		return m_primary_community_id;
	case ClusterType::SecondaryCommunity:
		return m_secondary_community_id;
	default:
		FATAL_ERROR("Should not reach default.");
	}
}

template <class BehaviourPolicy, class BeliefPolicy>
bool GenericPersonData<BehaviourPolicy, BeliefPolicy>::IsInCluster(ClusterType c) const
{
	switch (c) {
	case ClusterType::Household:
		return m_at_household;
	case ClusterType::School:
		return m_at_school;
	case ClusterType::Work:
		return m_at_work;
	case ClusterType::PrimaryCommunity:
		return m_at_primary_community;
	case ClusterType::SecondaryCommunity:
		return m_at_secondary_community;
	default:
		FATAL_ERROR("Should not reach default.");
	}
}

template <class BehaviourPolicy, class BeliefPolicy>
void GenericPersonData<BehaviourPolicy, BeliefPolicy>::Update(
    bool is_work_off, bool is_school_off, double fraction_infected)
{
	m_health.Update();

	// Vaccination behavior. TODO: multiple behaviors
	/* if (BehaviorPolicy::PracticesBehavior(BeliefPolicy::BelievesIn(m_belief_data))) {
		m_health.SetImmune();
	} */

	// Update presence in clusters.
	if (is_work_off || (m_age <= MinAdultAge() && is_school_off)) {
		m_at_school = false;
		m_at_work = false;
		m_at_secondary_community = false;
		m_at_primary_community = true;
	} else {
		m_at_school = true;
		m_at_work = true;
		m_at_secondary_community = true;
		m_at_primary_community = false;
	}

	BeliefPolicy::Update(m_belief_data, m_health);
}

template <class BehaviourPolicy, class BeliefPolicy>
void GenericPersonData<BehaviourPolicy, BeliefPolicy>::Update(std::shared_ptr<const GenericPersonData> p)
{
	BeliefPolicy::Update(m_belief_data, p);
}

/// Creates a copy of this person and gives it the given id.
template <class BehaviourPolicy, class BeliefPolicy>
GenericPerson<BehaviourPolicy, BeliefPolicy> GenericPerson<BehaviourPolicy, BeliefPolicy>::WithId(PersonId new_id) const
{
	auto result = Clone();
	result.m_id = new_id;
	return result;
}

//--------------------------------------------------------------------------
// All explicit instantiations.
//--------------------------------------------------------------------------
template class GenericPersonData<NoBehaviour, NoBelief>;
template class GenericPersonData<AlwaysFollowBeliefs, Threshold<true, false>>;
template class GenericPersonData<AlwaysFollowBeliefs, Threshold<false, true>>;
template class GenericPersonData<AlwaysFollowBeliefs, Threshold<true, true>>;
template class GenericPerson<NoBehaviour, NoBelief>;
template class GenericPerson<AlwaysFollowBeliefs, Threshold<true, false>>;
template class GenericPerson<AlwaysFollowBeliefs, Threshold<false, true>>;
template class GenericPerson<AlwaysFollowBeliefs, Threshold<true, true>>;

} // end_of_namespace
