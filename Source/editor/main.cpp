#include "stdafx.h"
#include "OgreApp.h"


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
#else
	int main(int argc, char *argv[])
#endif
{
	//  Load Settings
	string setFile = "_editor.cfg";
	SETTINGS settings;
	settings.Load(setFile);

	App* pApp = new App();
	pApp->pSet = &settings;
	//app.appThr.pSet = &settings;

	try
	{
		pApp->Run( settings.ogre_dialog || lpCmdLine[0]!=0 );
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
