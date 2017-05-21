#include <iostream>
#include <gtest/gtest.h>
#include "sim/run_stride.h"

namespace Tests {

TEST(RunSimulator, RunSimulatorDefault) { stride::run_stride(false, "../config/run_default.xml", "", ""); }

TEST(RunSimulator, RunSimulatorMultiregion) { stride::run_stride(false, "../config/run_multiregion.xml", "", ""); }

TEST(RunSimulator, RunSimulatorTravel) { stride::run_stride(false, "../config/run_travel_test.xml", "", ""); }

} // Tests
