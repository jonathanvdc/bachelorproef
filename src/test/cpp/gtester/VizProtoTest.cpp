#include <iostream>
#include <gtest/gtest.h>
#include "viz/prototype.h"
#include "pop/Population.h"
#include "sim/Simulator.h"


using namespace Stride;

namespace Tests {

// Tests that make use of X (the linux window system) need to end in '_x'
TEST(Visualiser_x, VisualiserRunPrototype_x)
{
	VizProto p = VizProto();

    // If this does not fail, the test is complete
    p.run();
}

} // Tests
