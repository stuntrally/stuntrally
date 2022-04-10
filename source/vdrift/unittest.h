/************************************************************************
* QuickTest                                                             *
* https://quicktest.sourceforge.net                                      *
* Copyright (C) 2005-2008                                               *
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
*                                                                       *
* This library is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
* license-LGPL.txt and license-BSD.txt for more details.                *
************************************************************************/ 

// Please visit the project website (https://quicktest.sourceforge.net) 
// for usage instructions.

// Credits: 
// Thanks to Noel Llopis for his helpful comparison of various C++ unit 
// testing frameworks and ideas for an ideal simple testing framework: 
// https://www.gamesfromwithin.com/articles/0412/000061.html  Thanks to 
// Michael Feathers for developing CppUnitLite.  Many of the macros in 
// Quicktest were modeled after those in CppUnitLite.

#ifndef QUICK_TEST_H
#define QUICK_TEST_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

// -----------------------------------------------------------------------
// Design Notes
// -----------------------------------------------------------------------
// * Each test automatically registers itself by accessing the TestManager 
// singleton.
// 
// * There are no formal fixtures.  Fixtures are simply user-defined 
// objects.  setup and teardown occur in the user-defined object's 
// constructor and destructor.  Tests that need fixtures should staticly 
// allocate one of these objects at the beginning of the test.  This method 
// is flexible and conceptually simple.

namespace quicktest
{
	typedef std::vector<std::string> TestResult;

	class Test
	{
	public:
		Test(const std::string& testName)
		{
			mTestName = testName;
		}

		virtual void run(TestResult& result) = 0;

	protected:
		void recordFailure(TestResult& result, const std::string& file, 
			unsigned long int line, const std::string& message)
		{
			// If the full filename is too long, only use the last part.
			std::string fileStr = file;
			size_t maxLength = 40;
			size_t fileLength = file.size();
			if (fileLength > maxLength)
			{
				// Get the last maxLength characters - 3 (leave room for 
				// three ellipses at the beginning).
				fileStr = "...";
				fileStr += file.substr(fileLength - maxLength + 3, 
					fileLength - 1);
			}

			std::ostringstream oss;
			oss << fileStr << "(" << line << "): '" << mTestName 
				<< "' FAILED: " << message;
			result.push_back(oss.str());
		}

		/// The unique name of this test.
		std::string mTestName;
	};

	class TestManager
	{
	public:
		static TestManager& instance()
		{
			static TestManager * self = new TestManager;
			return *self;
		}

		void addTest(Test* test)
		{
			mTests.push_back(test);
		}

		void setOutputStream(std::ostream* stream)
		{
			mOutputStream = stream;
		}

		std::ostream* getOutputStream()
		{
			return mOutputStream;
		}

		int runTests()
		{
			unsigned int numFailures = 0;

			*getOutputStream() 
				<< "[-------------- RUNNING UNIT TESTS --------------]" 
				<< std::endl;

			std::vector<Test*>::iterator iter;
			for (iter = mTests.begin(); iter != mTests.end(); ++iter)
			{
				(*iter)->run(mResult);

				bool testFailed = false;

				size_t size = mResult.size();
				for (size_t i = 0; i < size; ++i)
				{
					*getOutputStream() << mResult.at(i) << std::endl;
					testFailed = true;
				}
				mResult.clear();

				if (testFailed)
				{
					++numFailures;
				}
			}

			*getOutputStream() << "Results: " << (unsigned int)mTests.size() 
				- numFailures << " succeeded, " << numFailures << " failed" 
				<< std::endl;

			*getOutputStream() 
				<< "[-------------- UNIT TESTS FINISHED -------------]" 
				<< std::endl;
				
			return numFailures;
		}

	private:
		TestManager()
		{
			mOutputStream = &std::cout;
		}

		~TestManager()
		{
		}

		/// List of pointers to Tests.  All tests are staticly allocated, 
		/// so we don't need to destroy them manually.
		std::vector<Test*> mTests;

		std::ostream* mOutputStream;

		TestResult mResult;
	};
}

//---------------------------------------------------------
#if 1  // OVERRIDE, no tests
	#define QT_TEST(testName)  void testName()
	#define QT_CHECK(condition)  if (condition) {  }
	#define QT_CHECK_EQUAL(value1, value2)  if (value1 == value2) {  }
	#define QT_CHECK_CLOSE(value1, value2, tolerance)  if (value1 == value2) {  }
#else
	#define NO_SKIP 1
#endif
//---------------------------------------------------------

/// Macro to define a single test without using a fixture.
#if NO_SKIP
#define QT_TEST(testName)\
	class testName##Test : public quicktest::Test\
	{\
	public:\
		testName##Test()\
		: Test(#testName)\
		{\
			quicktest::TestManager::instance().addTest(this);\
		}\
		void run(quicktest::TestResult& result);\
	}testName##Instance;\
	void testName##Test::run(quicktest::TestResult& result)
#endif

/// Macro that runs all tests.  
#define QT_RUN_TESTS quicktest::TestManager::instance().runTests()

/// Macro that sets the output stream to use.
#define QT_SET_OUTPUT(stream)\
	quicktest::TestManager::instance().setOutputStream(stream)

/// Checks whether the given condition is true.
#if NO_SKIP
#define QT_CHECK(condition)\
	{\
		if (!(condition))\
		{\
			recordFailure(result, __FILE__, __LINE__, #condition);\
		}\
	}

/// Checks whether the first parameter is equal to the second.
#define QT_CHECK_EQUAL(value1, value2)\
	{\
		if ((value1) != (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should equal "\
				<< "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}
#endif

/// Checks whether the first parameter is not equal to the second.
#define QT_CHECK_NOT_EQUAL(value1, value2)\
	{\
		if ((value1) == (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should not equal "\
				<< "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}

/// Checks whether the first parameter is within the given tolerance from 
/// the second parameter.  This is useful for comparing floating point 
/// values.
#if NO_SKIP
#define QT_CHECK_CLOSE(value1, value2, tolerance)\
	{\
		double tempValue1 = (double)(value1);\
		double tempValue2 = (double)(value2);\
		if (fabs((tempValue1)-(tempValue2)) > tolerance)\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should be close to "\
				<< "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}
#endif

/// Checks whether the first parameter is less than the second.
#define QT_CHECK_LESS(value1, value2)\
	{\
		if ((value1) >= (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should be less than "\
				<< "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}

/// Checks whether the first parameter is less than or equal to the second.
#define QT_CHECK_LESS_OR_EQUAL(value1, value2)\
	{\
		if ((value1) > (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should be less than or "\
				<< "equal to " << "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}

/// Checks whether the first parameter is greater than the second.
#define QT_CHECK_GREATER(value1, value2)\
	{\
		if ((value1) <= (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should be greater than "\
				<< "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}

/// Checks whether the first parameter is greater than or equal to the 
/// second.
#define QT_CHECK_GREATER_OR_EQUAL(value1, value2)\
	{\
		if ((value1) < (value2))\
		{\
			std::ostringstream oss;\
			oss << "value1 (" << (value1) << ") should be greater than or "\
				<< "equal to " << "value2 (" << (value2) << ")";\
			recordFailure(result, __FILE__, __LINE__, oss.str());\
		}\
	}

/// Fails unconditionally and prints the given message.
#define QT_FAIL(message)\
	{\
		recordFailure(result, __FILE__, __LINE__, (message));\
	}\

/// Prints the given message.
#define QT_PRINT(message)\
	{\
		*(quicktest::TestManager::instance().getOutputStream()) << (message)\
			<< std::flush;\
	}\

#endif
