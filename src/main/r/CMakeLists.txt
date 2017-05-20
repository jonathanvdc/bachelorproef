#============================================================================
# Copy the R scripts to the lib folder.
#============================================================================

INSTALL( FILES
            plot_results_lib.R
			plot_social_contacts_lib.R

	DESTINATION ${LIB_INSTALL_LOCATION}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ
	)



#############################################################################