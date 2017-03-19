#include <iostream>
#include <Awesomium/WebCore.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/STLHelpers.h>
#include <gtest/gtest.h>
#include "sim/run_stride.h"

namespace Tests {

TEST(RunSimulator, RunSimulatorDefault)
{
	stride::run_stride(false, "../config/run_default.xml");
}

} // Tests
