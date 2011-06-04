#include "pch.h"
#include "optional.h"
#include "unittest.h"
#include "reseatable_reference.h"

QT_TEST(optional_test)
{
	class MYCLASS1
	{
		public:
			int data;
			MYCLASS1() : data(1337) {}
			optional <int> GetData(bool senddata)
			{
				if (senddata)
					return optional <int>(data);
				else
					return optional <int>();
			}
	};
	
	MYCLASS1 db1;
	QT_CHECK(db1.GetData(true));
	QT_CHECK_EQUAL(db1.GetData(true).get(), 1337);
	QT_CHECK(!db1.GetData(false));
	QT_CHECK_EQUAL(db1.GetData(false).get_or_default(1234), 1234);
	QT_CHECK_EQUAL(db1.GetData(true).get_or_default(1234), 1337);
	
	class MYCLASS2
	{
		public:
			int data;
			MYCLASS2() : data(1337) {}
			optional <int> GetData(bool senddata)
			{
				if (senddata)
					return data;
				else
					return optional <int>();
			}
			optional <reseatable_reference <int> > GetRef(bool senddata)
			{
				if (senddata)
					return optional <reseatable_reference <int> > (data);
				else
					return optional <reseatable_reference <int> > ();
			}
	};
	
	MYCLASS2 db2;
	QT_CHECK(db2.GetData(true));
	QT_CHECK_EQUAL(db2.GetData(true).get(), 1337);
	QT_CHECK(!db2.GetData(false));
	
	QT_CHECK(db2.GetRef(true));
	QT_CHECK_EQUAL(db2.GetRef(true).get().get(), 1337);
	QT_CHECK(!db2.GetRef(false));
}
