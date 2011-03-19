/************************************************************************
* QuickMP                                                               *
* http://quickmp.sourceforge.net                                        *
* Copyright (C) 2008                                                    *
* Tyler Streeter (http://www.tylerstreeter.net)                         *
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of EITHER:                                  *
*   (1) The GNU Lesser General Public License as published by the Free  *
*       Software Foundation; either version 2.1 of the License, or (at  *
*       your option) any later version. The text of the GNU Lesser      *
*       General Public License is included with this library in the     *
*       file license-LGPL.txt.                                          *
*   (2) The BSD-style license that is included with this library in     *
*       the file license-BSD.txt.                                       *
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* license-LGPL.txt and license-BSD.txt for more details.                *
************************************************************************/

#ifndef QUICK_MP_H
#define QUICK_MP_H

// QuickMP (Quick Multi-Processing) is a simple cross-platform C++ API for 
// generating parallel for loops in shared-memory programs, similar to 
// OpenMP.  It provides automatic scalable performance based on the number of 
// available processors.
// 
// Please visit the project website (http://quickprof.sourceforge.net) 
// for usage instructions.

// These macros generate a unique symbol name using the line number.  (We 
// must go through several helper macros to force full expansion of __LINE__.)  
// The resulting symbols will be unique within a given file.  Name collisions 
// with other files can be avoided as long as the files don't include one 
// another, or, if they do include one another, the symbols should be 
// declared within local scopes that aren't seen by the other file.
#define QMP_UNIQUE_SYMBOL_HELPER2(prefix, line) prefix##_uniqueSymbol##line
#define QMP_UNIQUE_SYMBOL_HELPER1(prefix, line) QMP_UNIQUE_SYMBOL_HELPER2(prefix, line)
#define QMP_UNIQUE_SYMBOL(prefix) QMP_UNIQUE_SYMBOL_HELPER1(prefix, __LINE__)

/// Defines the beginning of a parallel for loop.  The arguments are the 
/// name of the integer index variable (accessible within the loop), the 
/// starting value of the index, the number of iterations to perform, and 
/// (optionally) the schedule hint.  The index counts up from the starting 
/// value.  The valid schedule hints are: quickmp::SEQUENTIAL (default, 
/// better for equal-duration loop iterations; similar to OpenMP "static" 
/// schedule with default (equal) chunk size) and quickmp::INTERLEAVED 
/// (better for non-equal-duration loop iterations; similar to OpenMP 
/// "static" schedule with chunk size 1).
// Design notes: In order to create an arbitrary function containing the 
// user's for loop code (to be executed by the threads), we can define a 
// class to contain the function.  This class can be defined within any 
// other function.  The entire class method definition must be written 
// inline within the class definition (i.e. we can't define the function 
// at the end of a macro and let the user code write the {} brackets).  This 
// requires the use of two separate begin/end macros.  The instances of each 
// parallel section class should not go out of scope before they finish 
// executing.  We use a variadic macro here to allow an optional schedule hint 
// argument.
#define QMP_PARALLEL_FOR(indexName, loopFirstIndex, ...) \
{ \
	qmp_internal::ParallelTaskManager::instance().setLoopIndices( \
		loopFirstIndex, __VA_ARGS__); \
	static class QMP_UNIQUE_SYMBOL(ParallelTaskSubclass) : \
		public qmp_internal::ParallelTask \
	{ \
	public: \
		virtual void run(int QMP_UNIQUE_SYMBOL(parallelForLoopFirstIndex), \
			int QMP_UNIQUE_SYMBOL(parallelForLoopLastIndex), \
			const unsigned int parallelForLoopThreadIndexUniqueSymbol, \
			int QMP_UNIQUE_SYMBOL(parallelForLoopIndexIncrement)) \
		{ \
			for (int indexName = QMP_UNIQUE_SYMBOL(parallelForLoopFirstIndex); \
				indexName <= QMP_UNIQUE_SYMBOL(parallelForLoopLastIndex); \
				indexName += QMP_UNIQUE_SYMBOL(parallelForLoopIndexIncrement)) \
			{

/// Defines the end of a parallel for loop.
#define QMP_END_PARALLEL_FOR \
			} \
		} \
	}QMP_UNIQUE_SYMBOL(Instance); \
	qmp_internal::ParallelTaskManager::instance().process( \
		&QMP_UNIQUE_SYMBOL(Instance)); \
}

/// Specifies the number of threads to use in subsequent parallel for loops.  
/// This is optional; without calling this, the system will use one thread 
/// per processor.  If used, this must be called outside any parallel for 
/// loops.  This can be called any number of times.  This destroys and 
/// creates the internal thread pool, which might take time, so use it 
/// sparingly.
#define QMP_SET_NUM_THREADS(numThreads) \
	qmp_internal::ParallelTaskManager::instance().setNumThreads(numThreads)

/// Returns the number of threads currently being used.  In sequential 
/// code sections this returns 1; in parallel for loops this returns the 
/// total number of threads allocated for use in parallel for loops.
#define QMP_GET_NUM_THREADS \
	qmp_internal::ParallelTaskManager::instance().getNumThreads

/// Returns the total number of threads allocated for use in all parallel 
/// for loops.
#define QMP_GET_MAX_THREADS \
	qmp_internal::ParallelTaskManager::instance().getMaxThreads

/// The zero-based index of the current thread.  This is only valid within 
/// a parallel for loop code section.  Note: this is not a function call 
/// like most other macros (i.e. don't use () at the end).
#define QMP_THREAD_NUM parallelForLoopThreadIndexUniqueSymbol

/// Returns the number of processors in the current machine at runtime.
#define QMP_GET_NUM_PROCS \
	qmp_internal::ParallelTaskManager::instance().getNumProcessors

/// Returns true if called within a parallel for loop and false otherwise.
#define QMP_IN_PARALLEL \
	qmp_internal::ParallelTaskManager::instance().inParallel

/// Defines the beginning of a critical section used for synchronization.  
/// This is necessary to protect shared variables which are read and written 
/// by multiple threads.  The given id should be unique for each critical 
/// section within a parallel for loop.  Keep the ids low to avoid allocating 
/// too many internal critical sections.  Be very careful to use matching 
/// ids for the begin and end.
#define QMP_CRITICAL \
	qmp_internal::ParallelTaskManager::instance().criticalSectionBegin

/// Defines the beginning of a critical section used for synchronization.  
/// The given id must match the id given at the beginning of the critical 
/// section.  Keep the ids low to avoid allocating too many internal critical 
/// sections.  Be very careful to use matching ids for the begin and end.
#define QMP_END_CRITICAL \
	qmp_internal::ParallelTaskManager::instance().criticalSectionEnd

/// Defines a barrier routine used to synchronize threads.  Each thread blocks 
/// at the barrier until all threads have reached it.  This can be expensive 
/// and can often be avoided by splitting one parallel for loop into two.
#define QMP_BARRIER \
	qmp_internal::ParallelTaskManager::instance().barrier

/// Exposes the given variable to any parallel for loops later in the same 
/// scope.  The arguments are the variable's type and name.  This must be 
/// called outside the loop.  The variable must remain valid as long as they 
/// are being accessed by any loops.  Statically-allocated arrays must be 
/// given as pointers; for example, int myData[50] requires a pointer 
/// int* myDataPtr = myData, then QMP_SHARE(myDataPtr), not QMP_SHARE(myData).
// Design notes: Within the parallel tasks later we can access variables 
// outside the class definition only if they're static.  So we must make a 
// temporary static reference before the class definition.  We must be sure to 
// re-assign the static variable's value each time in case it changes, e.g., 
// if it's referring to a class variable, which would be different for each 
// class instance.  Statically-allocated arrays are problematic because the 
// address-of operator returns the same thing as the variable itself; we 
// could make a separate intermediate type-casted pointer variable, but we 
// don't want to do that for everything; solution: just have the user pass 
// in their own pointer.
#define QMP_SHARE(variableName) static void* variableName##_tempImportCopy = NULL; \
	variableName##_tempImportCopy = (void*)&variableName;

/// This provides access to the given variable within the parallel for loop, 
/// which must have been exposed before the beginning of the loop.  This must 
/// be called within the loop.  Statically-allocated arrays must be 
/// given as pointers; for example, int myData[50] requires a pointer 
/// int* myDataPtr = myData exposed via QMP_SHARE(myDataPtr) then accessed 
/// via QMP_USE_SHARED(int*, myDataPtr).
// Design notes: Here we make a reference to the temporary static reference, 
// which allows access to the original desired variable.  Also, this 2-step 
// process also allows us to maintain the same variable name as the original.  
// We use a variadic macro, which allows a variable number of arguments, to 
// handle types needing commas (e.g., std::map<int, int>) which would confuse 
// the macro preprocessor.
#define QMP_USE_SHARED(variableName, ...) __VA_ARGS__& variableName = \
	*((__VA_ARGS__*)variableName##_tempImportCopy);

/// A namespace for symbols that are part of the public API.
namespace quickmp
{
	/// Types of loop scheduling methods.
	enum ScheduleHint
	{
		/// This is the default.  It distributes loop iterations among threads 
		/// in large, equal chunks, similar to the OpenMP "static" scheduling 
		/// type with default (equal) chunk size.  This is better for loops 
		/// with equal-duration iterations.
		SEQUENTIAL, 

		/// Distributes loop iterations among threads in an interleaved 
		/// manner, similar to the OpenMP "static" scheduling type with 
		/// chunk size 1.  This is better for loops with non-equal-duration 
		/// iterations.
		INTERLEAVED
	};
}

/// A namespace for internal data structures.
namespace qmp_internal
{
	// Forward declaration.
	struct PlatformThreadObjects;

	/// A base class for parallel task classes which are defined by a set 
	/// of macros.
	class ParallelTask
	{
	public:
		/// The function which is executed by each thread with different 
		/// indices.  The last index is included.
		virtual void run(int firstIndex, int lastIndex, 
			const unsigned int threadIndex, int indexIncrement) = 0;
	};

	/// A singleton class to manage parallel code tasks.  This enables 
	/// automatic init on first use and destroy on exit.
	class ParallelTaskManager
	{
	public:
		/// Provides access to the singleton instance.
		inline static ParallelTaskManager& instance();

		/// Specifies the number of threads to use in subsequent parallel 
		/// for loops.  If not called explicitly by the user, this will be 
		/// called with the default value (zero), which uses one thread per 
		/// processor.  Can be called multiple times.
		inline void setNumThreads(unsigned int numThreads=0);

		/// Returns the number of threads currently being used.  In 
		/// sequential code sections this returns 1; in parallel for loops 
		/// this returns the total number of threads allocated for use in 
		/// parallel for loops.
		inline unsigned int getNumThreads()const;

		/// Returns the total number of threads allocated for use in all 
		/// parallel for loops.
		inline unsigned int getMaxThreads()const;

		/// Returns the number of processors in the current machine at runtime.
		inline unsigned int getNumProcessors()const;

		/// Returns true if called within a parallel for loop and false 
		/// otherwise.
		inline bool inParallel()const;

		/// Defines the range of the loop index.  Assumes the index begins 
		/// at the first index and counts up.  Internally, this sets the 
		/// loop indices to be used by each thread.
		inline void setLoopIndices(int loopFirstIndex, 
			unsigned int numIterations, quickmp::ScheduleHint scheduleHint);

		/// Separate version which is used when no schedule hint is supplied.
		inline void setLoopIndices(int loopFirstIndex, unsigned int numIterations);

		/// Unleashes the threads on the new task/loop.
		inline void process(ParallelTask* task);

		/// Called by individual threads to process a subset of the loop 
		/// iterations.
		inline void processSubset(unsigned int threadIndex);

		/// Defines the beginning of a critical section used for 
		/// synchronization.  This is necessary to protect shared variables 
		/// which are read and written by multiple threads.  The given id 
		/// should be unique for each critical section within a parallel for 
		/// loop.  Keep the ids low to avoid allocating too many internal 
		/// critical sections.
		inline void criticalSectionBegin(unsigned int id);

		/// Defines the end of a critical section used for synchronization.  
		/// The given id must match the id given at the beginning of the 
		/// critical section.  Keep the ids low to avoid allocating too many 
		/// internal critical sections.
		inline void criticalSectionEnd(unsigned int id);

		/// Defines a barrier routine used to synchronize threads.  Each 
		/// thread blocks at the barrier until all threads have reached it.
		inline void barrier();

		/// Provides access to the internal platform-specific data, like 
		/// thread handles and synchronization objects.  This gives access to 
		/// these things to the thread function.
		inline PlatformThreadObjects* getPlatformThreadObjects();

		/// Returns true if the main thread has requested the worker threads 
		/// to exit.
		inline bool shouldWorkerThreadsExit()const;

	private:
		inline ParallelTaskManager();

		inline ~ParallelTaskManager();

		/// Deallocates everything, closes threads, and returns the system 
		/// back to its uninitialized state.
		inline void destroy();

		PlatformThreadObjects* mPlatform;
		bool mInitialized;
		bool mInParallelSection;
		bool mShouldWorkerThreadsExit;
		ParallelTask* mCurrentTask;
		unsigned int mNumThreads;
		unsigned int mBarrierCount;
		int* mTaskFirstIndices;
		int* mTaskLastIndices;
		int mTaskIndexIncrement;
	};
}

//****************************************************************************
// If desired, this header file can be split into .h and .cpp files to speed 
// up compilation.  Simply move the remainder of this file (except the final 
// #endif) into a separate .cpp file, and add #include "quickmp.h" to the top.
//****************************************************************************

#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
	|| defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__)
	#define QMP_USE_WINDOWS_THREADS
	//#include <windows.h>
	//#include <process.h>
#elif defined(__APPLE__)
	//#include <pthread.h>

	// Required to get number of processors on OS X using sysctlbyname.
	//#include <sys/sysctl.h>
#elif defined(unix) || defined(__unix) || defined(__unix__)
	//#include <pthread.h>

	// Required to get number of processors using get_nprocs_conf.
	//#include <sys/sysinfo.h>
#else
	#error This development environment does not support pthreads or windows threads
#endif

//#include <iostream>
//#include <vector>

/// Assert macro.
#define QMP_ASSERT(condition)\
{\
	if (!(condition))\
	{\
		std::cout << "[QuickMP] Assertion failed in " << __FUNCTION__ \
			<< "(line " << __LINE__ << "): assert(" << #condition << ")" \
			<< std::endl;\
		::exit(1);\
	}\
}

namespace qmp_internal
{
	struct PlatformThreadObjects
	{
		PlatformThreadObjects()
		{
#ifdef QMP_USE_WINDOWS_THREADS
			barrierEventToggle = false;
			barrierEvent1 = NULL;
			barrierEvent2 = NULL;
			threadHandles = NULL;
			threadIDs = NULL;
#else
			threads = NULL;
#endif
		}

#ifdef QMP_USE_WINDOWS_THREADS
		// Windows condition variables are only available in Vista and later, 
		// so we must resort to using events for the barrier.

		CRITICAL_SECTION barrierCriticalSection;
		bool barrierEventToggle;
		HANDLE barrierEvent1;
		HANDLE barrierEvent2;
		CRITICAL_SECTION csVectorCriticalSection;
		std::vector<CRITICAL_SECTION*> userCriticalSections;
		HANDLE* threadHandles;
		DWORD* threadIDs;
#else
		pthread_t* threads;
		pthread_mutex_t barrierMutex;
		pthread_cond_t barrierCondition;
		pthread_mutex_t mutexVectorMutex;
		std::vector<pthread_mutex_t*> userMutexes;
#endif
	};

	/// The routine to be executed by the threads.  Note: Windows threads 
	/// require a __stdcall routine.
#ifdef QMP_USE_WINDOWS_THREADS
	inline unsigned __stdcall threadRoutine(void* threadIndex)
#else
	inline void* threadRoutine(void* threadIndex)
#endif
	{
		// Note: if the program crashes or data gets corrupted during thread 
		// execution, one possible cause is that we exceeded the default 
		// thread stack size.  Use thread functions to increase the stack size.

		// We cast to an unsigned long ints here because a void* on 64-bit 
		// machines is 64 bits long, and gcc won't cast a 64-bit void* 
		// directly to a 32-bit unsigned int.
		unsigned int myIndex = (unsigned int)((unsigned long int)threadIndex);

		// Loop until this thread is canceled by the main thread, which only 
		// occurs when the program exits.
		while (true)
		{
			// Between the barriers the main thread and worker threads are 
			// working on the loop iterations in parallel.  (Compare with 
			// ParallelTaskManager::process.)

			ParallelTaskManager::instance().barrier();

			if (ParallelTaskManager::instance().shouldWorkerThreadsExit())
			{
				// Exit the thread.
				break;
			}
			else
			{
				// Work on a subset of the loop.
				ParallelTaskManager::instance().processSubset(myIndex);
			}

			ParallelTaskManager::instance().barrier();
		}

#ifdef QMP_USE_WINDOWS_THREADS
		// _endthreadex is called automatically after start_address finishes.  
		// We could call it manually, but that causes C++ destructors pending 
		// in the thread not to be called.  In this case, though, we are 
		// having the main thread call TerminateThread on this thread when 
		// the program finishes.

		return 0;
#else
		// pthreads are terminated in several ways: implicitly when their 
		// start routine finishes, when they call pthread_exit (which might 
		// not let C++ destructors be called), when another thread calls 
		// pthread_cancel on them, or when the entire process is terminated.  
		// (If main() finishes normally, the pthreads will terminate, but if 
		// it finishes with pthread_exit(), the pthreads will continue to 
		// run.)  Here we are having the main thread call pthread_cancel on 
		// this thread when the program finishes.

		return NULL;
#endif
	}

	ParallelTaskManager& ParallelTaskManager::instance()
	{
		static ParallelTaskManager self;
		return self;
	}

	void ParallelTaskManager::setNumThreads(unsigned int numThreads)
	{
		if (mInitialized)
		{
			destroy();
		}

		if (0 == numThreads)
		{
			// By default, create one thread per processor.
			numThreads = getNumProcessors();
		}

		mNumThreads = numThreads;

		// We could either create n worker threads (and block the main thread 
		// while the workers are working), or use the main thread plus n-1 
		// workers.  Here we are doing the latter.

		QMP_ASSERT(numThreads > 0);
		unsigned int numWorkerThreads = numThreads - 1;

		mTaskFirstIndices = new int[numThreads];
		mTaskLastIndices = new int[numThreads];
		for (unsigned int i = 0; i < numThreads; ++i)
		{
			mTaskFirstIndices[i] = 0;
			mTaskLastIndices[i] = 0;
		}
		mTaskIndexIncrement = 0;

		if (numThreads > 1)
		{
#ifdef QMP_USE_WINDOWS_THREADS
			InitializeCriticalSection(&mPlatform->barrierCriticalSection);

			// Create the synchronization events.
			bool manualReset = true;
			bool startSignaled = false;
			mPlatform->barrierEvent1 = CreateEvent(NULL, manualReset, 
				startSignaled, NULL);
			mPlatform->barrierEvent2 = CreateEvent(NULL, manualReset, 
				startSignaled, NULL);

			InitializeCriticalSection(&mPlatform->csVectorCriticalSection);

			// Note: The Windows C runtime functions _beginthreadex/_endthreadex 
			// are preferred over the Windows API BeginThread/EndThread functions.  
			// See this: http://support.microsoft.com/default.aspx/kb/104641
			// Also, _beginthreadex is preferred over _beginthread because it 
			// is more flexible (and provides the same options as BeginThread).
			// Also, make sure to link with multithreaded runtime libraries 
			// (single threaded runtime libraries were actually removed starting 
			// in VC 2005).

			// uintptr_t _beginthreadex( 
			//	void* security, 
			//	unsigned stack_size, 
			//	unsigned (*start_address)(void*), 
			//	void* arglist, 
			//	unsigned initflag, 
			//	unsigned* thrdaddr);
			// 
			// Arguments: 
			// security: NULL means the returned thread handle cannot be 
			//           inherited by child processes
			// stack_size: 0 means use the same stack size as the main thread
			// start_address: __stdcall or __clrcall routine, returns exit code
			// arglist: arguments for start_address (or NULL)
			// initflag: 0 for running, CREATE_SUSPENDED for suspended
			// thrdaddr: receives thread ID, can be NULL if not needed
			// 
			// Return value: 
			// Handle to the new thread (or 0 on error), used for synchronization

			mPlatform->threadHandles = new HANDLE[numThreads];
			mPlatform->threadIDs = new DWORD[numThreads];
			// The main thread (index 0) handle is not used.
			mPlatform->threadHandles[0] = 0;
			mPlatform->threadIDs[0] = GetCurrentThreadId();
			for (unsigned int threadIndex = 1; threadIndex <= numWorkerThreads; ++threadIndex)
			{
				mPlatform->threadHandles[threadIndex] = 
					(HANDLE)_beginthreadex(NULL, 0, threadRoutine, 
					(void*)threadIndex, 0, (unsigned int*)&mPlatform->
					threadIDs[threadIndex]);
				QMP_ASSERT(0 != mPlatform->threadHandles[threadIndex])
			}
#else
			// Create synchronization objects.
			int returnCode = pthread_mutex_init(&mPlatform->barrierMutex, NULL);
			QMP_ASSERT(0 == returnCode);
			returnCode = pthread_cond_init(&mPlatform->barrierCondition, NULL);
			QMP_ASSERT(0 == returnCode);
			returnCode = pthread_mutex_init(&mPlatform->mutexVectorMutex, NULL);
			QMP_ASSERT(0 == returnCode);

			// int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
			//      void *(*start_routine)(void*), void* arg);
			//
			// Arguments:
			// thread: pthread_t pointer for later access
			// attr: thread attributes (NULL means use default attributes)
			// start_routine: C-style functor for function to be executed
			// arg: argument (void*) for start_routine
			// 
			// Return value: 
			// Return code (non-zero means an error occurred)

			pthread_attr_t threadAttributes;
			returnCode = pthread_attr_init(&threadAttributes);
			QMP_ASSERT(0 == returnCode);
			returnCode = pthread_attr_setdetachstate(&threadAttributes, 
				PTHREAD_CREATE_JOINABLE);
			QMP_ASSERT(0 == returnCode);

			mPlatform->threads = new pthread_t[numThreads];
			mPlatform->threads[0] = pthread_self();
			for (unsigned int threadIndex = 1; threadIndex <= numWorkerThreads; ++threadIndex)
			{
				returnCode = pthread_create(&mPlatform->threads[threadIndex], 
					&threadAttributes, threadRoutine, (void*)threadIndex);
				QMP_ASSERT(0 == returnCode);
			}

			returnCode = pthread_attr_destroy(&threadAttributes);
			QMP_ASSERT(0 == returnCode);
#endif
		}

		mInitialized = true;
	}

	unsigned int ParallelTaskManager::getNumThreads()const
	{
		if (mInParallelSection)
		{
			return mNumThreads;
		}
		else
		{
			return 1;
		}
	}

	unsigned int ParallelTaskManager::getMaxThreads()const
	{
		return mNumThreads;
	}

	unsigned int ParallelTaskManager::getNumProcessors()const
	{
#ifdef QMP_USE_WINDOWS_THREADS
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		return (unsigned int)systemInfo.dwNumberOfProcessors;
#elif defined (__APPLE__)
		int numProcessors = 0;
		size_t size = sizeof(numProcessors);
		int returnCode = sysctlbyname("hw.ncpu", &numProcessors, &size, NULL, 0);
		if (0 != returnCode)
		{
			std::cout << "[QuickMP] WARNING: Cannot determine number of " 
				<< "processors, defaulting to 1" << std::endl;
			return 1;
		}
		else
		{
			return (unsigned int)numProcessors;
		}
#else
		// Methods for getting the number of processors:

		// POSIX systems: sysconf() queries system info; the constants 
		// _SC_NPROCESSORS_CONF and _SC_NPROCESSORS_ONLN are provided on many 
		// systems, but they aren't part of the POSIX standard; they're not 
		// available on Mac OS X.
		
		// The GNU C library provides get_nprocs_conf() (number configured) 
		// and get_nprocs() (number available) in <sys/sysinfo.h>, but these 
		// are not available on Mac OS X.

		// In each of these methods there is a way to get the number of 
		// processors configured, which stays constant between reboots, and 
		// the number of processors online (capable of running processes), 
		// which can change during the lifetime of the calling process is 
		// the OS decides to disable some processors.

		// We'll just assume we have access to all processors.  (When setting 
		// the number of threads, we default to this value, but the user 
		// still has the option of setting any number of threads.)
		return (unsigned int)get_nprocs_conf();
#endif
	}

	bool ParallelTaskManager::inParallel()const
	{
		return mInParallelSection;
	}

	void ParallelTaskManager::setLoopIndices(int loopFirstIndex, 
		unsigned int numIterations, quickmp::ScheduleHint scheduleHint)
	{
		if (!mInitialized)
		{
			setNumThreads();
		}

		if (1 == mNumThreads)
		{
			mTaskFirstIndices[0] = loopFirstIndex;
			mTaskLastIndices[0] = loopFirstIndex + (int)numIterations - 1;
			mTaskIndexIncrement = 1;
			return;
		}

		// We divide up the iterations as equally as possible among the 
		// threads.  The difference between the heaviest and lightest loads 
		// should be no more than one iteration.

		switch(scheduleHint)
		{
			case quickmp::SEQUENTIAL:
			{
				// Similar to the OpenMP "static" scheduling type with default 
				// (equal) chunk size.  Using large, sequential chunks is 
				// good if all iterations require equal durations.  This 
				// reduces thread contention for overlapping memory locations 
				// because loops usually access sequential addresses.
				unsigned int numIterationsPerThread = numIterations / mNumThreads;
				unsigned int numRemainderIterations = numIterations % mNumThreads;
				int currentFirstIndex = loopFirstIndex;
				for (unsigned int i = 0; i < mNumThreads; ++i)
				{
					mTaskFirstIndices[i] = currentFirstIndex;

					// Distribute the remainder iterations.
					unsigned int numIterationsForThisThread = numIterationsPerThread;
					if (i < numRemainderIterations)
					{
						++numIterationsForThisThread;
					}

					// The last index represents the final iteration.
					mTaskLastIndices[i] = currentFirstIndex + 
						(int)numIterationsForThisThread - 1;
					currentFirstIndex = mTaskLastIndices[i] + 1;
				}
				mTaskIndexIncrement = 1;
				break;
			}
			case quickmp::INTERLEAVED:
				// Similar to the OpenMP "static" scheduling type with chunk 
				// size 1.  If the iterations use unequal durations, it is 
				// better to interleave the iterations.
				for (unsigned int i = 0; i < mNumThreads; ++i)
				{
					mTaskFirstIndices[i] = loopFirstIndex + i;
					mTaskLastIndices[i] = loopFirstIndex + numIterations - 1;
				}
				mTaskIndexIncrement = mNumThreads;
				break;
			default:
				break;
		}
	}

	void ParallelTaskManager::setLoopIndices(int loopFirstIndex, 
		unsigned int numIterations)
	{
		setLoopIndices(loopFirstIndex, numIterations, quickmp::SEQUENTIAL);
	}

	void ParallelTaskManager::process(ParallelTask* task)
	{
		mInParallelSection = true;
		QMP_ASSERT(!mCurrentTask);
		mCurrentTask = task;

		// Between the barriers the main thread and worker threads are 
		// working on the loop iterations in parallel.  (Compare with the 
		// thread routine.)

		barrier();

		// Work on a subset of the loop.
		processSubset(0);

		barrier();

		mCurrentTask = NULL;
		mInParallelSection = false;
	}

	void ParallelTaskManager::processSubset(unsigned int threadIndex)
	{
		mCurrentTask->run(mTaskFirstIndices[threadIndex], 
			mTaskLastIndices[threadIndex], threadIndex, mTaskIndexIncrement);
	}

	void ParallelTaskManager::criticalSectionBegin(unsigned int id)
	{
		// Avoid synchronization if there are no worker threads.
		if (mNumThreads < 2)
		{
			return;
		}

		// Dynamically allocate new synchronization objects on first usage 
		// up to the given id.

#ifdef QMP_USE_WINDOWS_THREADS
		if (id >= mPlatform->userCriticalSections.size())
		{
			// Protect against extra allocations by other threads.
			EnterCriticalSection(&mPlatform->csVectorCriticalSection);
			while (id >= mPlatform->userCriticalSections.size())
			{
				CRITICAL_SECTION* cs = new CRITICAL_SECTION;
				mPlatform->userCriticalSections.push_back(cs);
				InitializeCriticalSection(cs);
			}
			LeaveCriticalSection(&mPlatform->csVectorCriticalSection);
		}
		EnterCriticalSection(mPlatform->userCriticalSections[id]);
#else
		if (id >= mPlatform->userMutexes.size())
		{
			// Protect against extra allocations by other threads.
			int returnCode = pthread_mutex_lock(&mPlatform->mutexVectorMutex);
			QMP_ASSERT(0 == returnCode);
			while (id >= mPlatform->userMutexes.size())
			{
				pthread_mutex_t* mutex = new pthread_mutex_t;
				mPlatform->userMutexes.push_back(mutex);
				returnCode = pthread_mutex_init(mutex, NULL);
				QMP_ASSERT(0 == returnCode);

			}
			returnCode = pthread_mutex_unlock(&mPlatform->mutexVectorMutex);
			QMP_ASSERT(0 == returnCode);
		}
		int returnCode = pthread_mutex_lock(mPlatform->userMutexes[id]);
		QMP_ASSERT(0 == returnCode);
#endif
	}

	void ParallelTaskManager::criticalSectionEnd(unsigned int id)
	{
		// Avoid synchronization if there are no worker threads.
		if (mNumThreads < 2)
		{
			return;
		}

#ifdef QMP_USE_WINDOWS_THREADS
		if (id >= mPlatform->userCriticalSections.size())
		{
			std::cout << "[QuickMP] WARNING: Critical section 'end' (id=" 
				<< id << ") has no matching 'begin'" << std::endl;
		}
		else
		{
			LeaveCriticalSection(mPlatform->userCriticalSections[id]);
		}
#else
		if (id >= mPlatform->userMutexes.size())
		{
			std::cout << "[QuickMP] WARNING: Critical section 'end' (id=" 
				<< id << ") has no matching 'begin'" << std::endl;
		}
		else
		{
			int returnCode = pthread_mutex_unlock(mPlatform->userMutexes[id]);
			QMP_ASSERT(0 == returnCode);
		}
#endif
	}

	void ParallelTaskManager::barrier()
	{
		// Avoid synchronization if there are no worker threads.
		if (mNumThreads < 2)
		{
			return;
		}

		// Lock access to the shared variables.
#ifdef QMP_USE_WINDOWS_THREADS
		EnterCriticalSection(&mPlatform->barrierCriticalSection);
#else
		int returnCode = pthread_mutex_lock(&mPlatform->barrierMutex);
		QMP_ASSERT(0 == returnCode);
#endif

		++mBarrierCount;
		if (mBarrierCount == mNumThreads)
		{
			// All threads have reached the barrier.  First we must reset 
			// the count.  Then we signal all threads to continue.
			mBarrierCount = 0;

#ifdef QMP_USE_WINDOWS_THREADS
			// Use alternating events for every other barrier to avoid race 
			// conditions; otherwise, a fast thread might reach the next 
			// barrier and reset the event before the others get unblocked.
			if (mPlatform->barrierEventToggle)
			{
				SetEvent(mPlatform->barrierEvent1);
			}
			else
			{
				SetEvent(mPlatform->barrierEvent2);
			}
			mPlatform->barrierEventToggle = !mPlatform->barrierEventToggle;
			LeaveCriticalSection(&mPlatform->barrierCriticalSection);
#else
			// This must be called while the mutex is locked.  We must 
			// unlock the mutex afterwards in order to unblock the waiting 
			// threads.
			returnCode = pthread_cond_broadcast(&mPlatform->barrierCondition);
			QMP_ASSERT(0 == returnCode);
			returnCode = pthread_mutex_unlock(&mPlatform->barrierMutex);
			QMP_ASSERT(0 == returnCode);
#endif
		}
		else
		{
			// Wait until all threads have reached the barrier.

#ifdef QMP_USE_WINDOWS_THREADS
			// The first thread here must reset the event.
			if (1 == mBarrierCount)
			{
				if (mPlatform->barrierEventToggle)
				{
					ResetEvent(mPlatform->barrierEvent1);
				}
				else
				{
					ResetEvent(mPlatform->barrierEvent2);
				}
			}

			if (mPlatform->barrierEventToggle)
			{
				LeaveCriticalSection(&mPlatform->barrierCriticalSection);
				WaitForSingleObject(mPlatform->barrierEvent1, INFINITE);
			}
			else
			{
				LeaveCriticalSection(&mPlatform->barrierCriticalSection);
				WaitForSingleObject(mPlatform->barrierEvent2, INFINITE);
			}
#else
			// This must be called while the mutex is locked.  It unlocks 
			// the mutex while waiting and locks it again when finished.
			returnCode = pthread_cond_wait(&mPlatform->barrierCondition, 
				&mPlatform->barrierMutex);
			QMP_ASSERT(0 == returnCode);
			returnCode = pthread_mutex_unlock(&mPlatform->barrierMutex);
			QMP_ASSERT(0 == returnCode);
#endif
		}
	}

	PlatformThreadObjects* ParallelTaskManager::getPlatformThreadObjects()
	{
		return mPlatform;
	}

	bool ParallelTaskManager::shouldWorkerThreadsExit()const
	{
		return mShouldWorkerThreadsExit;
	}

	ParallelTaskManager::ParallelTaskManager()
	{
		mPlatform = new PlatformThreadObjects();
		mInitialized = false;
		mInParallelSection = false;
		mShouldWorkerThreadsExit = false;
		mCurrentTask = NULL;
		mNumThreads = 0;
		mBarrierCount = 0;
		mTaskFirstIndices = NULL;
		mTaskLastIndices = NULL;
		mTaskIndexIncrement = 0;
	}

	ParallelTaskManager::~ParallelTaskManager()
	{
		// This is called when the program exits because the singleton 
		// instance is static.

		destroy();
		delete mPlatform;
	}

	void ParallelTaskManager::destroy()
	{
		// If ::exit is called within a worker thread, things get a little 
		// messy: we can't be expected to close all the worker threads 
		// from within one of the worker threads because control would never 
		// return to the main thread.  We just have to quit without 
		// deallocating everything, which shouldn't be a big problem because 
		// the process is quitting anyway.
#ifdef QMP_USE_WINDOWS_THREADS
		if (mNumThreads > 1 && GetCurrentThreadId() != mPlatform->threadIDs[0])
#else
		if (mNumThreads > 1 && !pthread_equal(pthread_self(), mPlatform->threads[0]))
#endif
		{
			return;
		}

		// Clean up worker threads and synchronization objects.
		if (mNumThreads > 1)
		{
			// Signal the worker threads to exit, and wait until they're 
			// finished.  At this point all the worker threads are waiting 
			// at the first barrier.
			mShouldWorkerThreadsExit = true;
			barrier();

#ifdef QMP_USE_WINDOWS_THREADS
			// Wait for all thread handles to become signaled, indicating that 
			// the threads have exited.  Note: WaitForMultipleObjects would be 
			// ideal here, but it only supports up to MAXIMUM_WAIT_OBJECTS 
			// threads.
			for (unsigned int threadIndex = 1; threadIndex < mNumThreads; ++threadIndex)
			{
				DWORD returnCode = WaitForSingleObject(mPlatform->
					threadHandles[threadIndex], INFINITE);
				QMP_ASSERT(WAIT_OBJECT_0 == returnCode);
			}
#else
			// Call pthread_join on all worker threads, which blocks until the 
			// thread exits.
			for (unsigned int threadIndex = 1; threadIndex < mNumThreads; ++threadIndex)
			{
				int returnCode = pthread_join(mPlatform->threads[threadIndex], NULL);
				QMP_ASSERT(0 == returnCode);
			}
#endif

			// Clean up platform-specific objects, and return everything to its 
			// original state in case we're resetting the number of threads.
#ifdef QMP_USE_WINDOWS_THREADS
			DeleteCriticalSection(&mPlatform->barrierCriticalSection);

			mPlatform->barrierEventToggle = false;

			BOOL returnCode2 = CloseHandle(mPlatform->barrierEvent1);
			QMP_ASSERT(0 != returnCode2);
			mPlatform->barrierEvent1 = NULL;

			returnCode2 = CloseHandle(mPlatform->barrierEvent2);
			QMP_ASSERT(0 != returnCode2);
			mPlatform->barrierEvent2 = NULL;

			DeleteCriticalSection(&mPlatform->csVectorCriticalSection);

			// The main thread (index 0) handle is not used.
			for (unsigned int threadIndex = 1; threadIndex < mNumThreads; ++threadIndex)
			{
				int returnCode = CloseHandle(mPlatform->
					threadHandles[threadIndex]);
				QMP_ASSERT(0 != returnCode);
			}
			delete [] mPlatform->threadHandles;
			mPlatform->threadHandles = NULL;

			delete [] mPlatform->threadIDs;
			mPlatform->threadIDs = NULL;

			while (!mPlatform->userCriticalSections.empty())
			{
				DeleteCriticalSection(mPlatform->userCriticalSections.back());
				delete mPlatform->userCriticalSections.back();
				mPlatform->userCriticalSections.pop_back();
			}
#else
			delete mPlatform->threads;
			mPlatform->threads = NULL;

			int returnCode = pthread_mutex_destroy(&mPlatform->barrierMutex);
			QMP_ASSERT(0 == returnCode);

			returnCode = pthread_cond_destroy(&mPlatform->barrierCondition);
			QMP_ASSERT(0 == returnCode);

			returnCode = pthread_mutex_destroy(&mPlatform->mutexVectorMutex);
			QMP_ASSERT(0 == returnCode);

			while (!mPlatform->userMutexes.empty())
			{
				int returnCode = pthread_mutex_destroy(mPlatform->userMutexes.back());
				QMP_ASSERT(0 == returnCode);
				delete mPlatform->userMutexes.back();
				mPlatform->userMutexes.pop_back();
			}
#endif
		}

		mInitialized = false;
		mInParallelSection = false;
		mShouldWorkerThreadsExit = false;
		mCurrentTask = NULL;
		mNumThreads = 0;
		mBarrierCount = 0;

		if (mTaskFirstIndices)
		{
			delete [] mTaskFirstIndices;
			mTaskFirstIndices = NULL;
		}
		if (mTaskLastIndices)
		{
			delete [] mTaskLastIndices;
			mTaskLastIndices = NULL;
		}

		mTaskIndexIncrement = 0;
	}
}

#endif
