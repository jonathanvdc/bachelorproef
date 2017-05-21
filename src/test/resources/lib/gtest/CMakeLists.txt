#============================================================================
# Build Google test framework gtest (build only, no install)
#============================================================================
set( GTEST_RESOURCE_DIR	   ${CMAKE_SOURCE_DIR}/test/resources/lib/gtest/ )
set( CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} -w" )
include_directories( ${GTEST_RESOURCE_DIR} )
add_definitions( -DGTEST_LANG_CXX11 )
add_library( gtest STATIC ${GTEST_RESOURCE_DIR}/src/gtest-all.cc )

#############################################################################
