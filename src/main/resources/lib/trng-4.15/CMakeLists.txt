#############################################################################
#
#  Build trng.
#
#############################################################################

#============================================================================
# Build Google test framework gtest (build only, no install)
#============================================================================
set( SRC
	src/lcg64.cc
	src/lcg64_shift.cc
	src/minstd.cc
	src/mrg2.cc
	src/mrg3.cc
	src/mrg3s.cc
	src/mrg4.cc
	src/mrg5.cc
	src/mrg5s.cc
	src/mt19937.cc
	src/mt19937_64.cc
	src/yarn2.cc
	src/yarn3.cc
	src/yarn3s.cc
	src/yarn4.cc
	src/yarn5.cc
	src/yarn5s.cc
)

add_library( trng OBJECT ${SRC} )
#target_compile_options(trng PUBLIC "-flto")
unset( SRC )

#############################################################################
