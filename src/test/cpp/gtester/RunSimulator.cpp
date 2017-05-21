#include <iostream>
#include <gtest/gtest.h>
#include "sim/run_stride.h"

namespace Tests {

<<<<<<< HEAD
TEST(RunSimulator, RunSimulatorDefault)
{
	stride::run_stride(false, "../config/run_default.xml","","");
}

TEST(RunSimulator, RunSimulatorMultiregion)
{
	stride::run_stride(false, "../config/run_multiregion.xml","","");
}

TEST(RunSimulator, RunSimulatorTravel)
{
	stride::run_stride(false, "../config/run_travel_test.xml","","");
}
=======
TEST(RunSimulator, RunSimulatorDefault) { stride::run_stride(false, "../config/run_default.xml"); }

TEST(RunSimulator, RunSimulatorMultiregion) { stride::run_stride(false, "../config/run_multiregion.xml"); }

TEST(RunSimulator, RunSimulatorTravel) { stride::run_stride(false, "../config/run_travel_test.xml"); }
>>>>>>> 7c725e4042e52a3bab5dabbdff93de93b56683f2

} // Tests
