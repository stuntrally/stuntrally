#include "stdafx.h"
#include "OgreApp.h"
#include "../vdrift/pathmanager.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
#else
	int main(int argc, char *argv[])
#endif
{
	//  Load Settings
	PATHMANAGER::Init(std::cout, std::cerr);
	string setFile = PATHMANAGER::GetUserConfigDir() + "/editor.cfg";
	SETTINGS settings;
	if (!PATHMANAGER::FileExists(setFile)) {
		settings.Load(PATHMANAGER::GetGameConfigDir() + "/editor-default.cfg");
		settings.Save(setFile);
	}
	settings.Load(setFile);

	App* pApp = new App();
	pApp->pSet = &settings;
	//app.appThr.pSet = &settings;

	try
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pApp->Run( settings.ogre_dialog || lpCmdLine[0]!=0 );
#else
		pApp->Run( settings.ogre_dialog );
#endif
	}
	catch (Ogre::Exception& e)
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		#else
			std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
		#endif
	}

	pApp->SaveCam();
	delete pApp;
	settings.Save(setFile);
	return 0;
}
