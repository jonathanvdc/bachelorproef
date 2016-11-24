#############################################################################
#  This file is part of the indismo software. 
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
# Build & install gtester
#============================================================================
set( EXEC       gtester     )
set( EXEC_OMP   gtester_omp )
set( SRC
		main.cpp
		BatchDemos.cpp
)

#------------------------------ non omp gtester ------------------------------
add_executable(${EXEC}   ${SRC})
target_link_libraries( ${EXEC}   indismo_lib ${LIBS} gtest pthread ${TRNG_LIBRARY})  
install(TARGETS ${EXEC}  DESTINATION   ${BIN_INSTALL_LOCATION})

#------------------------------ omp gtester ----------------------------------
add_executable(${EXEC_OMP}   ${SRC})
set_target_properties(${EXEC_OMP} PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")
target_link_libraries( ${EXEC_OMP}  indismo_lib_omp ${LIBS} gtest_omp pthread ${TRNG_LIBRARY})  
install(TARGETS ${EXEC_OMP}  DESTINATION   ${BIN_INSTALL_LOCATION})

#------------------------------ run test ------------------------------------
add_test( NAME  ${EXEC} 
		WORKING_DIRECTORY  ${TESTS_DIR}
		COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC} 
			--gtest_filter=*:-*Perf*.* --gtest_output=xml:reports/${EXEC}.xml 
)
add_test( NAME  ${EXEC_OMP}
		WORKING_DIRECTORY  ${TESTS_DIR}
		COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC_OMP} 
			--gtest_filter=*:-*Perf*.* --gtest_output=xml:reports/${EXEC_OMP}.xml 
)

#------------------------------ clean up -----------------------------------
unset( EXEC_OMP  )	
unset( EXEC      )
unset( SRC       )

#############################################################################