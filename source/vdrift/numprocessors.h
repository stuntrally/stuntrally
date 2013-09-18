#pragma once

namespace NUMPROCESSORS
{

#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
	|| defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__)
	#include <windows.h>
	#include <process.h>
#elif defined(__APPLE__) || defined (__FreeBSD__)
	//#include <pthread.h>

	// Required to get number of processors on OS X using sysctlbyname.
	#include <sys/sysctl.h>
#elif defined(unix) || defined(__unix) || defined(__unix__)
	//#include <pthread.h>

	// Required to get number of processors using get_nprocs_conf.
	#include <sys/sysinfo.h>
#else
	#error This development environment doesnt support pthreads or windows threads
#endif

	unsigned int GetNumProcessors()
	{
#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
		|| defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__)
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		return (unsigned int)systemInfo.dwNumberOfProcessors;
#elif defined (__APPLE__) || defined(__FreeBSD__)
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

}
