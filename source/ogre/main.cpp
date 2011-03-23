#include "stdafx.h"
//#include "vld.h" //+
#include "../vdrift/game.h"
#include "OgreGame.h"

#include "../vdrift/logging.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"

#include <OgrePlatform.h>

#include "boost/thread.hpp"

/* old win threads ** #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
DWORD WINAPI VprThread(LPVOID lpParam)
{ 
	App* pA = (App*)lpParam;
	if (pA)
		pA->UpdThr();
    return 0;
}
#endif*/
void VprThread(App* pA)
{
	if (pA)
		pA->UpdThr();
}

#if OGRE_PLATFORM  == OGRE_PLATFORM_WIN32
	int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
#else
	int main(int argc, char* argv[])
#endif
{	
	//  Enable run-time memory check for debug builds
	/*#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( //_CRTDBG_CHECK_CRT_DF |
			//_CRTDBG_CHECK_ALWAYS_DF | 
			//_CRTDBG_CHECK_EVERY_1024_DF | 
			_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif/**/

	//  Load Settings
	std::stringstream dummy;
	PATHMANAGER::Init(dummy, dummy);
	string logfilename = PATHMANAGER::GetLogFile();
	SETTINGS* settings = new SETTINGS();
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
/* old win threads ** #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	HANDLE hpr;
#endif*/

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
		if (settings->mult_thr > 0)  ///
		{
/* old win thread ** #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			hpr = CreateThread(NULL,0,VprThread,(LPVOID)pApp,0,NULL);
#endif*/
			boost::thread t(VprThread, pApp);
		}
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

	if (settings->mult_thr > 0)  ///
	{
/* old win thread ** #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		TerminateThread(hpr, 1);
#endif*/
		
	}
	info_output << "Exiting" << std::endl;
	delete pApp;
	delete pGame;
	delete settings;
	return 0;
}
