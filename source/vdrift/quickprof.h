/************************************************************************
* QuickProf                                                             *
* https://quickprof.sourceforge.net                                      *
* Copyright (C) 2006-2008                                               *
* Tyler Streeter (https://www.tylerstreeter.net)                         *
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
*   (3) The zlib/libpng license that is included with this library in   *
*       the file license-zlib-libpng.txt.                               *
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* license-LGPL.txt, license-BSD.txt, and license-zlib-libpng.txt for    *
* more details.                                                         *
************************************************************************/

// Please visit the project website (https://quickprof.sourceforge.net) 
// for usage instructions.

#pragma once
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <map>
//#include <math.h>

#if defined(WIN32) || defined(_WIN32)
	#define USE_WINDOWS_TIMERS
	//#include <windows.h>
	//#include <time.h>
#else
	//#include <sys/time.h>
#endif

/// Use this macro to access the profiler singleton.  For example: 
/// PROFILER.init();
/// ...
/// PROFILER.beginBlock("foo");
/// foo();
/// PROFILER.endBlock("foo");
#define PROFILER quickprof::Profiler::instance()

/// The main namespace that contains everything.
namespace quickprof
{
	/// A simple data structure representing a single timed block 
	/// of code.
	struct ProfileBlock
	{
		ProfileBlock()
		{
			currentBlockStartMicroseconds = 0;
			currentCycleTotalMicroseconds = 0;
			avgCycleTotalMicroseconds = 0;
			totalMicroseconds = 0;
		}

		/// The starting time (in us) of the current block update.
		unsigned long long int currentBlockStartMicroseconds;

		/// The accumulated time (in us) spent in this block during the 
		/// current profiling cycle.
		unsigned long long int currentCycleTotalMicroseconds;

		/// The accumulated time (in us) spent in this block during the 
		/// past profiling cycle.
		double avgCycleTotalMicroseconds;

		/// The total accumulated time (in us) spent in this block.
		unsigned long long int totalMicroseconds;
	};

	/// A cross-platform clock class inspired by the Timer classes in 
	/// Ogre (https://www.ogre3d.org).
	class Clock
	{
	public:
		Clock()
		{
#ifdef USE_WINDOWS_TIMERS
			QueryPerformanceFrequency(&mClockFrequency);
#endif
			reset();
		}

		~Clock()
		{
		}

		/**
		Resets the initial reference time.
		*/
		void reset()
		{
#ifdef USE_WINDOWS_TIMERS
			QueryPerformanceCounter(&mStartTime);
			mStartTick = GetTickCount();
			mPrevClockCycles = 0;
#else
			gettimeofday(&mStartTime, NULL);
#endif
		}

		/**
		Returns the time in us since the last call to reset or since 
		the Clock was created.

		@return The requested time in microseconds.  Assuming 64-bit 
                integers are available, the return value is valid for 2^63 
                clock cycles (over 104 years w/ clock frequency 2.8 GHz).
		*/
		unsigned long long int getTimeMicroseconds()
		{
#ifdef USE_WINDOWS_TIMERS
			// Compute the number of elapsed clock cycles since the 
			// clock was created/reset.  Using 64-bit signed ints, this 
			// is valid for 2^63 clock cycles (over 104 years w/ clock 
			// frequency 2.8 GHz).
			LARGE_INTEGER currentTime;
			QueryPerformanceCounter(&currentTime);
			LONGLONG clockCycles = currentTime.QuadPart - 
				mStartTime.QuadPart;

			// Compute the total elapsed seconds.  This is valid for 2^63 
			// clock cycles (over 104 years w/ clock frequency 2.8 GHz).
			LONGLONG sec = clockCycles / mClockFrequency.QuadPart;

			// Check for unexpected leaps in the Win32 performance counter.  
			// (This is caused by unexpected data across the PCI to ISA 
			// bridge, aka south bridge.  See Microsoft KB274323.)  Avoid 
			// the problem with GetTickCount() wrapping to zero after 47 
			// days (because it uses 32-bit unsigned ints to represent 
			// milliseconds).
			LONGLONG msec1 = sec * 1000 + (clockCycles - sec * 
				mClockFrequency.QuadPart) * 1000 / mClockFrequency.QuadPart;
			DWORD tickCount = GetTickCount();
			if (tickCount < mStartTick)
			{
				mStartTick = tickCount;
			}
			LONGLONG msec2 = (LONGLONG)(tickCount - mStartTick);
			LONGLONG msecDiff = msec1 - msec2;
			if (msecDiff < -100 || msecDiff > 100)
			{
				// Adjust the starting time forwards.
				LONGLONG adjustment = (std::min)(msecDiff * 
					mClockFrequency.QuadPart / 1000, clockCycles - 
					mPrevClockCycles);
				mStartTime.QuadPart += adjustment;
				clockCycles -= adjustment;

				// Update the measured seconds with the adjustments.
				sec = clockCycles / mClockFrequency.QuadPart;
			}

			// Compute the milliseconds part.  This is always valid since 
			// it will never be greater than 1000000.
			LONGLONG usec = (clockCycles - sec * mClockFrequency.QuadPart) * 
				1000000 / mClockFrequency.QuadPart;

			// Store the current elapsed clock cycles for adjustments next 
			// time.
			mPrevClockCycles = clockCycles;

			// The return value here is valid for 2^63 clock cycles (over 
			// 104 years w/ clock frequency 2.8 GHz).
			return sec * 1000000 + usec;
#else
			// Assuming signed 32-bit integers for tv_sec and tv_usec, and 
			// casting the seconds difference to a 64-bit unsigned long long 
			// int, the return value here is valid for over 136 years.
			struct timeval currentTime;
			gettimeofday(&currentTime, NULL);
			return (unsigned long long int)(currentTime.tv_sec - 
				mStartTime.tv_sec) * 1000000 + (currentTime.tv_usec - 
				mStartTime.tv_usec);
#endif
		}

	private:
#ifdef USE_WINDOWS_TIMERS
		LARGE_INTEGER mClockFrequency;
		DWORD mStartTick;
		LONGLONG mPrevClockCycles;
		LARGE_INTEGER mStartTime;
#else
		struct timeval mStartTime;
#endif
	};

	/// A set of ways to represent timing results.
	enum TimeFormat
	{
		SECONDS,
		MILLISECONDS,
		MICROSECONDS,
		PERCENT
	};

	/// A singleton class that manages timing for a set of profiling blocks.
	class Profiler
	{
	public:
		/**
		Useful for creating multiple Profiler instances.

		Normally the Profiler class should be accessed only through the 
		singleton instance method, which provides global access to a single 
		static Profiler instance.  However, it is also possible to create 
		several local Profiler instances, if necessary.
		*/
		inline Profiler();

		inline ~Profiler();

		/**
		Accesses the singleton instance.

		@return The Profiler instance.
		*/
		inline static Profiler& instance();

		/**
		Initializes the profiler. 

		This must be called first.  If this is never called, the profiler 
		is effectively disabled, and all other functions will return 
		immediately.  This can be called more than once to re-initialize the 
		profiler, which erases all previous profiling block names and their 
		timing data.

		@param smoothing      The measured duration for each profile 
                              block can be averaged across multiple 
                              cycles, and this parameter defines the 
                              smoothness of this averaging process.
                              The higher the value, the smoother the 
                              resulting average durations will appear.  
                              Leaving it at zero will essentially 
                              disable the smoothing effect.  More 
                              specifically, this parameter is a time 
                              constant (defined in terms of cycles) that 
                              defines an exponentially-weighted moving 
                              average.  For example, a value of 4.0 
                              means the past four cycles will contribute 
                              63% of the current weighted average.  This 
                              value must be >= 0.
		@param outputFilename If defined, enables timing data to be 
                              printed to a data file for later analysis.
		@param printPeriod    Defines how often data is printed to the 
                              file, in number of profiling cycles.  For 
                              example, set this to 1 if you want data 
                              printed after each cycle, or 5 if you want 
                              it printed every 5 cycles.  It is a good 
                              idea to increase this if you don't want 
                              huge data files.  Keep in mind, however, 
                              that when you increase this, you might 
                              want to increase the smoothing 
                              parameter.  (A good heuristic is to set 
                              the smoothing parameter equal to the 
                              print period.)  This value must be >= 1.
		@param printFormat    Defines the format used when printing data 
                              to a file.
		*/
		inline void init(double smoothing=0.0, 
			const std::string outputFilename="", size_t printPeriod=1,
			TimeFormat printFormat=MILLISECONDS);

		/**
		Begins timing the named block of code.

		@param name The name of the block.
		*/
		inline void beginBlock(const std::string& name);

		/**
		Defines the end of the named timing block.

		@param name The name of the block.
		*/
		inline void endBlock(const std::string& name);

		/**
		Defines the end of a profiling cycle. 

		Use this regularly by calling it at the end of all timing blocks.  
		This is necessary for smoothing and for file output, but not if 
		you just want a total summary at the end of execution (i.e. from 
		getSummary).  This must not be called within a timing block.
		*/
		inline void endCycle();

		/**
		Returns the average time used in the named block per profiling cycle.
		
		If smoothing is disabled (see init), this returns the most recent 
		duration measurement.

		@param name   The name of the block.
		@param format The desired time format to use for the result.
		@return       The block's average duration per cycle.
		*/
		inline double getAvgDuration(const std::string& name, 
			TimeFormat format)const;

		/**
		Returns the total time spent in the named block since the profiler was 
		initialized.

		@param name   The name of the block.
		@param format The desired time format to use for the result.
		@return       The block total time.
		*/
		inline double getTotalDuration(const std::string& name, 
			TimeFormat format);

		/**
		Computes the elapsed time since the profiler was initialized.

		@param format The desired time format to use for the result.
		@return The elapsed time.
		*/
		inline double getTimeSinceInit(TimeFormat format);

		/**
		Returns a summary of total times in each block.

		@param format The desired time format to use for the results.
		@return       The timing summary as a string.
		*/
		inline std::string getSummary(TimeFormat format=PERCENT);
		inline std::string getAvgSummary(TimeFormat format=PERCENT);

	private:
		/**
		Returns everything to its initial state.

		Deallocates memory, closes output files, and resets all variables.  
		This is called when the profiler is re-initialized and when the 
		process exits.
		*/
		inline void destroy();

		/**
		Prints an error message to standard output.

		@param msg The string to print.
		*/
		inline void printError(const std::string& msg)const;

		/**
		Returns a named profile block.

		@param name The name of the block to return.
		@return     The named ProfileBlock, or NULL if it can't be found.
		*/
		inline ProfileBlock* getProfileBlock(const std::string& name)const;

		/**
		Returns the appropriate suffix string for the given time format.

		@return The suffix string.
		*/
		inline std::string getSuffixString(TimeFormat format)const;

		/// Determines whether the profiler is enabled.
		bool mEnabled;

		/// The clock used to time profile blocks.
		Clock mClock;

		/// The starting time (in us) of the current profiling cycle.
		unsigned long long int mCurrentCycleStartMicroseconds;

		/// The average profiling cycle duration (in us).  If smoothing is 
		/// disabled, this is the same as the duration of the most recent 
		/// cycle.
		double mAvgCycleDurationMicroseconds;

		/// Internal map of named profile blocks.
		std::map<std::string, ProfileBlock*> mProfileBlocks;

		/// The data output file used if this feature is enabled in init.
		std::ofstream mOutputFile;

		/// Tracks whether we have begun printing data to the output file.
		bool mFirstFileOutput;

		/// A pre-computed scalar used to update exponentially-weighted moving 
		/// averages.
		double mMovingAvgScalar;

		/// Determines how often (in number of profiling cycles) timing data 
		/// is printed to the output file.
		size_t mPrintPeriod;

		/// The time format used when printing timing data to the output file.
		TimeFormat mPrintFormat;

		/// Keeps track of how many cycles have elapsed (for printing).
		size_t mCycleCounter;

		/// Used to update the initial average cycle times.
		bool mFirstCycle;
	};

	Profiler::Profiler()
	{
		mEnabled = false;
		mCurrentCycleStartMicroseconds = 0;
		mAvgCycleDurationMicroseconds = 0;
		mFirstFileOutput = true;
		mMovingAvgScalar = 0;
		mPrintPeriod = 1;
		mPrintFormat = SECONDS;
		mCycleCounter = 0;
		mFirstCycle = true;
	}

	Profiler::~Profiler()
	{
		// This is called when the program exits because the singleton 
		// instance is static.

		destroy();
	}

	Profiler& Profiler::instance()
	{
		static Profiler self;
		return self;
	}

	void Profiler::destroy()
	{
		mEnabled = false;
		mClock.reset();
		mCurrentCycleStartMicroseconds = 0;
		mAvgCycleDurationMicroseconds = 0;
		while (!mProfileBlocks.empty())
		{
			delete (*mProfileBlocks.begin()).second;
			mProfileBlocks.erase(mProfileBlocks.begin());
		}
		if (mOutputFile.is_open())
		{
			mOutputFile.close();
		}
		mFirstFileOutput = true;
		mMovingAvgScalar = 0;
		mPrintPeriod = 1;
		mPrintFormat = SECONDS;
		mCycleCounter = 0;
		mFirstCycle = true;
	}

	void Profiler::init(double smoothing, const std::string outputFilename, 
		size_t printPeriod, TimeFormat printFormat)
	{
		if (mEnabled)
		{
			// Reset everything to its initial state and re-initialize.
			destroy();
			std::cout << "[QuickProf] Re-initializing profiler, " 
				<< "erasing all profiling blocks" << std::endl;
		}

		mEnabled = true;

		if (smoothing <= 0)
		{
			if (smoothing < 0)
			{
				printError("Smoothing parameter must be >= 0. Using 0.");
			}

			mMovingAvgScalar = 0;
		}
		else
		{
			// Treat smoothing as a time constant.
			mMovingAvgScalar = ::exp(-1 / smoothing);
		}

		if (!outputFilename.empty())
		{
			mOutputFile.open(outputFilename.c_str());
		}

		if (printPeriod < 1)
		{
			printError("Print period must be >= 1. Using 1.");
			mPrintPeriod = 1;
		}
		else
		{
			mPrintPeriod = printPeriod;
		}
		mPrintFormat = printFormat;

		mClock.reset();

		// Set the start time for the first cycle.
		mCurrentCycleStartMicroseconds = mClock.getTimeMicroseconds();
	}

	void Profiler::beginBlock(const std::string& name)
	{
		if (!mEnabled)
		{
			return;
		}

		if (name.empty())
		{
			printError("Cannot allow unnamed profile blocks.");
			return;
		}

		ProfileBlock* block = NULL;

		std::map<std::string, ProfileBlock*>::iterator iter = 
			mProfileBlocks.find(name);
		if (mProfileBlocks.end() == iter)
		{
			// The named block does not exist.  Create a new ProfileBlock.
			block = new ProfileBlock();
			mProfileBlocks[name] = block;
		}
		else
		{
			block = iter->second;
		}

		// We do this at the end to get more accurate results.
		block->currentBlockStartMicroseconds = mClock.getTimeMicroseconds();
	}

	void Profiler::endBlock(const std::string& name)
	{
		if (!mEnabled)
		{
			return;
		}

		// We do this at the beginning to get more accurate results.
		unsigned long long int endTick = mClock.getTimeMicroseconds();

		ProfileBlock* block = getProfileBlock(name);
		if (!block)
		{
			return;
		}

		unsigned long long int blockDuration = endTick - 
			block->currentBlockStartMicroseconds;
		block->currentCycleTotalMicroseconds += blockDuration;
		block->totalMicroseconds += blockDuration;
	}

	void Profiler::endCycle()
	{
		if (!mEnabled)
		{
			return;
		}

		// Update the average total cycle time.
		// On the first cycle we set the average cycle time equal to the 
		// measured cycle time.  This avoids having to ramp up the average 
		// from zero initially.
		unsigned long long int currentCycleDurationMicroseconds = 
			mClock.getTimeMicroseconds() - mCurrentCycleStartMicroseconds;
		if (mFirstCycle)
		{
			mAvgCycleDurationMicroseconds = 
				(double)currentCycleDurationMicroseconds;
		}
		else
		{
			mAvgCycleDurationMicroseconds = mMovingAvgScalar * 
				mAvgCycleDurationMicroseconds + (1 - mMovingAvgScalar) 
				* (double)currentCycleDurationMicroseconds;
		}

		// Update the average cycle time for each block.
		std::map<std::string, ProfileBlock*>::iterator blocksBegin = 
			mProfileBlocks.begin();
		std::map<std::string, ProfileBlock*>::iterator blocksEnd = 
			mProfileBlocks.end();
		std::map<std::string, ProfileBlock*>::iterator iter = blocksBegin;
		for (; iter != blocksEnd; ++iter)
		{
			ProfileBlock* block = iter->second;

			// On the first cycle we set the average cycle time equal to the 
			// measured cycle time.  This avoids having to ramp up the average 
			// from zero initially.
			if (mFirstCycle)
			{
				block->avgCycleTotalMicroseconds = 
					(double)block->currentCycleTotalMicroseconds;
			}
			else
			{
				block->avgCycleTotalMicroseconds = mMovingAvgScalar * 
					block->avgCycleTotalMicroseconds + (1 - mMovingAvgScalar) * 
					(double)block->currentCycleTotalMicroseconds;
			}

			block->currentCycleTotalMicroseconds = 0;
		}

		if (mFirstCycle)
		{
			mFirstCycle = false;
		}

		// If enough cycles have passed, print data to the output file.
		if (mOutputFile.is_open() && mCycleCounter % mPrintPeriod == 0)
		{
			mCycleCounter = 0;

			if (mFirstFileOutput)
			{
				// On the first iteration, print a header line that shows the 
				// names of each data column (i.e. profiling block names).
				mOutputFile << "# t(s)";

				std::string suffix = getSuffixString(mPrintFormat);
				for (iter = blocksBegin; iter != blocksEnd; ++iter)
				{
					mOutputFile  << " " << (*iter).first << "(" << suffix << ")";
				}

				mOutputFile << std::endl;
				mFirstFileOutput = false;
			}

			mOutputFile << getTimeSinceInit(SECONDS) * (double)0.000001;

			// Print the cycle time for each block.
			for (iter = blocksBegin; iter != blocksEnd; ++iter)
			{
				mOutputFile << " " << getAvgDuration((*iter).first, 
					mPrintFormat);
			}

			mOutputFile << std::endl;
		}

		++mCycleCounter;
		mCurrentCycleStartMicroseconds = mClock.getTimeMicroseconds();
	}

	double Profiler::getAvgDuration(const std::string& name, 
		TimeFormat format)const
	{
		if (!mEnabled)
		{
			return 0;
		}

		ProfileBlock* block = getProfileBlock(name);
		if (!block)
		{
			return 0;
		}

		double result = 0;

		switch(format)
		{
			case SECONDS:
				result = block->avgCycleTotalMicroseconds * (double)0.000001;
				break;
			case MILLISECONDS:
				result = block->avgCycleTotalMicroseconds * (double)0.001;
				break;
			case MICROSECONDS:
				result = block->avgCycleTotalMicroseconds;
				break;
			case PERCENT:
			{
				if (0 == mAvgCycleDurationMicroseconds)
				{
					result = 0;
				}
				else
				{
					result = 100.0 * block->avgCycleTotalMicroseconds / 
						mAvgCycleDurationMicroseconds;
				}
				break;
			}
			default:
				break;
		}

		return result;
	}

	double Profiler::getTotalDuration(const std::string& name, 
		TimeFormat format)
	{
		if (!mEnabled)
		{
			return 0;
		}

		ProfileBlock* block = getProfileBlock(name);
		if (!block)
		{
			return 0;
		}

		double blockTotalMicroseconds = (double)block->totalMicroseconds;
		double result = 0;

		switch(format)
		{
			case SECONDS:
				result = blockTotalMicroseconds * (double)0.000001;
				break;
			case MILLISECONDS:
				result = blockTotalMicroseconds * (double)0.001;
				break;
			case MICROSECONDS:
				result = blockTotalMicroseconds;
				break;
			case PERCENT:
				{
					double microsecondsSinceInit = getTimeSinceInit(MICROSECONDS);
					if (0 == microsecondsSinceInit)
					{
						result = 0;
					}
					else
					{
						result = 100.0 * blockTotalMicroseconds / 
							microsecondsSinceInit;
					}
				}
				break;
			default:
				break;
		}

		return result;
	}

	double Profiler::getTimeSinceInit(TimeFormat format)
	{
		double timeSinceInit = 0;

		switch(format)
		{
			case SECONDS:
				timeSinceInit = (double)mClock.getTimeMicroseconds() * (double)0.000001;
				break;
			case MILLISECONDS:
				timeSinceInit = (double)mClock.getTimeMicroseconds() * (double)0.001;
				break;
			case MICROSECONDS:
				timeSinceInit = (double)mClock.getTimeMicroseconds();
				break;
			case PERCENT:
			{
				timeSinceInit = 100;
				break;
			}
			default:
				break;
		}

		if (timeSinceInit < 0)
		{
			timeSinceInit = 0;
		}

		return timeSinceInit;
	}

	std::string Profiler::getSummary(TimeFormat format)
	{
		if (!mEnabled)
		{
			return "";
		}

		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2);  //
		std::string suffix = getSuffixString(format);

		std::map<std::string, ProfileBlock*>::iterator blocksBegin = 
			mProfileBlocks.begin();
		std::map<std::string, ProfileBlock*>::iterator blocksEnd = 
			mProfileBlocks.end();
		std::map<std::string, ProfileBlock*>::iterator iter = blocksBegin;
		for (; iter != blocksEnd; ++iter)
		{
			if (iter != blocksBegin)
			{
				oss << "\n";
			}

			oss << iter->first;
			oss << ": ";
			oss << getTotalDuration(iter->first, format);
			oss << " ";
			oss << suffix;
		}

		return oss.str();
	}
	
	std::string Profiler::getAvgSummary(TimeFormat format)
	{
		if (!mEnabled)
		{
			return "";
		}

		std::ostringstream oss;
		oss.precision(2);  //par
		std::string suffix = getSuffixString(format);

		std::map<std::string, ProfileBlock*>::iterator blocksBegin = 
				mProfileBlocks.begin();
		std::map<std::string, ProfileBlock*>::iterator blocksEnd = 
				mProfileBlocks.end();
		std::map<std::string, ProfileBlock*>::iterator iter = blocksBegin;
		for (; iter != blocksEnd; ++iter)
		{
			if (iter != blocksBegin)
			{
				oss << "\n";
			}

			oss << iter->first;
			oss << ": ";
			oss << getAvgDuration(iter->first, format);
			oss << " ";
			oss << suffix;
		}

		return oss.str();
	}

	void Profiler::printError(const std::string& msg)const
	{
		std::cout << "[QuickProf error] " << msg << std::endl;
	}

	ProfileBlock* Profiler::getProfileBlock(const std::string& name)const
	{
		std::map<std::string, ProfileBlock*>::const_iterator iter = 
			mProfileBlocks.find(name);
		if (mProfileBlocks.end() == iter)
		{
			// The named block does not exist.  Print an error.
			printError("The profile block named '" + name + 
				"' does not exist.");
			return NULL;
		}
		else
		{
			return iter->second;
		}
	}

	std::string Profiler::getSuffixString(TimeFormat format)const
	{
		std::string suffix;
		switch(format)
		{
			case SECONDS:
				suffix = "s";
				break;
			case MILLISECONDS:
				suffix = "ms";
				break;
			case MICROSECONDS:
				suffix = "us";
				break;
			case PERCENT:
			{
				suffix = "%";
				break;
			}
			default:
				break;
		}

		return suffix;
	}
};
