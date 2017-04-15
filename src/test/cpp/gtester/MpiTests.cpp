#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <gtest/gtest.h>
#include "pop/Model.h"

namespace mpi = boost::mpi;

namespace Tests {

TEST(Mpi, GetEnvironment)
{
	mpi::environment env;
	mpi::communicator world;
	std::cout << "I am process " << world.rank() << " of " << world.size()
	<< "." << std::endl;
}

} // Tests
