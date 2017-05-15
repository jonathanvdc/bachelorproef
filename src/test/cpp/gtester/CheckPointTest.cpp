#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <checkpoint/CheckPoint.h>
#include <util/InstallDirs.h>
#include <gtest/gtest.h>
#include <pop/Population.h>
#include <core/ClusterType.h>
#include <stdio.h>

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

TEST(CheckPoint, WriteConfig){
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_default.xml",util::InstallDirs::GetRootDir(),pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	stride::checkpoint::CheckPoint cp("WriteConfig.h5",1);
	cp.CreateFile();
	boost::filesystem::path f("test.h5");
	cp.OpenFile();
	cp.WriteConfig(config);
	cp.CloseFile();
}

TEST(CheckPoint, SaveCheckPoint)
{	
	boost::property_tree::ptree pt_config;
	util::InstallDirs::ReadXmlFile("config/run_default.xml",util::InstallDirs::GetRootDir(),pt_config);

	MultiSimulationConfig config;
	config.Parse(pt_config.get_child("run"));
	config.common_config->track_index_case = 0;

	stride::checkpoint::CheckPoint cp("SaveCheckPoint.h5",1);
	cp.CreateFile();
	cp.OpenFile();
	cp.WriteConfig(config);
	cp.CloseFile();

	boost::property_tree::ptree pt_contacts;
	util::InstallDirs::GetDataDir();
	util::InstallDirs::ReadXmlFile("contact_matrix.xml",util::InstallDirs::GetDataDir(),pt_contacts);
	Cluster::AddContactProfile(ClusterType::Household,ContactProfile(ClusterType::Household, pt_contacts));
	Cluster::AddContactProfile(ClusterType::School,ContactProfile(ClusterType::School, pt_contacts));
	Cluster::AddContactProfile(ClusterType::Work,ContactProfile(ClusterType::Work, pt_contacts));
	Cluster::AddContactProfile(ClusterType::PrimaryCommunity,ContactProfile(ClusterType::PrimaryCommunity, pt_contacts));
	Cluster::AddContactProfile(ClusterType::SecondaryCommunity,ContactProfile(ClusterType::SecondaryCommunity, pt_contacts));

	Population toSave;
	disease::Fate fate;
	fate.start_infectiousness = 1;
	fate.start_symptomatic = 2;
	fate.end_infectiousness = 4;
	fate.end_symptomatic = 3;
	std::vector<Cluster> households;
	for (unsigned int i = 0; i < 6; i++) {
		households.push_back(Cluster(i, ClusterType::Household));
	}
	std::vector<Cluster> school;
	for (unsigned int i = 0; i < 5; i++) {
		school.push_back(Cluster(i, ClusterType::School));
	}
	std::vector<Cluster> work;
	for (unsigned int i = 0; i < 5; i++) {
		work.push_back(Cluster(i, ClusterType::Work));
	}
	std::vector<Cluster> primary;
	for (unsigned int i = 0; i < 5; i++) {
		primary.push_back(Cluster(i, ClusterType::PrimaryCommunity));
	}
	std::vector<Cluster> secondary;
	for (unsigned int i = 0; i < 5; i++) {
		secondary.push_back(Cluster(i, ClusterType::SecondaryCommunity));
	}
	for (unsigned int i = 0; i < 100; i++) {
		Person p(i, 18.0, i % 6, i % 5, i % 4, i % 3, i % 2, fate);
		toSave.emplace(p);
		households[i % 6].AddPerson(p);
		school[i % 5].AddPerson(p);
		work[i % 4].AddPerson(p);
		primary[i % 3].AddPerson(p);
		secondary[i % 2].AddPerson(p);
	}
	std::vector<std::vector<Cluster> > clusters = {households, school, work, primary, secondary};
	cp.OpenFile();
	cp.SaveCheckPoint(toSave, clusters, 0);
	cp.CloseFile();
}

} // Tests