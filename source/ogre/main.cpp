#include "stdafx.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//	#include "vld.h" // mem leaks +
#endif
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/logging.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"

#include <OgrePlatform.h>
#include <boost/thread.hpp>

void VprThread(App* pA)
{
	if (pA)
		pA->UpdThr();
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{	
	//  Load Settings
	std::stringstream dummy;
	PATHMANAGER::Init(dummy, dummy);
	string logfilename = PATHMANAGER::GetLogDir() + "/log.txt";
	SETTINGS* settings = new SETTINGS();
	if (!PATHMANAGER::FileExists(PATHMANAGER::GetSettingsFile()))
	{
		settings->Load(PATHMANAGER::GetGameConfigDir() + "/game-default.cfg");
		settings->Save(PATHMANAGER::GetSettingsFile());
	}
	settings->Load(PATHMANAGER::GetSettingsFile());

	// open the log file
	std::ofstream logfile(logfilename.c_str());
	if (!logfile)
	{
		std::cerr << "Couldn't open log file: " << logfilename << std::endl;
		return EXIT_FAILURE;
	}
	
	// set up logging arrangement
	logging::splitterstreambuf infosplitter(std::cout, logfile);	std::ostream infosplitterstream(&infosplitter);
	logging::splitterstreambuf errorsplitter(std::cerr, logfile);	std::ostream errorsplitterstream(&errorsplitter);
	logging::logstreambuf infolog("INFO: ", infosplitterstream);	//logstreambuf infolog("INFO: ", logfile);
	logging::logstreambuf errorlog("ERROR: ", errorsplitterstream);

	// primary logging ostreams
	std::ostream info_output(&infolog);
	std::ostream error_output(&errorlog);/**/

	// HACK: we initialize paths a second time now that we have the output streams
	PATHMANAGER::Init(info_output, error_output);

	///  game  ------------------------------
	GAME* pGame = new GAME(info_output, error_output, settings);
	std::list <std::string> args;//(argv, argv + argc);
	pGame->Start(args);  //game.End();

	App* pApp = new App();
	pApp->pSet = settings;
	pApp->pGame = pGame;
	pGame->pOgreGame = pApp;

	try
	{
		if (settings->mult_thr > 0)
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
