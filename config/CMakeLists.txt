
# Install config files
file(GLOB CONFIG_FILES "*.cfg")
install(FILES ${CONFIG_FILES} DESTINATION ${SHARE_INSTALL}/config/)

