#============================================================================
# Copy the python script to the bin folder.
#============================================================================

INSTALL( FILES
         wrapper_stride.py
   	DESTINATION ${BIN_INSTALL_LOCATION}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ
	)

INSTALL( FILES
        create_contactmatrix.py
        interactive_maps.py
        log2csv.py
        plot_maps.py
   	DESTINATION ${LIB_INSTALL_LOCATION}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ
	)



#############################################################################
