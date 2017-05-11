#include <iostream>
#include <vector>
#include <checkpoint/CheckPoint.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <boost/filesystem.hpp>


namespace Tests {

TEST(CheckPoint, CheckPoint)
{
	stride::checkpoint::CheckPoint("test.h5");
	stride::checkpoint::CheckPoint("test2.h5",3);
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


} // Tests