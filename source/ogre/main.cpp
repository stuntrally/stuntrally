#include "pch.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//	#include "vld.h" // mem leaks +
#endif
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/logging.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"
#include "../network/enet-wrapper.hpp"

#include <string>
#include <sstream>
#include <list>
#include <ostream>

#include <OgrePlatform.h>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <locale.h>


//  load settings from default file
void LoadDefaultSet(SETTINGS* settings, std::string setFile)
{
	settings->Load(PATHMANAGER::GetGameConfigDir() + "/game-default.cfg");
	settings->Save(setFile);
	//  delete old keys.xml too
	if (boost::filesystem::exists(PATHMANAGER::GetUserConfigDir() + "/keys.xml"))
		boost::filesystem::remove(PATHMANAGER::GetUserConfigDir() + "/keys.xml");
}


void VprThread(App* pA)
{
	if (pA)
		pA->UpdThr();
}


//  . . . . . . . . . .  MAIN  . . . . . . . . . .
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{
	setlocale(LC_NUMERIC, "C");

	std::stringstream dummy;
	PATHMANAGER::Init(dummy, dummy);

	// Open the log file
	std::string logfilename = PATHMANAGER::GetLogDir() + "/log.txt";
	std::ofstream logfile(logfilename.c_str());
	if (!logfile)
	{
		std::cerr << "Couldn't open log file: " << logfilename << std::endl;
		return EXIT_FAILURE;
	}
	
	// Set up logging arrangement
	logging::splitterstreambuf infosplitter(std::cout, logfile);	std::ostream infosplitterstream(&infosplitter);
	logging::splitterstreambuf errorsplitter(std::cerr, logfile);	std::ostream errorsplitterstream(&errorsplitter);
	logging::logstreambuf infolog("INFO: ", infosplitterstream);	//logstreambuf infolog("INFO: ", logfile);
	logging::logstreambuf errorlog("ERROR: ", errorsplitterstream);

	// Primary logging ostreams
	std::ostream info_output(&infolog);
	std::ostream error_output(&errorlog);/**/

	// HACK: We initialize paths a second time now that we have the output streams
	PATHMANAGER::Init(info_output, error_output);

	// Initialize networking
	net::ENetContainer enet;

	///  Load Settings
	//----------------------------------------------------------------
	SETTINGS* settings = new SETTINGS();
	std::string setFile = PATHMANAGER::GetSettingsFile();
	
	if (!PATHMANAGER::FileExists(setFile))
	{
		info_output << "Settings not found - loading defaults." << std::endl;
		LoadDefaultSet(settings,setFile);
	}
	settings->Load(setFile);  // LOAD
	if (settings->version != SET_VER)  // loaded older, use default
	{
		info_output << "Settings found, but older version - loading defaults." << std::endl;
		LoadDefaultSet(settings,setFile);
		settings->Load(setFile);  // LOAD
	}
	
	// HACK: we initialize paths a second time now that we have the output streams
	PATHMANAGER::Init(info_output, error_output);


	///  Game start
	//----------------------------------------------------------------
	GAME* pGame = new GAME(info_output, error_output, settings);
	std::list <std::string> args;//(argv, argv + argc);
	pGame->Start(args);  //game.End();

	App* pApp = new App();
	pApp->pSet = settings;
	pApp->pGame = pGame;
	pGame->pOgreGame = pApp;

	try
	{
		if (settings->multi_thr > 0)
			boost::thread t(VprThread, pApp);

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
			std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
		#endif
	}

	info_output << "Exiting" << std::endl;
	delete pApp;
	delete pGame;
	delete settings;
	return 0;
}
