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

/*
TEST(CheckPoint, WriteConfig)
{
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_default.xml", util::InstallDirs::GetRootDir(), pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	stride::checkpoint::CheckPoint cp("WriteConfig.h5", 1);
	cp.CreateFile();
	boost::filesystem::path f("test.h5");
	cp.OpenFile();
	cp.WriteConfig(config);
	cp.CloseFile();
}

TEST(CheckPoint, WriteHolidays)
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
}

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