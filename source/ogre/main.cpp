#include "pch.h"
 //#include "vld.h" // mem leaks +
#include "CGame.h"
#include "../vdrift/game.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"
#include "../network/enet-wrapper.hpp"
#include <string>
#include <sstream>
#include <list>
#include <ostream>
#include <OgrePlatform.h>
#include <OgreStringConverter.h>
#include <OgreRoot.h>
#include <boost/filesystem.hpp>
#include <locale.h>
using namespace std;


//  load settings from default file
void LoadDefaultSet(SETTINGS* settings, string setFile)
{
	settings->Load(PATHMANAGER::GameConfigDir() + "/game-default.cfg");
	settings->Save(setFile);
	//  delete old keys.xml too
	string sKeys = PATHMANAGER::UserConfigDir() + "/keys.xml";
	if (boost::filesystem::exists(sKeys))
		boost::filesystem::rename(sKeys, PATHMANAGER::UserConfigDir() + "/keys_old.xml");
}


//  . . . . . . . . . .  MAIN  . . . . . . . . . .
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{
	Ogre::Timer ti;
	setlocale(LC_NUMERIC, "C");

	//  Paths
	PATHMANAGER::Init();


	//  redirect cerr
	streambuf* oldCout = cout.rdbuf(), *oldCerr = cerr.rdbuf();
	#if 0  //par
	string po = PATHMANAGER::UserConfigDir() + "/ogre.out";
	ofstream out(po.c_str());  cout.rdbuf(out.rdbuf());
	#endif
	string pa = PATHMANAGER::UserConfigDir() + "/ogre.err";
	ofstream oute(pa.c_str());  cerr.rdbuf(oute.rdbuf());


	//  Initialize networking
	net::ENetContainer enet;


	///  Load Settings
	//----------------------------------------------------------------
	SETTINGS* settings = new SETTINGS();
	string setFile = PATHMANAGER::SettingsFile();
	
	if (!PATHMANAGER::FileExists(setFile))
	{
		cerr << "Settings not found - loading defaults." << endl;
		LoadDefaultSet(settings,setFile);
	}
	settings->Load(setFile);  // LOAD
	if (settings->version != SET_VER)  // loaded older, use default
	{
		cerr << "Settings found, but older version - loading defaults." << endl;
		boost::filesystem::rename(setFile, PATHMANAGER::UserConfigDir() + "/game_old.cfg");
		LoadDefaultSet(settings,setFile);
		settings->Load(setFile);  // LOAD
	}
	
	
	//  Helper for testing networked games on 1 computer
	//  use number > 0 in command parameter,  adds it to nick, port and own ogre.log
	int num = -1;
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	if (lpCmdLine)
		num = Ogre::StringConverter::parseInt(lpCmdLine);
	#else
	if (argc > 1)
		num = Ogre::StringConverter::parseInt(argv[1]);
	#endif
	if (num > 0)
	{
		settings->net_local_plr = num;
		settings->local_port += num;
		settings->nickname += Ogre::StringConverter::toString(num);
	}
	

	//  Ogre Root for .log
	int net = settings->net_local_plr;
	Ogre::Root* root = OGRE_NEW Ogre::Root("", PATHMANAGER::UserConfigDir() + "/ogreset.cfg",
		PATHMANAGER::UserConfigDir() + "/ogre" + (net >= 0 ? toStr(net) : "") + ".log");

	LogO(Ogre::String("::: Time Init main: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
		LogO("System: Linux");
	#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		LogO("System: WinRT");
	#elif OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		LogO("System: Win32");
	#endif

	//  paths
	LogO(PATHMANAGER::info.str());


	///  Game start
	//----------------------------------------------------------------
	GAME* pGame = new GAME(settings);
	list <string> args;  //(argv, argv + argc);
	pGame->Start(args);

	App* pApp = new App(settings, pGame);
	pApp->mRoot = root;
	pGame->app = pApp;

	try
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			pApp->Run( settings->ogre_dialog || lpCmdLine[0]!=0 );  //Release change-
		#else
			pApp->Run( settings->ogre_dialog);
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

	delete pApp;
	delete pGame;
	delete settings;

	cout.rdbuf(oldCout);
	cerr.rdbuf(oldCerr);

	return 0;
}
