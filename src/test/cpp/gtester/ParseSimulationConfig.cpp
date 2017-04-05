#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gtest/gtest.h>
#include "core/LogMode.h"
#include "sim/SimulationConfig.h"

namespace Tests {

void assert_default_travel_config(
	const std::vector<stride::multiregion::RegionTravelRef>& regions);

TEST(ParseSimulationConfig, ParseDefaultSimulationConfig)
{
    // <rng_seed>1</rng_seed>
    // <r0>11</r0>
    // <seeding_rate>0.002</seeding_rate>
    // <immunity_rate>0.8</immunity_rate>
    // <population_file>pop_nassau.csv</population_file>
    // <num_days>50</num_days>
    // <output_prefix></output_prefix>
    // <disease_config_file>disease_measles.xml</disease_config_file>
    // <generate_person_file>1</generate_person_file>
    // <num_participants_survey>10</num_participants_survey>
    // <start_date>2017-01-01</start_date>
    // <holidays_file>holidays_none.json</holidays_file>
    // <age_contact_matrix_file>contact_matrix_average.xml</age_contact_matrix_file>
    // <log_level>Transmissions</log_level>

	std::ifstream pop_file{"../config/run_default.xml"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::MultiSimulationConfig config;
	config.Parse(pt.get_child("run"));

	EXPECT_EQ(config.common_config->rng_seed, 1u);
	EXPECT_DOUBLE_EQ(config.common_config->r0, 11);
	EXPECT_DOUBLE_EQ(config.common_config->seeding_rate, 0.002);
	EXPECT_DOUBLE_EQ(config.common_config->immunity_rate, 0.8);
	EXPECT_EQ(config.region_models.size(), 1u);
	EXPECT_EQ(config.region_models[0]->GetRegionId(), 0u);
	EXPECT_EQ(config.region_models[0]->GetRegionPopulationPath(), "pop_nassau.csv");
	EXPECT_EQ(config.common_config->number_of_days, 50u);
	EXPECT_EQ(config.log_config->output_prefix, "");
	EXPECT_EQ(config.common_config->disease_config_file_name, "disease_measles.xml");
	EXPECT_TRUE(config.log_config->generate_person_file);
	EXPECT_EQ(config.common_config->number_of_survey_participants, 10u);
	EXPECT_EQ(config.common_config->initial_calendar.GetDay(), 1u);
	EXPECT_EQ(config.common_config->initial_calendar.GetMonth(), 1u);
	EXPECT_EQ(config.common_config->initial_calendar.GetYear(), 2017u);
	EXPECT_EQ(config.common_config->contact_matrix_file_name, "contact_matrix_average.xml");
	EXPECT_EQ(config.log_config->log_level, stride::LogMode::Transmissions);
}

TEST(ParseSimulationConfig, ExceptionOnInvalidFile)
{
	std::istringstream pop_file{"<a>123</a>"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::MultiSimulationConfig config;
	EXPECT_THROW(config.Parse(pt.get_child("run")), boost::property_tree::ptree_error);
}

TEST(ParseSimulationConfig, ParseTravelSimulationConfig)
{
    // <rng_seed>1</rng_seed>
    // <r0>11</r0>
    // <seeding_rate>0.002</seeding_rate>
    // <immunity_rate>0.8</immunity_rate>
    // <travel_file>travel_test.xml</travel_file>
    // <num_days>50</num_days>
    // <output_prefix></output_prefix>
    // <disease_config_file>disease_measles.xml</disease_config_file>
    // <generate_person_file>1</generate_person_file>
    // <num_participants_survey>10</num_participants_survey>
    // <start_date>2017-01-01</start_date>
    // <holidays_file>holidays_none.json</holidays_file>
    // <age_contact_matrix_file>contact_matrix_average.xml</age_contact_matrix_file>
    // <log_level>Transmissions</log_level>

	std::ifstream pop_file{"../config/run_travel_test.xml"};
	boost::property_tree::ptree pt;
	boost::property_tree::read_xml(pop_file, pt);
	stride::MultiSimulationConfig config;
	config.Parse(pt.get_child("run"));

	EXPECT_EQ(config.common_config->rng_seed, 1u);
	EXPECT_DOUBLE_EQ(config.common_config->r0, 11);
	EXPECT_DOUBLE_EQ(config.common_config->seeding_rate, 0.002);
	EXPECT_DOUBLE_EQ(config.common_config->immunity_rate, 0.8);
	assert_default_travel_config(config.region_models);
	EXPECT_EQ(config.common_config->number_of_days, 50u);
	EXPECT_EQ(config.log_config->output_prefix, "");
	EXPECT_EQ(config.common_config->disease_config_file_name, "disease_measles.xml");
	EXPECT_TRUE(config.log_config->generate_person_file);
	EXPECT_EQ(config.common_config->number_of_survey_participants, 10u);
	EXPECT_EQ(config.common_config->initial_calendar.GetDay(), 1u);
	EXPECT_EQ(config.common_config->initial_calendar.GetMonth(), 1u);
	EXPECT_EQ(config.common_config->initial_calendar.GetYear(), 2017u);
	EXPECT_EQ(config.common_config->contact_matrix_file_name, "contact_matrix_average.xml");
	EXPECT_EQ(config.log_config->log_level, stride::LogMode::Transmissions);
}

} // Tests
