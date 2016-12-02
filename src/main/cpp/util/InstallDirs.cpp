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
 * Interface for install directory queries.
 */

#include "InstallDirs.h"
#include "util/StringUtils.h"

#include <boost/filesystem.hpp>
#include <boost/predef/os.h>
#include <algorithm>
#include <string>

#if (BOOST_OS_WINDOWS)
#  include <stdlib.h>
#  include <windows.h>
#elif (BOOST_OS_LINUX)
#  include <unistd.h>
#  include <limits.h>
#elif (BOOST_OS_MACOS)
#  include <mach-o/dyld.h>
#  include <limits.h>
#endif



using namespace std;
using namespace boost;
using namespace indismo::util;

namespace indismo {

boost::filesystem::path     InstallDirs::g_bin_dir;
boost::filesystem::path     InstallDirs::g_config_dir;
boost::filesystem::path     InstallDirs::g_current_dir;
boost::filesystem::path     InstallDirs::g_data_dir;
boost::filesystem::path     InstallDirs::g_exec_name;
boost::filesystem::path     InstallDirs::g_root_dir;

inline void InstallDirs::Check()
{
        static bool initialized = false;
	if (!initialized) {
		Initialize();
		initialized = true;
	}
}

void InstallDirs::Initialize()
{
        // Empty path.
        boost::filesystem::path exec_path;
        boost::filesystem::path exec_name;

	//------- Retrieving Install Root Path
	{
		// Returns the full path to the currently running executable, or an empty string in case of failure.
		// http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe/33249023#33249023

	        #if (BOOST_OS_WINDOWS)
                        char exePath[MAX_PATH];
                        HMODULE hModule = GetModuleHandle(NULL);
                        if (GetModuleFileName(NULL, exePath, sizeof(exePath)) !=0); {
                                exec_name = boost::filesystem::system_complete(exePath);
                                exec_path = exec_name.parent_path();
                        }
		#elif (BOOST_OS_LINUX)
			char exePath[PATH_MAX];
			size_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
		        if (size > 0 && size != sizeof(path)) {
                                exec_name = boost::filesystem::system_complete(exePath);
                                exec_path = exec_name.parent_path();
		        }
		#elif (BOOST_OS_MACOS)
			char exePath[PATH_MAX];
			uint32_t size = sizeof(exePath);
		        if (_NSGetExecutablePath(exePath, &size) == 0) {
                                exec_name = boost::filesystem::system_complete(exePath);
                                exec_path = exec_name.parent_path();
		        }
		#endif

		if (!exec_path.empty()) {
                        #if (BOOST_OS_MACOS)
                                if (exec_path.filename().string() == "MacOS") {
                                        // app
                                        //      -Contents               <-Root Path
                                        //              -MacOS
                                        //                   -binaries
                                        exec_path = exec_path.parent_path();
                                } else
                        #endif
                                if (StringUtils::ToLower(exec_path.filename().string()) == "debug"
                                        || StringUtils::ToLower(exec_path.filename().string()) == "release") {
                                        //x/exec                <-Root Path
                                        //      -bin
                                        //              -release/debug
                                        //                      -binaries
                                        exec_path = exec_path.parent_path().parent_path();
                                } else
                        #if (BOOST_OS_WINDOWS)
                                if (exec_path.filename().string() != "bin") {
                                        // Do Nothing, binaries in root folder
                                } else
                        #endif
                                {
                                        //x/exec                <-Root Path
                                        //      -bin
                                        //              -binaries
                                        exec_path = exec_path.parent_path();
                                }
		}

		// Really make sure everything is ok
                const bool exists = boost::filesystem::is_directory(exec_path);
                g_root_dir = (exists) ? boost::filesystem::system_complete(exec_path) : boost::filesystem::path();
	}

	//------- Exec name
	{
	        g_exec_name = boost::filesystem::system_complete(exec_name);
	}
        //------- Bin Dir
        {
                g_bin_dir = g_root_dir / "bin";
                const bool exists = boost::filesystem::exists(g_bin_dir);
                g_bin_dir = (exists) ? g_bin_dir : boost::filesystem::path();
        }
	//------- Config Dir
	{
                g_config_dir = g_root_dir / "config";
                const bool exists = boost::filesystem::exists(g_config_dir);
                g_config_dir = (exists) ? g_config_dir : boost::filesystem::path();
	}
	//------- Retrieving Config Dir
	{
                g_data_dir = g_root_dir / "data";
                const bool exists = boost::filesystem::exists(g_data_dir);
                g_data_dir = (exists) ? g_data_dir : boost::filesystem::path();
	}
	//------- Current Dir
	{
	        boost::filesystem::path executablePath;
	        g_current_dir = boost::filesystem::system_complete(boost::filesystem::current_path());
	}
}

boost::filesystem::path InstallDirs::GetBinDir()
{
        Check();
        return g_bin_dir;
}

boost::filesystem::path InstallDirs::GetConfigDir()
{
        Check();
        return g_config_dir;
}

boost::filesystem::path InstallDirs::GetCurrentDir()
{
        Check();
        return g_current_dir;
}

boost::filesystem::path InstallDirs::GetDataDir()
{
	Check();
	return g_data_dir;
}

boost::filesystem::path InstallDirs::GetExecName()
{
        Check();
        return g_exec_name;
}

boost::filesystem::path InstallDirs::GetRootDir()
{
	Check();
	return g_root_dir;
}

} // namespace
