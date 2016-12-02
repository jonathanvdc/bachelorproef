/*
 * Copyright 2011-2016 Universiteit Antwerpen
 *
 * Licensed under the EUPL, Version 1.1 or  as soon they will be approved by
 * the European Commission - subsequent versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl5
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */
/**
 * @file
 * Implementation for FileSys class.
 */

#include "InstallDirs.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <boost/filesystem.hpp>

#include <iostream>
#include <stdexcept>

using namespace std;

namespace SimShell {

std::string 	InstallDirs::g_data_dir;
std::string 	InstallDirs::g_root_dir;
std::string 	InstallDirs::g_workspace_dir;
bool            InstallDirs::g_initialized = false;


inline void InstallDirs::Check()
{
	if (!g_initialized) {
		Initialize();
	}
}

void InstallDirs::Initialize()
{
	//------- Retrieving Install Root Path
	{

    //If this does not work, no fallback value!
    boost::filesystem::path executablePath;
    executablePath = boost::filesystem::system_complete(boost::filesystem::current_path());

    //https://stackoverflow.com/questions/1528298/get-path-of-executable

	#if defined(__APPLE__)
        #include <mach-o/dyld.h>
        char path[10000];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0)
            executablePath = boost::filesystem::system_complete(path).parent_path();
    #elif defined(__WIN32__)
        #include <windows.h>
        char ownPth[MAX_PATH];
        HMODULE hModule = GetModuleHandle(NULL);
        if (hModule != NULL){
            GetModuleFileName(hModule,ownPth,(sizeof(ownPth)));
            executablePath = boost::filesystem::system_complete(path).parent_path();
        }
    #else //Linux
        #include <unistd.h>
        char path[10000] = {0};
        ssize_t size = readlink("/proc/self/exe", path, sizeof(path));
        if (size > 0 && size != sizeof(path))
            executablePath = boost::filesystem::system_complete(path).parent_path();
    #endif

	#if defined(__APPLE__)
		//if (dir.dirName() == "MacOS") {
		if (executablePath.filename().string() == "MacOS") {
			// app
			//	-Contents		<-Root Path
			//		-MacOS
			//		     -binaries
            executablePath = executablePath.parent_path();
		} else
	#endif
        std::string lowercasePath = executablePath.filename().string();
        std::transform(lowercasePath.begin(),lowercasePath.end(),lowercasePath.begin(),::tolower);
		if (lowercasePath == "debug" || lowercasePath == "release") {
			//x/exec		<-Root Path
			//	-bin
			//		-release/debug
			//			-binaries
            executablePath = executablePath.parent_path().parent_path();
		} else 
	#if defined(__WIN32__)
		if (executablePath.filename().string() != "bin") {
				// Do Nothing, binaries in root folder
		} else
	#endif
		{
			//x/exec		<-Root Path
			//	-bin
			//		-binaries
            executablePath = executablePath.parent_path();
		}

        const bool exists = boost::filesystem::is_directory(executablePath);
		g_root_dir = (exists) ? boost::filesystem::system_complete(executablePath).string() : "";

	}
	//------- Retrieving Data Dir
	{
        boost::filesystem::path dataPath (g_root_dir);
        dataPath /= "data";
        const bool exists = boost::filesystem::is_directory(dataPath);
		g_data_dir = (exists) ? boost::filesystem::system_complete(dataPath).string() : "";
	}
	//------- Retrieving Workspace
	{
        boost::filesystem::path workspacePath (g_data_dir);
        workspacePath /= "simPT_Default_workspace";
        const bool exists = boost::filesystem::is_directory(workspacePath);
		g_workspace_dir = (exists) ? boost::filesystem::system_complete(workspacePath).string() : "";
	}

	g_initialized = true;
}

string InstallDirs::GetDataDir()
{
	Check();
	return g_data_dir;
}

string InstallDirs::GetRootDir()
{
    Check();
    return g_root_dir;
}

string InstallDirs::GetWorkspaceDir()
{
	Check();
	return g_workspace_dir;
}

} // namespace

