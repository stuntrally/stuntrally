#ifndef SH_EDITOR_H
#define SH_EDITOR_H

class QApplication;

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>


namespace boost
{
	class thread;
}

namespace sh
{
	class MainWindow;

	struct SynchronizationState
	{
		// Signals that Editor::update has been invoked.
		boost::condition_variable mMainConditionVariable;

		// Signals when a worker thread is finished.
		boost::condition_variable mWorkerConditionVariable;

		boost::mutex mUpdateMutex;

		boost::mutex mActionMutex;
		boost::mutex mQueryMutex;
	};

	class Editor
	{
	public:
		Editor();
		~Editor();

		void show();

		void update();

	private:
		bool mInitialized;

		MainWindow* mMainWindow;
		QApplication* mApplication;

		SynchronizationState mSync;

		boost::thread* mThread;

		void runThread();

		void processShowWindow();
	};

}

#endif
