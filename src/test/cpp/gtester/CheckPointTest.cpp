#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <checkpoint/CheckPoint.h>
#include <core/ClusterType.h>
#include <gtest/gtest.h>
#include <pop/Population.h>
#include <sim/SimulatorBuilder.h>
#include <stdio.h>
#include <util/InstallDirs.h>

using namespace stride;

namespace Tests {

TEST(CheckPoint, CheckPoint)
{
	stride::checkpoint::CheckPoint("test.h5");
	stride::checkpoint::CheckPoint("test2.h5", 3);
}

TEST(CheckPoint, CreateFile)
{
	stride::checkpoint::CheckPoint cp("test.h5");
	cp.CreateFile();
	boost::filesystem::path f("test.h5");
	EXPECT_TRUE(boost::filesystem::is_regular_file(f));
	boost::filesystem::remove(f);
}

TEST(CheckPoint, Open_CloseFile)
{
	stride::checkpoint::CheckPoint cp("test.h5");
	cp.CreateFile();
	boost::filesystem::path f("test.h5");
	cp.OpenFile();
	cp.CloseFile();
	boost::filesystem::remove(f);
}

TEST(CheckPoint, Write_LoadConfig)
{
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_default.xml", util::InstallDirs::GetRootDir(), pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	auto toSave = config.AsSingleConfig();

	stride::checkpoint::CheckPoint cp("WriteConfig.h5", 1);
	cp.CreateFile();
	cp.OpenFile();
	cp.WriteConfig(toSave);
	cp.CloseFile();

	SingleSimulationConfig toCheck;

	cp.OpenFile();
	toCheck = cp.LoadSingleConfig();
	cp.CloseFile();

	EXPECT_EQ(toSave.common_config->track_index_case, toCheck.common_config->track_index_case);
	EXPECT_EQ(toSave.common_config->rng_seed, toCheck.common_config->rng_seed);
	EXPECT_EQ(toSave.common_config->r0, toCheck.common_config->r0);
	EXPECT_EQ(toSave.common_config->seeding_rate, toCheck.common_config->seeding_rate);
	EXPECT_EQ(toSave.common_config->immunity_rate, toCheck.common_config->immunity_rate);
	EXPECT_EQ(toSave.common_config->number_of_days, toCheck.common_config->number_of_days);
	EXPECT_EQ(
	    toSave.common_config->number_of_survey_participants, toCheck.common_config->number_of_survey_participants);

	EXPECT_EQ(toSave.log_config->output_prefix, toCheck.log_config->output_prefix);
	EXPECT_EQ(toSave.log_config->generate_person_file, toCheck.log_config->generate_person_file);
	EXPECT_EQ(toSave.log_config->log_level, toCheck.log_config->log_level);

	EXPECT_EQ(toSave.GetId(),toCheck.GetId());

	// TODO: check contents of files
	// TODO: check travelmodel
}

TEST(CheckPoint, Write_LoadHolidays)
{
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_default.xml", util::InstallDirs::GetRootDir(), pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	stride::checkpoint::CheckPoint cp("WriteHolidays.h5", 1);
	cp.CreateFile();
	boost::filesystem::path f("test.h5");
	cp.OpenFile();
	cp.WriteHolidays(pt_config.get_child("run").get<std::string>("holidays_file", "holidays_flanders_2016.json"));
	cp.CloseFile();

	cp.OpenFile();
	Calendar c =cp.LoadCalendar(boost::gregorian::date(2017,01,01));
	cp.CloseFile();

	EXPECT_EQ(c.GetYear(),2017);
	EXPECT_EQ(c.GetMonth(),1);
	EXPECT_EQ(c.GetDay(),1);


}

/*
TEST(CheckPoint, SaveCheckPoint)
{
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_test_popgen.xml", util::InstallDirs::GetRootDir(), pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	std::string sim_output_prefix = "test";

	auto log_name = std::string("contact_logger_") + sim_output_prefix;
	auto file_logger = spdlog::rotating_logger_mt(
	    log_name, sim_output_prefix + "_logfile", std::numeric_limits<size_t>::max(),
	    std::numeric_limits<size_t>::max());
	file_logger->set_pattern("%v"); // Remove meta data from log => time-stamp of logging

	auto sim = SimulatorBuilder::Build(config.AsSingleConfig(), file_logger);

	stride::checkpoint::CheckPoint* cp = new stride::checkpoint::CheckPoint("SaveCheckPoint.h5", 1);

	std::cout << sim->GetPopulation()->size() << std::endl;

	cp->CreateFile();
	cp->OpenFile();
	cp->SaveCheckPoint(*sim->GetPopulation(), sim->GetClusters(), 0);
	cp->CloseFile();
}

TEST(CheckPoint, todo)
{

}
*/
} // Tests