
if(UNIX)
	install(FILES stuntrally.desktop sr-editor.desktop DESTINATION "share/applications/")
	install(FILES ../data/gui/stuntrally.png DESTINATION "share/icons/hicolor/512x512/apps/")
	install(FILES ../data/gui/sr-editor.png DESTINATION "share/icons/hicolor/512x512/apps/")
endif()

