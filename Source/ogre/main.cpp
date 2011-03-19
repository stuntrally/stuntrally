#include "stdafx.h"
#include "../vdrift/game.h"
#include "OgreGame.h"

#include "../vdrift/logging.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"


DWORD WINAPI VprThread(LPVOID lpParam)
{ 
	App* pA = (App*)lpParam;
	if (pA)
		pA->UpdThr();
    return 0;
}

//int main(int argc, char* argv[])
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpCmdLine, int nCmdShow)
{	
	//  Load Settings
	PATHMANAGER paths;  std::stringstream dummy;
	paths.Init(dummy, dummy);
	string logfilename = paths.GetLogFile();
	SETTINGS settings;
	settings.Load(paths.GetSettingsFile());
	
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
	HANDLE hpr;

    ///  game  ------------------------------
    GAME* pGame = new GAME(info_output, error_output, &settings);
	std::list <string> args;//(argv, argv + argc);
	pGame->Start(args);  //game.End();

	App* pApp = new App();
	pApp->pSet = &settings;
	pApp->pGame = pGame;
	pGame->pOgreGame = pApp;

	try
	{
		if (settings.mult_thr > 0)  ///
			hpr = CreateThread(NULL,0,VprThread,(LPVOID)pApp,0,NULL);
		pApp->Run( settings.ogre_dialog || lpCmdLine[0]!=0 );  //Release change-
	}
	catch (Ogre::Exception& e)
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		#else
			std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
		#endif
	}

	if (settings.mult_thr > 0)  ///
		TerminateThread(hpr, 1);
	info_output << "Exiting" << std::endl;
	delete pApp;
	delete pGame;
	return 0;
}
