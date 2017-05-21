#include <cerrno>
#include <exception>
#include <iostream>
#include <gtest/gtest.h>
#include "util/Signals.h"

using namespace std;

int main(int argc, char** argv)
{
	stride::util::setup_segfault_handler();

	std::cout << "START TEST ENVIRONMENT" << std::endl;

	int exit_status = EXIT_SUCCESS;
	try {
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	} catch (std::exception& e) {
		cerr << "Exception caught: " << e.what() << endl << endl;
		exit_status = EXIT_FAILURE;
	}
	return exit_status;
}
