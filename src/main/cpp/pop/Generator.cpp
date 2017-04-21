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
#include "core/Cluster.h"
#include "core/ClusterType.h"
#include "util/Errors.h"

namespace stride {
namespace population {

/// Calculate the sum of all the values of a map.
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
	// Types used in this method.
	using stride::geo::GeoPosition;
	using GeoBRNG = alias::BiasedRandomValueGenerator<GeoPosition>;
	using SchoolClusterID = ClusterID;
	using WorkClusterID = ClusterID;
	using CommunityClusterID = ClusterID;
	using HouseholdClusterID = ClusterID;
	using School = std::vector<SchoolClusterID>;
	using College = School;

	// Logging.
	auto console = spdlog::stderr_logger_st("popgen");
	console->set_level(spdlog::level::debug);
	console->set_pattern("\x1b[36;1m[popgen] %v\x1b[0m");

	// The population object we're filling up.
	Population population;

	// Throughout this method, `n` is a counter for how many people are
	// left to accomodate for. When towns, schools, etc. are added to the
	// simulation area, the number of people they can serve is subtracted
	// from `n`, and facilities keep being added until `n <= 0`.
	int n;

	// Generated cluster data.
	std::map<GeoPosition, std::vector<ReferenceHousehold>> households;
	std::map<GeoPosition, std::vector<School>> schools;
	std::map<GeoPosition, College> colleges;
	std::map<GeoPosition, std::vector<WorkClusterID>> workplaces;
	std::map<GeoPosition, std::vector<CommunityClusterID>> primary_communities;
	std::map<GeoPosition, std::vector<CommunityClusterID>> secondary_communities;

	// A map from GeoPositions to # of people to generate there.
	std::map<GeoPosition, int> population_distribution;

	// Generate cities.
	{
		console->debug("Generating cities...");
		int cities_created = 0;
		n = model->population_size;
		for (const auto& city : geo_profile->GetCities()) {
			int city_size = city.relative_population * model->city_ratio * model->population_size;
			n -= city_size;
			bool inserted = population_distribution.insert({city.geo_position, city_size}).second;
			if (!inserted) {
				FATAL_ERROR("Overlapping coordinates in geoprofile city list!");
			}
			cities_created++;
		}
		console->debug("Created {} cities.", cities_created);
	}

	// Generate towns.
	{
		console->debug("Generating towns...");
		int towns_created = 0;
		while (n > 0) {
			int town_size = GetRandomTownSize();
			bool inserted = population_distribution.insert({GetRandomGeoPosition(), town_size}).second;
			if (inserted) {
				n -= town_size;
			}
			towns_created++;
		}
		console->debug("Created {} towns.", towns_created);
	}

	// A GeoPosition distribution, biased towards locations with high populations.
	auto geo_brng = GeoBRNG::CreateDistribution(population_distribution, random);

	// Generate households; find school/work distribution.
	int num_college_students = 0;
	std::map<GeoPosition, int> school_population_distribution;
	std::map<GeoPosition, int> work_population_distribution;
	{
		console->debug("Generating households...");
		int households_created = 0;
		n = model->population_size;
		while (n > 0) {
			auto geo_position = geo_brng.Next();
			auto rh = GetRandomReferenceHousehold();
			households[geo_position].emplace_back(rh);

			for (int age : rh.ages) {
				if (model->IsSchoolAge(age))
					school_population_distribution[geo_position]++;
				if (model->IsEmployableAge(age))
					work_population_distribution[geo_position]++;
				if (model->IsCollegeAge(age))
					num_college_students++;
			}

			n -= rh.ages.size();
			households_created++;
		}
		console->debug("Created {} households.", households_created);
	}

	// Generate schools.
	std::map<GeoPosition, double> college_distribution;
	{
		console->debug("Generating schools...");
		SchoolClusterID school_cluster_id = 1;
		int schools_created = 0;

		auto school_geo_brng = GeoBRNG::CreateDistribution(school_population_distribution, random);
		const int clusters_per_school = model->school_size / model->school_cluster_size;

		n = SumValues(school_population_distribution);
		while (n > 0) {
			auto& vec = schools[school_geo_brng.Next()];
			vec.emplace_back(School());
			for (int i = 0; i < clusters_per_school; i++)
				vec.back().emplace_back(school_cluster_id++);
			n -= model->school_size;
			schools_created++;
		}
		console->debug("Created {} schools.", schools_created);

		// Generate colleges.
		console->debug("Generating colleges...");
		const int clusters_per_college = model->college_size / model->college_cluster_size;
		int colleges_created = 0;

		// Loop over cities in descending order of population.
		n = num_college_students;
		for (const auto& city : geo_profile->GetCities()) {
			if (n <= 0)
				break;

			College& college = colleges[city.geo_position];
			for (int i = 0; i < clusters_per_college; i++)
				college.emplace_back(school_cluster_id++);

			college_distribution[city.geo_position] = city.relative_population;

			n -= model->college_size;
			colleges_created++;
		}
		console->debug("Created {} colleges.", colleges_created);
	}

	// This biased distribution is sampled from for students who commute to college.
	auto college_brng = GeoBRNG::CreateDistribution(college_distribution, random);

	// This biased distribution is sampled from for employees who commute to work.
	auto work_geo_brng = GeoBRNG::CreateDistribution(work_population_distribution, random);

	// Generate workplaces.
	{
		console->debug("Generating workplaces...");
		WorkClusterID work_cluster_id = 1;
		int workplaces_created = 0;

		n = SumValues(work_population_distribution);
		while (n > 0) {
			workplaces[work_geo_brng.Next()].emplace_back(work_cluster_id++);
			n -= model->workplace_size;
			workplaces_created++;
		}
		console->debug("Created {} workplaces.", workplaces_created);
	}

	// Generate communities.
	{
		console->debug("Generating primary communities...");
		CommunityClusterID primary_community_cluster_id = 1;
		int primary_communities_created = 0;

		n = SumValues(population_distribution);
		while (n > 0) {
			primary_communities[geo_brng.Next()].emplace_back(primary_community_cluster_id++);
			n -= model->community_size;
			primary_communities_created++;
		}

		console->debug("Created {} primary communities.", primary_communities_created);
	}

	// Generate secondary communities.
	{
		console->debug("Generating secondary communities...");
		CommunityClusterID secondary_community_cluster_id = 1;
		int secondary_communities_created = 0;

		n = SumValues(population_distribution);
		while (n > 0) {
			secondary_communities[geo_brng.Next()].emplace_back(secondary_community_cluster_id++);
			n -= model->community_size;
			secondary_communities_created++;
		}

		console->debug("Created {} secondary communities.", secondary_communities_created);
	}

	// Generate people.
	{
		console->debug("Generating people...");
		HouseholdClusterID household_id = 1;
		ClusterID person_id = 1;

		for (const auto& p : households) {
			const auto& home = p.first;
			for (const auto& rh : p.second) {
				for (const int age : rh.ages) {
					int school_id = 0;
					int work_id = 0;
					if (model->IsSchoolAge(age)) {
						school_id = random.Sample(random.Sample(FindLocal(home, schools)));
					} else if (model->IsCollegeAge(age)) {
						bool commutes = random.Chance(model->college_commute_ratio);
						school_id = random.Sample(
						    commutes ? colleges[college_brng.Next()]
							     : FindLocal(home, colleges));
					} else if (
					    model->IsEmployableAge(age) && random.Chance(model->employed_ratio)) {
						// TODO: technically, commuters should commute to workplaces in big
						// cities, not
						// just random ones.
						bool commutes = random.Chance(model->work_commute_ratio);
						work_id = random.Sample(
						    FindLocal(commutes ? work_geo_brng.Next() : home, workplaces));
					}

					int primary_community_id = random.Sample(FindLocal(home, primary_communities));
					int secondary_community_id =
					    random.Sample(FindLocal(home, secondary_communities));

					population.emplace(
					    person_id++, age, household_id, school_id, work_id, primary_community_id,
					    secondary_community_id, disease.Sample(random));

					if (person_id % 10000 == 0) {
						console->debug("Person {}...", person_id);
					}
				}
				household_id++;
			}
		}
		console->debug("Generated {} people.", person_id - 1);
	}

	// Store all the cluster's locations in the population's atlas.
	{
		console->debug("Creating atlas of cluster locations...");
		HouseholdClusterID household_id = 1;
		for (const auto& p : households)
			for (auto it = p.second.begin(); it != p.second.end(); ++it)
				population.AtlasEmplace({household_id++, ClusterType::Household}, p.first);

		for (const auto& p : schools)
			for (const auto& school : p.second)
				for (SchoolClusterID id : school)
					population.AtlasEmplace({id, ClusterType::School}, p.first);

		for (const auto& p : colleges)
			for (SchoolClusterID id : p.second)
				population.AtlasEmplace({id, ClusterType::School}, p.first);

		for (const auto& p : workplaces)
			for (WorkClusterID id : p.second)
				population.AtlasEmplace({id, ClusterType::Work}, p.first);

		for (const auto& p : primary_communities)
			for (CommunityClusterID id : p.second)
				population.AtlasEmplace({id, ClusterType::PrimaryCommunity}, p.first);

		for (const auto& p : secondary_communities)
			for (CommunityClusterID id : p.second)
				population.AtlasEmplace({id, ClusterType::SecondaryCommunity}, p.first);
	}

	console->debug("Done!");
	return population;
}

bool Generator::FitsModel(const Population& population, bool verbose) { return true; }

} // namespace population
} // namespace stride
