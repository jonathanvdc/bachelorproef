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

#include "util/Errors.h"
#include "util/StringUtils.h"

#include <memory>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#if defined(WIN32)
#include <stdlib.h>
#include <windows.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
#endif

namespace stride {
namespace util {

using namespace std;
using namespace boost::filesystem;

path InstallDirs::g_bin_dir;
path InstallDirs::g_current_dir;
path InstallDirs::g_data_dir;
path InstallDirs::g_exec_path;
path InstallDirs::g_root_dir;

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
	//------- Retrieving path of executable
	{
// Returns the full path to the currently running executable, or an empty string in case of failure.
// http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe/33249023#33249023

#if defined(WIN32)
		char exePath[MAX_PATH];
		HMODULE hModule = GetModuleHandle(NULL);
		if (GetModuleFileName(NULL, exePath, sizeof(exePath)) != 0)
			;
		{
			g_exec_path = canonical(system_complete(exePath));
		}
#elif defined(__linux__)
		char exePath[PATH_MAX + 1];
		std::memset(exePath, 0, sizeof exePath);
		std::size_t size = ::readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
		if (size > 0 && size != sizeof(exePath) - 1) {
			g_exec_path = canonical(system_complete(exePath));
		}
#elif defined(__APPLE__)
		char exePath[PATH_MAX];
		uint32_t size = sizeof(exePath);
		if (_NSGetExecutablePath(exePath, &size) == 0) {
			g_exec_path = canonical(system_complete(exePath));
		}
#endif
	}

	//------- Retrieving root and bin directory (the subdirectory of the install root)
	{
		path exec_dir = g_exec_path.parent_path();
		if (!g_exec_path.empty()) {
#if (__APPLE__)
			if (exec_dir.filename().string() == "MacOS") {
				// app
				//      -Contents               <-Root Path
				//              -MacOS
				//                   -executables
				g_bin_dir = exec_dir;
				g_root_dir = exec_dir.parent_path();
			} else
#endif
			    if (StringUtils::ToLower(exec_dir.filename().string()) == "debug" ||
				StringUtils::ToLower(exec_dir.filename().string()) == "release") {
				// x/exec                <-Root Path
				//      -bin
				//              -release/debug
				//                      -executables
				g_bin_dir = exec_dir.parent_path();
				g_root_dir = exec_dir.parent_path().parent_path();
			} else
#if (WIN32)
			    if (exec_dir.filename().string() != "bin") {
				// Executables in root folder
				g_bin_dir = exec_dir;
				g_root_dir = exec_dir;
			} else
#endif
			{
				// x/exec                <-Root Path
				//      -bin
				//              -executables
				g_bin_dir = exec_dir;
				g_root_dir = exec_dir.parent_path();
			}
		}
	}

	//------- Data Dir
	{
		g_data_dir = g_root_dir / "data";
		g_data_dir = is_directory(g_data_dir) ? g_data_dir : path();
	}
	//------- Current Dir
	{
		g_current_dir = system_complete(current_path());
	}
}

path InstallDirs::GetBinDir()
{
	Check();
	return g_bin_dir;
}

path InstallDirs::GetCurrentDir()
{
	Check();
	return g_current_dir;
}

path InstallDirs::GetDataDir()
{
	Check();
	return g_data_dir;
}

path InstallDirs::GetExecPath()
{
	Check();
	return g_exec_path;
}

path InstallDirs::GetRootDir()
{
	Check();
	return g_root_dir;
}

std::unique_ptr<boost::filesystem::ifstream> InstallDirs::OpenFile(
    const boost::filesystem::path& relative_path, const boost::filesystem::path& anchor_path)
{
	auto file_path = anchor_path / relative_path;
	if (!is_regular_file(file_path)) {
		FATAL_ERROR("File " + file_path.string() + " not present.");
	}

	auto stream = std::make_unique<boost::filesystem::ifstream>();
	stream->open(file_path.string());
	if (!stream->is_open()) {
		FATAL_ERROR("Error opening file " + file_path.string());
	}

	return stream;
}

void InstallDirs::ReadXmlFile(
    const boost::filesystem::path& relative_path, const boost::filesystem::path& anchor_path,
    boost::property_tree::ptree& result)
{
	auto stream = OpenFile(relative_path, anchor_path);
	boost::property_tree::read_xml(*stream, result);
}

std::unique_ptr<boost::filesystem::ifstream> InstallDirs::OpenDataFile(const boost::filesystem::path& relative_path)
{
	return OpenFile(relative_path, InstallDirs::GetDataDir());
}

} // namespace
} // namespace
