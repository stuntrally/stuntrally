#include "pch.h"
 //#include "vld.h" // mem leaks +
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "../vdrift/pathmanager.h"
#include <OgrePlatform.h>
#include <OgreRoot.h>
#include <locale.h>
#include <boost/filesystem.hpp>
using namespace std;


//  load settings from default file
void LoadDefaultSet(SETTINGS* settings, string setFile)
{
	settings->Load(PATHMANAGER::GameConfigDir() + "/editor-default.cfg");
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

	//  Paths
	PATHMANAGER::Init();


	//  redirect cerr
	streambuf* oldCout = cout.rdbuf(), *oldCerr = cerr.rdbuf();
	#if 0
    string po = PATHMANAGER::UserConfigDir() + "/ogre_ed.out";
    ofstream out(po.c_str());  cout.rdbuf(out.rdbuf());
    #endif
    string pa = PATHMANAGER::UserConfigDir() + "/ogre_ed.err";
    ofstream oute(pa.c_str());  cerr.rdbuf(oute.rdbuf());
	

	///  Load Settings
	//----------------------------------------------------------------
	SETTINGS* settings = new SETTINGS();
	string setFile = PATHMANAGER::EditorSetFile();

	if (!PATHMANAGER::FileExists(setFile))
	{
		cerr << "Settings not found - loading defaults." << endl;
		LoadDefaultSet(settings,setFile);
	}
	settings->Load(setFile);  // LOAD
	if (settings->version != SET_VER)  // loaded older, use default
	{
		cerr << "Settings found, but older version - loading defaults." << endl;
		boost::filesystem::rename(setFile, PATHMANAGER::UserConfigDir() + "/editor_old.cfg");
		LoadDefaultSet(settings,setFile);
		settings->Load(setFile);  // LOAD
	}
	

	//  Ogre Root for .log
	Ogre::Root* root = OGRE_NEW Ogre::Root("", PATHMANAGER::UserConfigDir() + "/ogreset_ed.cfg",
		PATHMANAGER::UserConfigDir() + "/ogre_ed.log");

	//  paths
	LogO(PATHMANAGER::info.str());


	//  Start
	//----------------------------------------------------------------
	App* pApp = new App(settings);
	pApp->mRoot = root;

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
			cerr << "An exception has occured: " << e.getFullDescription().c_str() << endl;
		#endif
	}

	pApp->SaveCam();
	delete pApp;
	
	settings->Save(setFile);
	delete settings;

	cout.rdbuf(oldCout);
	cerr.rdbuf(oldCerr);

	return 0;
}
