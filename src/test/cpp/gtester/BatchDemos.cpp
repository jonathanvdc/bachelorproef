/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Implementation of scenario test running demo cases in batch mode.
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <omp.h>
#include <string>
#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>

#include "sim/Simulator.h"

using namespace std;
using namespace indismo;
using namespace ::testing;

namespace Tests {

class BatchDemos: public TestWithParam<string>
{
public:
	/// TestCase set up.
	static void SetUpTestCase() {}

	/// Tearing down TestCase
	static void TearDownTestCase() {}

protected:
	/// Destructor has to be virtual.
	virtual ~BatchDemos() {}

	/// Set up for the test fixture
	virtual void SetUp() {}

	/// Tearing down the test fixture
	virtual void TearDown() {}

	// Data members of the test fixture
	static const string            g_population_file;
	static const double            g_r0;
	static const unsigned int      g_num_days;
	static const unsigned int      g_rng_seed;
	static const double            g_seeding_rate;
	static const double            g_immunity_rate;
	static const double            g_transmission_rate;
	static const string            g_disease_config_file;

	static const unsigned int      g_rng_seed_adapted;
	static const double            g_seeding_rate_adapted;
	static const double            g_immunity_rate_adapted;
	static const double            g_transmission_rate_adapted;
	static const string            g_disease_config_file_adapted;


	static const map<string, unsigned int>   g_results;
#ifdef _OPENMP
	static const map<string, unsigned int>   g_results_omp;
#endif
};

// Default values
const string         BatchDemos::g_population_file             = "nassau_synt_pop_sorted.csv";
const double         BatchDemos::g_r0                          = 1.1;
const unsigned int   BatchDemos::g_num_days                    = 100U;
const unsigned int   BatchDemos::g_rng_seed                    = 2015U;
const double         BatchDemos::g_seeding_rate                = 0.00001;
const double         BatchDemos::g_immunity_rate               = 0.0;
const double         BatchDemos::g_transmission_rate           = 0.00003;
const string         BatchDemos::g_disease_config_file         = "disease_config_influenza.xml";

const unsigned int   BatchDemos::g_rng_seed_adapted            = 1U;
const double         BatchDemos::g_seeding_rate_adapted        = 0.00002;
const double         BatchDemos::g_immunity_rate_adapted       = 0.1;
const double         BatchDemos::g_transmission_rate_adapted   = 0.000035;
const string         BatchDemos::g_disease_config_file_adapted = "disease_config_measles.xml";

/// map<tranmsmission rate, number of cases>
const map<string, unsigned int> BatchDemos::g_results {
	make_pair("default",120822),
	make_pair("rng_seed",29),
	make_pair("seeding_rate",124047),
	make_pair("immunity_rate",87474),
	make_pair("transmission_rate",176324),
	make_pair("measles",60398)
};

TEST_P( BatchDemos, RunOnce )
{
#ifdef _OPENMP
	// set nb of threads to 1 to reproduce serial execution results
	omp_set_num_threads(1);
	omp_set_schedule(omp_sched_dynamic,1);
#endif

	// Prepare the configuration
        const string test_tag = GetParam();
	const string disease_config_file = (test_tag != "measles") ? g_disease_config_file : g_disease_config_file_adapted;

	boost::property_tree::ptree pt_config;
	pt_config.put("run.r0", g_r0);
	pt_config.put("run.population_file", g_population_file);
	pt_config.put("run.transmission_rate",g_transmission_rate);
	pt_config.put("run.num_days", g_num_days);
	pt_config.put("run.rng_seed", g_rng_seed);
	pt_config.put("run.seeding_rate", g_seeding_rate);
	pt_config.put("run.immunity_rate", g_immunity_rate);
	pt_config.put("run.disease_config_file", disease_config_file);

	// override scenario setting
	if (test_tag == "default"){
		// do nothing
	}
	if (test_tag == "rng_seed"){
		pt_config.put("run.rng_seed", g_rng_seed_adapted);
	}
	if (test_tag == "seeding_rate"){
		pt_config.put("run.seeding_rate", g_seeding_rate_adapted);
	}
	if (test_tag == "immunity_rate"){
		pt_config.put("run.immunity_rate", g_immunity_rate_adapted);
	}
	if (test_tag == "transmission_rate"){
		pt_config.put("run.transmission_rate", g_transmission_rate_adapted);
	}

	// Initialize simulator and run it.
	Simulator sim(pt_config);
	cerr << "  starting up: " << test_tag << endl;
	for(size_t i = 0U; i < g_num_days; i++){
		sim.RunTimeStep();
	}

	// Round up
	const unsigned int num_cases = sim.GetPopulation()->GetInfectedCount();
	ASSERT_EQ(num_cases, g_results.at(test_tag)) << "!! CHANGED !!";
	cerr << "  finished:    " << test_tag << endl;
}


#ifdef _OPENMP

const map<string, unsigned int> BatchDemos::g_results_omp {
	make_pair("default", 125099),
	make_pair("rng_seed",111392),
	make_pair("seeding_rate",127143),
	make_pair("immunity_rate",87034),
	make_pair("transmission_rate",181001),
	make_pair("measles",123104)
};

TEST_P( BatchDemos, RunOnce_omp )
{
	// set nb of threads to 4 and use static scheduling to obtain repeatable results
	omp_set_num_threads(4);
	omp_set_schedule(omp_sched_static,1);

        // Prepare the configuration
        const string test_tag = GetParam();
	const string disease_config_file = (test_tag != "measles") ? g_disease_config_file : g_disease_config_file_adapted;

	// create pt_config
	boost::property_tree::ptree pt_config;
	pt_config.put("run.population_file", g_population_file);
	pt_config.put("run.transmission_rate",g_transmission_rate);
	pt_config.put("run.num_days", g_num_days);
	pt_config.put("run.rng_seed", g_rng_seed);
	pt_config.put("run.seeding_rate", g_seeding_rate);
	pt_config.put("run.immunity_rate", g_immunity_rate);
	pt_config.put("run.disease_config_file", disease_config_file);

	// override scenario setting
	if (test_tag == "default"){
		// do nothing
	}
	if (test_tag == "rng_seed"){
		pt_config.put("run.rng_seed", g_rng_seed_adapted);
	}
	if (test_tag == "seeding_rate"){
		pt_config.put("run.seeding_rate", g_seeding_rate_adapted);
	}
	if (test_tag == "immunity_rate"){
		pt_config.put("run.immunity_rate", g_immunity_rate_adapted);
	}
	if (test_tag == "transmission_rate"){
		pt_config.put("run.transmission_rate", g_transmission_rate_adapted);
	}


	// Initialize simulator and run it.
	Simulator sim(pt_config);
	cerr << "  starting up ..." << endl;
	for (size_t i = 0U; i < g_num_days; i++){
		sim.RunTimeStep();
	}

	// Round up
	const unsigned int num_cases = sim.GetPopulation()->GetInfectedCount();
	ASSERT_EQ(num_cases, g_results_omp.at(test_tag)) << "!! CHANGED !!";
	cerr << "  finished:    " << test_tag << endl;
}
#endif

namespace {
	string scenarios[] {
		"default",
		"rng_seed",
		"seeding_rate",
		"immunity_rate",
		"transmission_rate",
		"measles"
	};
}

INSTANTIATE_TEST_CASE_P(RunBatch, BatchDemos, ValuesIn(scenarios));

} //end-of-namespace-Tests


