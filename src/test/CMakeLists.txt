#############################################################################
#  This file is part of the Indismo software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and 
#  Broeckhove J. (2015) Optimizing agent-based transmission models for  
#  infectious diseases, BMC Bioinformatics.
#
#  Copyright 2015, Willem L, Stijven S & Broeckhove J
#############################################################################

#============================================================================
# Test verbosity:
#============================================================================
unset( CTEST_VERBOSE_OPTION )
if( INDISMO2_VERBOSE_TESTING )
	set( CTEST_VERBOSE_OPTION "-V" )
endif()

#============================================================================
# Top level custom targets:
#============================================================================
set( TESTS_DIR ${CMAKE_INSTALL_PREFIX}/${TESTS_INSTALL_LOCATION} )
add_custom_target( run_ctest
	COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_DIR}
	COMMAND ctest --exclude-regex .*perf.* ${CTEST_VERBOSE_OPTION} 
				-O ${TESTS_DIR}/reports/ctests.txt
)

#============================================================================
# Config related to using Google test (src in resources/lib/gtest) for cpp
#============================================================================
set( GTEST_RESOURCE_DIR	   ${CMAKE_SOURCE_DIR}/test/resources/lib/gtest/ )
include_directories( ${GTEST_RESOURCE_DIR} )
add_definitions( -DGTEST_LANG_CXX11 )

#============================================================================
# Add subdirectories:
#============================================================================
add_subdirectory( cpp       )
add_subdirectory( resources )

#############################################################################
