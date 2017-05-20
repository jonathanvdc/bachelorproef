#include "calendar/Calendar.h"
#include "core/Cluster.h"
#include "core/Health.h"
#include "core/Infector.h"
#include "core/LogMode.h"
#include "pop/Person.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <spdlog/spdlog.h>
#include "RngHandler.h"

namespace stride {

using namespace std;

/**
 * Primary R0_POLICY: do nothing i.e. track all cases.
 */
template <bool track_index_case = false>
class R0_POLICY
{
public:
	static void Execute(const Person& p) {}
};

/**
 * Specialized R0_POLICY: track only the index case.
 */
template <>
class R0_POLICY<true>
{
public:
	static void Execute(const Person& p) { p.GetHealth().StopInfection(); }
};

/**
 * Primary LOG_POLICY policy, implements LogMode::None.
 */
template <LogMode log_level = LogMode::None>
class LOG_POLICY
{
public:
	static void Execute(
	    const shared_ptr<spdlog::logger>& logger, const Person& p1, const Person& p2, ClusterType cluster_type,
	    const CalendarRef& environ)
	{
	}
};

/**
 * Specialized LOG_POLICY policy LogMode::Transmissions.
 */
template <>
class LOG_POLICY<LogMode::Transmissions>
{
public:
	static void Execute(
	    const shared_ptr<spdlog::logger>& logger, const Person& p1, const Person& p2, ClusterType cluster_type,
	    const CalendarRef& environ)
	{
		logger->info(
		    "[TRAN] {} {} {} {}", p1.GetId(), p2.GetId(), ToString(cluster_type), environ->GetSimulationDay());
	}
};

/**
 * Specialized LOG_POLICY policy LogMode::Contacts.
 */
template <>
class LOG_POLICY<LogMode::Contacts>
{
public:
	static void Execute(
	    const shared_ptr<spdlog::logger>& logger, const Person& p1, const Person& p2, ClusterType cluster_type,
	    const CalendarRef& calendar)
	{
		unsigned int home = (cluster_type == ClusterType::Household);
		unsigned int work = (cluster_type == ClusterType::Work);
		unsigned int school = (cluster_type == ClusterType::School);
		unsigned int primary_community = (cluster_type == ClusterType::PrimaryCommunity);
		unsigned int secundary_community = (cluster_type == ClusterType::SecondaryCommunity);

		logger->info(
		    "[CONT] {} {} {} {} {} {} {} {} {}", p1.GetId(), p1.GetAge(), p2.GetAge(), home, school, work,
		    primary_community, secundary_community, calendar->GetSimulationDay());
	}
};

//--------------------------------------------------------------------------
// Definition for primary template covers the situation for
// LogMode::None & LogMode::Transmissions, both with
// track_index_case false and true.
// And every local information policy except NoLocalInformation
//--------------------------------------------------------------------------
template <LogMode log_level, bool track_index_case, typename local_information_policy>
void Infector<log_level, track_index_case, local_information_policy>::Execute(
    Cluster& cluster, DiseaseProfile disease_profile, RngHandler& contact_handler, const CalendarRef& calendar,
    const std::shared_ptr<spdlog::logger>& logger)
{
	cluster.UpdateMemberPresence();

	// set up some stuff
	const auto c_type = cluster.m_cluster_type;
	const auto& c_members = cluster.m_members;
	const auto transmission_rate = disease_profile.GetTransmissionRate();

	// check all contacts
	for (size_t i_person1 = 0; i_person1 < c_members.size(); i_person1++) {
		// check if member is present today
		if (c_members[i_person1].second) {
			auto p1 = c_members[i_person1].first;
			const double contact_rate = cluster.GetContactRate(p1);

			// loop over possible contacts
			// FIXME should this loop start from 0? Because of asymm. contact rates
			for (size_t i_person2 = i_person1 + 1; i_person2 < c_members.size(); i_person2++) {
				// check if member is present today
				if (c_members[i_person2].second) {
					auto p2 = c_members[i_person2].first;

					// check for contact
					if (contact_handler.HasContact(contact_rate)) {
						// exchange information about health state & beliefs
						p1.Update(p2.GetData());
						p2.Update(p1.GetData());

						bool transmission = contact_handler.HasTransmission(transmission_rate);
						if (transmission) {
							if (p1.GetHealth().IsInfectious() &&
							    p2.GetHealth().IsSusceptible()) {
								LOG_POLICY<log_level>::Execute(
								    logger, p1, p2, c_type, calendar);
								p2.GetHealth().StartInfection();
								R0_POLICY<track_index_case>::Execute(p2);
							} else if (
							    p2.GetHealth().IsInfectious() &&
							    p1.GetHealth().IsSusceptible()) {
								LOG_POLICY<log_level>::Execute(
								    logger, p2, p1, c_type, calendar);
								p1.GetHealth().StartInfection();
								R0_POLICY<track_index_case>::Execute(p1);
							}
						}
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------
// Definition of partial specialization for LocalInformationPolicy:NoLocalInformation.
//-------------------------------------------------------------------------------------------
template <LogMode log_level, bool track_index_case>
void Infector<log_level, track_index_case, NoLocalInformation>::Execute(
    Cluster& cluster, DiseaseProfile disease_profile, RngHandler& contact_handler, const CalendarRef& calendar,
    const std::shared_ptr<spdlog::logger>& logger)
{
	// check if the cluster has infected members and sort
	bool infectious_cases;
	std::size_t num_cases;
	tie(infectious_cases, num_cases) = cluster.SortMembers();

	if (infectious_cases) {
		cluster.UpdateMemberPresence();

		// set up some stuff
		const auto c_type = cluster.m_cluster_type;
		const auto c_immune = cluster.m_index_immune;
		const auto& c_members = cluster.m_members;
		const auto transmission_rate = disease_profile.GetTransmissionRate();

		// match infectious in first part with susceptible in second part, skip last part (immune)
		for (size_t i_infected = 0; i_infected < num_cases; i_infected++) {
			// check if member is present today
			if (c_members[i_infected].second) {
				const auto p1 = c_members[i_infected].first;
				// FIXME Is it necessary to check for infectiousness here? Infectious members are
				// already sorted...
				if (p1.GetHealth().IsInfectious()) {
					const double contact_rate = cluster.GetContactRate(p1);
					// FIXME if loop 2 in all contacts algorithm should start from 0, we should also
					// implement this symmetry here!
					for (size_t i_contact = num_cases; i_contact < c_immune; i_contact++) {
						// check if member is present today
						if (c_members[i_contact].second) {
							auto p2 = c_members[i_contact].first;
							if (contact_handler.HasContactAndTransmission(
								contact_rate, transmission_rate)) {
								LOG_POLICY<log_level>::Execute(
								    logger, p1, p2, c_type, calendar);
								p2.GetHealth().StartInfection();
								R0_POLICY<track_index_case>::Execute(p2);
							}
						}
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------
// Definition of partial specialization for LogMode::Contacts and NoLocalInformation policy.
//-------------------------------------------------------------------------------------------
template <bool track_index_case>
void Infector<LogMode::Contacts, track_index_case, NoLocalInformation>::Execute(
    Cluster& cluster, DiseaseProfile disease_profile, RngHandler& contact_handler, const CalendarRef& calendar,
    const std::shared_ptr<spdlog::logger>& logger)
{
	cluster.UpdateMemberPresence();

	// set up some stuff
	const auto c_type = cluster.m_cluster_type;
	const auto& c_members = cluster.m_members;
	const auto transmission_rate = disease_profile.GetTransmissionRate();

	// check all contacts
	for (size_t i_person1 = 0; i_person1 < c_members.size(); i_person1++) {
		// check if member participates in the social contact survey && member is present today
		if (c_members[i_person1].second && c_members[i_person1].first.IsParticipatingInSurvey()) {
			auto p1 = c_members[i_person1].first;
			const double contact_rate = cluster.GetContactRate(p1);
			// loop over possible contacts
			for (size_t i_person2 = i_person1 + 1; i_person2 < c_members.size(); i_person2++) {
				// check if member is present today
				if (c_members[i_person2].second) {
					auto p2 = c_members[i_person2].first;
					// check for contact
					if (contact_handler.HasContact(contact_rate)) {
						bool transmission = contact_handler.HasTransmission(transmission_rate);

						if (transmission) {
							if (p1.GetHealth().IsInfectious() &&
							    p2.GetHealth().IsSusceptible()) {
								p2.GetHealth().StartInfection();
								R0_POLICY<track_index_case>::Execute(p2);
							} else if (
							    p2.GetHealth().IsInfectious() &&
							    p1.GetHealth().IsSusceptible()) {
								p1.GetHealth().StartInfection();
								R0_POLICY<track_index_case>::Execute(p1);
							}
						}

						LOG_POLICY<LogMode::Contacts>::Execute(
						    logger, p1, p2, c_type, calendar);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
// All explicit instantiations.
//--------------------------------------------------------------------------
template class Infector<LogMode::None, false, NoLocalInformation>;

template class Infector<LogMode::None, true, NoLocalInformation>;

template class Infector<LogMode::Transmissions, false, NoLocalInformation>;

template class Infector<LogMode::Transmissions, true, NoLocalInformation>;

template class Infector<LogMode::Contacts, false, NoLocalInformation>;

template class Infector<LogMode::Contacts, true, NoLocalInformation>;

} // end_of_namespace
