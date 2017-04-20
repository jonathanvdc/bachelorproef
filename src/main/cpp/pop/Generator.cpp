#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <numeric>
#include <vector>
#include <spdlog/spdlog.h>

#include "Generator.h"
#include "Population.h"

#include "alias/Alias.h"
#include "core/ClusterType.h"
#include "util/Errors.h"

namespace stride {
namespace population {

template <typename K, typename V>
V SumValues(const std::map<K, V>& map)
{
	V sum = 0;
	for (const auto& p : map) {
		sum += p.second;
	}
	return sum;
}

Population Generator::Generate()
{
	//////////////////////////////////////////////////////////////////////////
	auto console = spdlog::stderr_logger_st("popgen");
	console->set_level(spdlog::level::debug);
	console->set_pattern("\x1b[36;1m[popgen] %v\x1b[0m");
	//////////////////////////////////////////////////////////////////////////

	Population population;
	std::vector<ReferenceHousehold> reference_households;

	// A map from GeoPositions to # of people to generate there.
	std::map<geo::GeoPosition, int> population_distribution;

	// Generate cities.
	console->debug("Generating cities...");
	int n = model->population_size;
	for (const auto& city : geo_profile->GetCities()) {
		int city_size = city.relative_population * model->city_ratio * model->population_size;
		n -= city_size;
		bool inserted = population_distribution.insert({city.geo_position, city_size}).second;
		if (!inserted) {
			FATAL_ERROR("Overlapping coordinates in geoprofile city list!");
		}
	}

	// Generate towns.
	console->debug("Generating towns...");
	while (n > 0) {
		int town_size = GetRandomTownSize();
		bool inserted = population_distribution.insert({GetRandomGeoPosition(), town_size}).second;
		if (inserted) {
			n -= town_size;
		}
	}

	using GeoBRNG = alias::BiasedRandomValueGenerator<geo::GeoPosition>;
	auto geo_brng = GeoBRNG::CreateDistribution(population_distribution, random);

	// Generate households; find school/work distribution.
	console->debug("Generating households...");
	std::map<geo::GeoPosition, std::vector<ReferenceHousehold>> households;
	std::map<geo::GeoPosition, int> school_population_distribution;
	std::map<geo::GeoPosition, int> work_population_distribution;
	int num_college_students = 0;

	n = model->population_size;
	while (n > 0) {
		auto geo_position = geo_brng.Next();
		auto rh = GetRandomReferenceHousehold();
		households[geo_position].emplace_back(rh);

		for (int age : rh.ages) {
			if (model->IsSchoolAge(age)) {
				school_population_distribution[geo_position]++;
			}
			if (model->IsEmployableAge(age)) {
				work_population_distribution[geo_position]++;
			}
			if (model->IsCollegeAge(age)) {
				num_college_students++;
			}
		}

		n -= rh.ages.size();
	}

	// Generate schools.
	console->debug("Generating schools...");
	using SchoolClusterID = std::size_t;
	using School = std::vector<SchoolClusterID>;
	SchoolClusterID school_cluster_id = 1;

	auto school_geo_brng = GeoBRNG::CreateDistribution(school_population_distribution, random);
	std::map<geo::GeoPosition, std::vector<School>> schools;
	n = SumValues(school_population_distribution);
	const int clusters_per_school = model->school_size / model->school_cluster_size;

	while (n > 0) {
		auto vec = schools[school_geo_brng.Next()];
		vec.emplace_back(School());
		for (int i = 0; i < clusters_per_school; i++)
			vec.back().emplace_back(school_cluster_id++);
		n -= model->school_size;
	}

	// Generate workplaces.
	console->debug("Generating workplaces...");
	using WorkClusterID = std::size_t;
	using Workplace = WorkClusterID;
	WorkClusterID work_cluster_id = 1;

	auto work_geo_brng = GeoBRNG::CreateDistribution(work_population_distribution, random);
	std::map<geo::GeoPosition, std::vector<Workplace>> workplaces;
	n = SumValues(work_population_distribution);

	while (n > 0) {
		workplaces[work_geo_brng.Next()].emplace_back(work_cluster_id++);
		n -= model->workplace_size;
	}

	// Generate colleges.
	console->debug("Generating colleges...");
	using College = School;
	std::map<geo::GeoPosition, College> colleges;
	n = num_college_students;
	const int clusters_per_college = model->college_size / model->college_cluster_size;

	// Loop over cities in descending order of population.
	for (const auto& city : geo_profile->GetCities()) {
		if (n <= 0)
			break;

		College& college = colleges[city.geo_position] = College();
		for (int i = 0; i < clusters_per_college; i++)
			college.emplace_back(school_cluster_id++);

		n -= model->college_size;
	}

	// Generate communities.
	console->debug("Generating communities...");
	using CommunityClusterID = std::size_t;
	using Community = CommunityClusterID;
	CommunityClusterID community_cluster_id = 1;

	std::map<geo::GeoPosition, std::vector<Community>> communities;
	n = SumValues(work_population_distribution);

	while (n > 0) {
		communities[geo_brng.Next()].emplace_back(community_cluster_id++);
		n -= model->community_size;
	}

	// Generate people.
	console->debug("Generating people...");
	using HouseholdClusterID = std::size_t;
	HouseholdClusterID household_id = 1;
	std::size_t person_id = 1;

	for (const auto& p : households) {
		const auto& geo_position = p.first;
		for (const auto& rh : p.second) {
			for (const int age : rh.ages) {
				int school_id = 0;
				int work_id = 0;
				int primary_community_id = 0;
				int secondary_community_id = 0;

				if (model->IsSchoolAge(age)) {
					school_id = FindLocal(geo_position, schools);
				} else if (model->IsCollegeAge(age)) {
				} else if (model->IsEmployableAge(age) && random.Chance(model->employed_ratio)) {
				}

				population.emplace(
				    person_id++, age, household_id, school_id, work_id, primary_community_id,
				    secondary_community_id, disease.Sample(random));

				if (person_id % 1000 == 0) {
					console->debug("Person {}...", person_id);
				}
			}
			household_id++;
		}
	}

	return population;
}

bool Generator::FitsModel(const Population& population, bool verbose) { return true; }

/*

bool Generator::FitsModel(const Population& population, bool verbose)
{
	auto console = spdlog::stderr_logger_st("popgen");
	console->set_level(verbose ? spdlog::level::debug : spdlog::level::off);
	console->set_pattern("\x1b[36;1m[popgen] %v\x1b[0m");
	console->debug("Testing if generated population fits model...");

	console->debug("Generated {} people with invalid ages.", age_errors);
}

*/

} // namespace population
} // namespace stride
