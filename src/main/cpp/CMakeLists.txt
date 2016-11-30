#############################################################################
#  This is free software: you can redistribute it and/or modify
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
#  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
#############################################################################

#============================================================================
# Sources.
#============================================================================
set(LIB_SRC
	output/CasesFile.cpp
	output/PersonFile.cpp
	output/SummaryFile.cpp
	sim/Simulator.cpp
	sim/WorldEnvironment.cpp
)

set(MAIN_SRC
	sim/main.cpp
)

#============================================================================
# Build & install the (non omp) executable.
#============================================================================
add_library(indismo_lib  STATIC  ${LIB_SRC})
#
add_executable(indismo  ${MAIN_SRC})
target_link_libraries(indismo   indismo_lib ${LIBS}) 
install(TARGETS indismo  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Build & install the (omp) executable.
#============================================================================
add_library(indismo_lib_omp  STATIC  ${LIB_SRC})
set_target_properties(indismo_lib_omp PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")
#
add_executable(indismo_omp  ${MAIN_SRC})
set_target_properties(indismo_omp PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp")
target_link_libraries(indismo_omp   indismo_lib_omp ${LIBS}) 
install(TARGETS indismo_omp  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Build & install the (R0) executable.
#============================================================================
add_library(indismo_lib_r0  STATIC  ${LIB_SRC})
set_target_properties(indismo_lib_r0 PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp" COMPILE_DEFINITIONS TRACKINDEXCASE)
#
add_executable(indismo_r0  ${MAIN_SRC})
set_target_properties(indismo_r0 PROPERTIES COMPILE_FLAGS "-fopenmp" LINK_FLAGS "-fopenmp" COMPILE_DEFINITIONS TRACKINDEXCASE)
target_link_libraries( indismo_r0   indismo_lib_r0 ${LIBS}) 
install(TARGETS indismo_r0  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Clean up.
#============================================================================
unset(LIB_SRC)
unset(MAIN_SRC)

#############################################################################
