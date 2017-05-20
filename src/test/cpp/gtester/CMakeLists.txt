#============================================================================
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
set( EXEC       gtester     )
set( SRC
		AliasTest.cpp
		CheckPointTest.cpp
		BatchRuns.cpp
		GeoPosition.cpp
		main.cpp
		ParallelTest.cpp
		ParsePopulationModel.cpp
		ParseSimulationConfig.cpp
		ParseTravelConfig.cpp
		PopulationGeneration.cpp
		RunSimulator.cpp
		TravelModelGraph.cpp
)

if ( MPI_FOUND )
	set( SRC ${SRC} MpiTests.cpp )
endif()

add_executable(${EXEC}   ${SRC} $<TARGET_OBJECTS:libstride> $<TARGET_OBJECTS:trng>)
target_link_libraries( ${EXEC}    ${LIBS} gtest pthread)
install(TARGETS ${EXEC}  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Define tests.
#============================================================================
add_test( NAME  ${EXEC}_default
		WORKING_DIRECTORY  ${TESTS_DIR}
		COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC}
			--gtest_filter=*default*  --gtest_output=xml:gtester_default.xml
)

add_test( NAME  ${EXEC}_all
        WORKING_DIRECTORY  ${TESTS_DIR}
		COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC}
		    --gtest_output=xml:gtester_all.xml
)

#============================================================================
# Clean up.
#============================================================================
unset( EXEC      )
unset( SRC       )

#############################################################################
