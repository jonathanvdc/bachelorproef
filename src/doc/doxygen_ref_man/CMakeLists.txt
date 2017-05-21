#============================================================================
# api_doc
#============================================================================
find_package( Doxygen )

if( DOXYGEN_FOUND )

	#=== dot tool ===
	if( DOXYGEN_DOT_FOUND )
		set( DOXY_HAVE_DOT YES )
	else()
		set( DOXY_HAVE_DOT NO )
	endif()

	#=== setup ===
	set( DOXY_HTML_DIR 	"reference_doc_html" )
	configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in
					${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )

	#=== target ===
	add_custom_target( api_doc ALL	${DOXYGEN_EXECUTABLE} Doxyfile )
	add_custom_target( images ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/images
			 ${CMAKE_CURRENT_BINARY_DIR}/${DOXY_HTML_DIR} )

	#=== install ===
	install(DIRECTORY		${CMAKE_CURRENT_BINARY_DIR}/${DOXY_HTML_DIR}
	       DESTINATION		${DOC_INSTALL_LOCATION} )
	install(FILES      		ReferenceManual.html
	       DESTINATION		${DOC_INSTALL_LOCATION} )

	#=== unset ===
	unset( DOXY_HAVE_DOT )
	unset( DOXY_HTML_DIR )

endif( DOXYGEN_FOUND )

#############################################################################