# Install config files
file(GLOB CONFIG_FILES "*.cfg")
install(FILES ${CONFIG_FILES} DESTINATION ${SHARE_INSTALL}/config/)
file(GLOB XML_FILES "*.xml")
install(FILES ${XML_FILES} DESTINATION ${SHARE_INSTALL}/config/)
