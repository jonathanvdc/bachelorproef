#============================================================================
# Copy all files from data/* to ${DATA_INSTALL_LOCATION}
#============================================================================
#install( DIRECTORY data   DESTINATION  ${DATA_INSTALL_LOCATION}  )
file(GLOB DEPLOY_FILES "data/*")
INSTALL( FILES ${DEPLOY_FILES} DESTINATION ${DATA_INSTALL_LOCATION} )


#============================================================================
# Copy all files from config/* to ${CONFIG_INSTALL_LOCATION}
#============================================================================
#install( DIRECTORY config   DESTINATION  ${CONFIG_INSTALL_LOCATION}  )
file(GLOB DEPLOY_FILES "config/*")
INSTALL( FILES ${DEPLOY_FILES} DESTINATION ${CONFIG_INSTALL_LOCATION} )

#============================================================================
# Add subdirs:
#============================================================================
add_subdirectory( lib )

#############################################################################