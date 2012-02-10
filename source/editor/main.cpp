#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../vdrift/pathmanager.h"
#include <locale.h>


//  load settings from default file
void LoadDefaultSet(SETTINGS* settings, std::string setFile)
{
	settings->Load(PATHMANAGER::GetGameConfigDir() + "/editor-default.cfg");
	settings->Save(setFile);
}


//  . . . . . . . . . .  MAIN  . . . . . . . . . .
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{
	setlocale(LC_NUMERIC, "C");

	PATHMANAGER::Init(std::cout, std::cerr);

	///  Load Settings
	//----------------------------------------------------------------
	SETTINGS* settings = new SETTINGS();
	std::string setFile = PATHMANAGER::GetEditorSetFile();

	if (!PATHMANAGER::FileExists(setFile))
	{
		std::cout << "Settings not found - loading defaults." << std::endl;
		LoadDefaultSet(settings,setFile);
	}
	settings->Load(setFile);  // LOAD
	if (settings->version != SET_VER)  // loaded older, use default
	{
		std::cout << "Settings found, but older version - loading defaults." << std::endl;
		LoadDefaultSet(settings,setFile);
		settings->Load(setFile);  // LOAD
	}


	//  Start
	//----------------------------------------------------------------
	App* pApp = new App();
	pApp->pSet = settings;  //app.appThr.pSet = &settings;

	try
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			pApp->Run( settings->ogre_dialog || lpCmdLine[0]!=0 );
		#else
			pApp->Run( settings->ogre_dialog );
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
	settings->Save(setFile);
	delete settings;
	return 0;
}
