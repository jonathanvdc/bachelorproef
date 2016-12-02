#ifndef UTIL_INSTALL_DIRS_H_INCLUDED
#define UTIL_INSTALL_DIRS_H_INCLUDED
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
 * Interface for FileSys.
 */

#include <string>

namespace SimShell {

/**
 * Install directories.
 */
class InstallDirs {
public:
	/// Utility method: get name of the directory for data files.
	static std::string GetDataDir();

	/// Utility method: get name of the directory for data files.
	static std::string GetWorkspaceDir();

	/// Utility method: get application installation root directory.
	static std::string GetRootDir();

	/// Utility method: get name of working directory for the tests.
	static std::string GetTestsDir();

private:
	/// Check initialization.
	static void Check();

	/// Initialize all paths.
	static void Initialize();

	/// Check tests initialization.
	static void CheckTests();

	/// Initialize test paths.
	static void InitializeTests();

private:
	static bool	  	    g_initialized;
	static bool		    g_tests_initialized;

private:
	static std::string 	g_data_dir;
	static std::string 	g_workspace_dir;
	static std::string 	g_root_dir;
	static std::string  g_tests_dir;
	static std::string  g_tests_workspace_dir;
};

} // namespace

#endif // end-of-include-guard
