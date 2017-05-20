#ifndef NOBELIEF_H_INCLUDED
#define NOBELIEF_H_INCLUDED

#include "behaviour/belief_data/Nothing.h"

namespace stride {

template <typename BehaviourPolicy, typename BeliefPolicy>
class GenericPersonData;

class NoBelief
{
public:
	using Data = Nothing;

	static void Initialize(Data& belief_data, double risk_averseness) {}

	static void Update(Data& belief_data, Health& health_data) {}

	template <typename BehaviourPolicy>
	static void Update(Data& belief_data, std::shared_ptr<const GenericPersonData<BehaviourPolicy, NoBelief>> p)
	{
	}

	static bool HasAdopted(const Data& belief_data) { return false; }
};

} /* namespace stride */

#endif // include-guard
