#include "Editor.hpp"

#include <QApplication>
#include <QTimer>

#include <boost/thread.hpp>

#include "../Main/Factory.hpp"

#include "MainWindow.hpp"

namespace sh
{
	Editor::Editor()
		: mMainWindow(NULL)
		, mApplication(NULL)
		, mInitialized(false)
	{
	}

	Editor::~Editor()
	{
		mThread->join();
	}

	void Editor::show()
	{
		if (!mInitialized)
		{
			mInitialized = true;

			mThread = new boost::thread(boost::bind(&Editor::runThread, this));
		}
		else
		{
			if (mMainWindow)
				mMainWindow->mRequestShowWindow = true;
		}
	}

	void Editor::runThread()
	{
		int argc = 0;
		char** argv = NULL;
		mApplication = new QApplication(argc, argv);
		mApplication->setQuitOnLastWindowClosed(false);
		mMainWindow = new MainWindow();
		mMainWindow->mSync = &mSync;
		mMainWindow->show();

		mApplication->exec();

		delete mApplication;
	}

	void Editor::update()
	{
		if (!mMainWindow)
			return;


		{
			boost::mutex::scoped_lock lock(mSync.mActionMutex);

			// execute pending actions
			while (mMainWindow->mActionQueue.size())
			{
				Action* action = mMainWindow->mActionQueue.front();
				action->execute();
				delete action;
				mMainWindow->mActionQueue.pop();
			}
		}

		boost::mutex::scoped_lock lock2(mSync.mUpdateMutex);

		// update the list of materials
		mMainWindow->mMaterialList.clear();
		sh::Factory::getInstance().listMaterials(mMainWindow->mMaterialList);

		// update global settings
		mMainWindow->mGlobalSettingsMap.clear();
		sh::Factory::getInstance().listGlobalSettings(mMainWindow->mGlobalSettingsMap);
	}




}
