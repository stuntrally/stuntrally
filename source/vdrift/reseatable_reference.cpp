#include "pch.h"
#include "reseatable_reference.h"
#include "unittest.h"

QT_TEST(reseatable_reference_test)
{
	int i1 = 1337;
	
	reseatable_reference <int> r1;
	QT_CHECK(!r1);
	r1 = i1;
	QT_CHECK(r1);
	QT_CHECK_EQUAL(r1.get(), 1337);
	QT_CHECK_EQUAL(*r1, 1337);
	
	reseatable_reference <int> r2 = i1;
	QT_CHECK(r2);
	QT_CHECK_EQUAL(r2.get(), 1337);
	
	int & ref = i1;
	
	reseatable_reference <int> r3 = ref;
	QT_CHECK(r3);
	QT_CHECK_EQUAL(r3.get(), 1337);
}
