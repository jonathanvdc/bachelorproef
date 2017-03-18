#include <iostream>
#include <Awesomium/WebCore.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/STLHelpers.h>
#include <gtest/gtest.h>
#include "viz/prototype.h"
#include "pop/Population.h"
#include "sim/Simulator.h"


using namespace Stride;

namespace Tests {

// Tests that make use of X need to end in '_x'
TEST(Awesomium_x, AwesomiumRunPrototype_x)
{
	AwesomiumProto p = AwesomiumProto();

	p.main();	
}

} // Tests
