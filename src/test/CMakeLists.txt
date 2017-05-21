#============================================================================
# Test verbosity:
#============================================================================
unset( CTEST_VERBOSE_OPTION )
if( STRIDE_VERBOSE_TESTING )
	set( CTEST_VERBOSE_OPTION "-V" )
endif()

#============================================================================
# Meke test directory in install tree:
#============================================================================
set( TESTS_DIR ${CMAKE_INSTALL_PREFIX}/${TESTS_INSTALL_LOCATION} )
install(DIRECTORY DESTINATION  ${TESTS_DIR} )

#============================================================================
# Top level custom target for instactive use through make:
#============================================================================
add_custom_target( run_ctest
	COMMAND ctest --tests-regex default ${CTEST_VERBOSE_OPTION}
				-O ${TESTS_DIR}/ctests_default.txt
)
add_custom_target( run_ctest_all
	COMMAND ctest --tests-regex all --exclude-regex no_x ${CTEST_VERBOSE_OPTION}
				-O ${TESTS_DIR}/ctests_all.txt
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
