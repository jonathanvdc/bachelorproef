#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "core/Disease.h"
#include "core/LogMode.h"
#include "multiregion/TravelModel.h"
#include "pop/Generator.h"
#include "sim/SimulatorBuilder.h"
#include "util/InstallDirs.h"

namespace Tests {

using namespace boost::property_tree;
using namespace stride;
using namespace stride::util;

TEST(PopulationGeneration, GeneratedPopulationFitsModel)
{
	ptree pt_config;
	InstallDirs::ReadXmlFile("../config/run_test_popgen.xml", InstallDirs::GetCurrentDir(), pt_config);
	stride::SingleSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	ptree pt_disease;
	InstallDirs::ReadXmlFile(config.common_config->disease_config_file_name, InstallDirs::GetDataDir(), pt_disease);
	const auto disease = disease::Disease::Parse(pt_disease);
	stride::util::Random rng(1);

	auto generator = population::Generator::FromConfig(config, *disease, rng);
	generator->Verbose(true);
	auto population = generator->Generate();
	ASSERT_TRUE(generator->FitsModel(population));
}

TEST(PopulationGeneration, GeneratedPopulationIsInfectious)
{
	auto log = spdlog::stderr_logger_st("test_popgen");
	log->set_level(spdlog::level::off);
	auto sim = stride::SimulatorBuilder::Build("../config/run_test_popgen.xml", log, 1, false);

	// Run the simulation for 10 days, and assert an increase in infected persons.
	unsigned int before = sim->GetPopulation()->get_infected_count();
	for (int i = 0; i < 10; i++)
		(void)sim->TimeStep({{}, {}});
	unsigned int after = sim->GetPopulation()->get_infected_count();
	ASSERT_GT(after, before);

	spdlog::drop("test_popgen");
}

} // Tests
