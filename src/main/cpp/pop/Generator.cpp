#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <numeric>
#include <vector>
#include <spdlog/spdlog.h>
#include "Population.h"
#include "Generator.h"
#include "core/ClusterType.h"

namespace stride {
namespace population {

Population Generator::Generate()
{
	Population population;

	return population;
}

bool Generator::FitsModel(const Population& population, bool verbose) { return true; }

/*

// Return a random sorted range of `count` values inside `range`, where all the
// absolute differences between values are inside `gap`.
std::vector<int> Generator::SampleApart(InclusiveRange<int> range, InclusiveRange<int> gap, std::size_t count)
{
	// TODO: make this function smart.

	assert(count > 0);
	std::vector<int> v(count);
	for (int tries = 0; tries < 100; tries++) {
		std::generate_n(v.begin(), count, [&]() { return random(range); });
		std::sort(v.begin(), v.end());

		if (v.back() - v.front() > gap.maximum) {
			// Maximum gap is too large.
			continue;
		}

		for (std::size_t i = 0; i < count - 1; i++) {
			if (v[i + 1] - v[i] < gap.minimum) {
				// Minimum gap is too small.
				goto sample_next_try;
			}
		}

		return v;
	sample_next_try:;
	}

	throw std::runtime_error("FATAL: SampleApart couldn't satisfy gap constraint.");
}

void Generator::GenerateHousehold(Population& population, int size)
{
	if (size > 2) {
		// Add (size - 2) children.
		const auto child_ages = SampleApart(InclusiveRange<int>(1, model.household.age.child_maximum),
						    model.household.age_gap.child, size - 2);
		for (int age : child_ages) {
			GeneratePerson(population, age);
		}

		// The eldest child's age might push the minimum parent age up, to enforce the child-parent gap:
		const int new_parent_minimum = child_ages.back() + model.household.age_gap.child_parent_minimum;
		auto range = model.household.age.parent;
		range.minimum = std::max(range.minimum, new_parent_minimum);

		// Add 2 parents.
		const auto parent_ages = SampleApart(range, model.household.age_gap.parent, 2);
		for (int age : parent_ages) {
			GeneratePerson(population, age);
		}
	} else {
		// TODO: ...well, oops, this approach makes it very hard to enforce the age distribution:
		//
		//     ------------.
		//     |           |'.
		//     |           |  '.
		//     |           |    '.
		//     |           |      '.
		//     0         elbow    maximum
		//
		// ...but I don't think it's *impossible*. At this point (h <= 2) we can "cheat",
		// analyze the distribution of all the people we've already generated, and start padding
		// with ages that make the total distribution approximate the above. Sounds hard, but
		// generating people first and *then* grouping them into households sounds impossible.

		const auto ages =
		    SampleApart(InclusiveRange<int>(model.household.age.live_alone_minimum, model.age.maximum),
				model.household.age_gap.parent, size);
		for (int age : ages) {
			GeneratePerson(population, age);
		}
	}
	household_id++;
}

unsigned int Generator::SchoolID(int age)
{
	// There are 4 types of school: kindergarten, primary, secondary, and college/uni.
	// Let's respectively reserve school IDs equal to 0, 1, 2, 3 modulo 4 to these types.

	// First, figure out if we're going to school at all.
	if (age < model.school.age.kindergarten || age > model.school.age.graduation ||
	    (age >= model.school.age.higher_education && random.NextDouble() > model.school.p_higher_education)) {
		return 0;
	}

	// If we are, figure out the school type.
	int school_type = 0;
	if (age >= model.school.age.higher_education) {
		school_type = 3;
	} else if (age >= model.school.age.secondary_school) {
		school_type = 2;
	} else if (age >= model.school.age.primary_school) {
		school_type = 1;
	}

	return (random(num_schools) / 4) * 4 + school_type + 1;
}

unsigned int Generator::WorkID(int age)
{
	if (model.work.age.Includes(age) && random.NextDouble() < model.work.p_employed) {
		return random(num_works) + 1;
	}

	return 0;
}

unsigned int Generator::CommunityID() { return random(num_communities) + 1; }

void Generator::GeneratePerson(Population& population, int age)
{
	population.emplace(people_generated++, age, household_id, SchoolID(age), WorkID(age), CommunityID(),
				       CommunityID(), disease.Sample(random));
}

bool Generator::FitsModel(const Population& population, bool verbose)
{
	auto console = spdlog::stderr_logger_st("popgen");
	console->set_level(verbose ? spdlog::level::debug : spdlog::level::off);
	console->set_pattern("\x1b[36;1m[popgen] %v\x1b[0m");
	console->debug("Testing if generated population fits model...");

	console->debug("=== Age requirements:");
	int age_errors = 0;
	std::map<int, int> age_histogram;
	for (const auto& p : population) {
		int age = static_cast<int>(p.GetAge());
		if (age < 0 || age > model.age.maximum) {
			age_errors++;
		} else {
			age_histogram[age]++;
		}
	}
	console->debug("Generated {} people with invalid ages.", age_errors);

	// console->debug("=== Age distribution:");
	// for (int age = 0; age <= model.age.maximum; age++) {
	//     console->debug("{:2}: {}", age, std::string(age_histogram[age] / 10, '#'));
	// }

	console->debug("=== Household distribution:");
	std::map<int, std::vector<const Person*>> households;
	for (const auto& p : population) {
		int hid = p.GetClusterId(stride::ClusterType::Household);
		households[hid].emplace_back(&p);
	}

	std::map<unsigned int, int> household_size_histogram;
	for (const auto& h : households) {
		household_size_histogram[h.second.size()]++;
	}

	for (unsigned int k = 1; household_size_histogram.count(k); k++) {
		console->debug("{} households of size {}", household_size_histogram[k], k);
	}

	// We can do more model testing here later.
	return true;
}

*/

} // namespace population
} // namespace stride
