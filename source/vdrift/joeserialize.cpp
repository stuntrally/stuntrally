#include "pch.h"
#include "joeserialize.h"

#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <cstdio>

#include "unittest.h"
#include "..\ogre\OgreGame.h"

using std::cout;
using std::cerr;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::stringstream;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::vector;

using namespace joeserialize;

class TEST_VERTEX
{
	friend class joeserialize::Serializer;
	public:
		TEST_VERTEX() : x(0), y(0), z(0) {}
		TEST_VERTEX(float nx, float ny, float nz) : x(nx),y(ny),z(nz) {}
		float x,y,z;
		bool operator==(const TEST_VERTEX & other) {return (memcmp(this,&other,sizeof(TEST_VERTEX)) == 0);}
		bool operator!=(const TEST_VERTEX & other) {return !operator==(other);}

		bool Serialize(joeserialize::Serializer & s)
		{
			if (!s.Serialize("x", x)) return false;
			if (!s.Serialize("y", y)) return false;
			if (!s.Serialize("z", z)) return false;
			return true;
		}
};

ostream& operator <<(ostream &os,const TEST_VERTEX &v)
{
	os << v.x << "," << v.y << "," << v.z;
	return os;
}

//if you hate typing
#define _SERIALIZE_(ser,varname) if (!ser.Serialize(#varname,varname)) return false

//example complex class
class TEST_PLAYER
{
	friend class joeserialize::Serializer;
	//private:
	public: //the test needs access to these; otherwise they'd be private
		TEST_VERTEX position;
		unsigned int animframe;
		string name;
		string player_description;
		bool alive;
		bool aiming;
		std::list <int> simplelist;
		std::list <TEST_VERTEX> complexlist;
		std::map <std::string, TEST_VERTEX> mymap;
		std::vector <TEST_VERTEX> myvector;
		std::vector <TEST_VERTEX> myvector2;
		double curtime;
		float x;
		pair <string, int> somepair;

	public:
		string & GetName() {return name;}
		TEST_PLAYER(bool defaults)
		{
			if (defaults)
			{
				name = "Test player";
				position.x = position.y = position.z = 1.0;
				x = 1.337;
				animframe = 1337;
				simplelist.push_back(9);
				simplelist.push_back(8);
				complexlist.push_back(TEST_VERTEX(7,6,5));
				complexlist.push_back(TEST_VERTEX(4,3,2));
				complexlist.push_back(TEST_VERTEX(1,0,-1));
				
				mymap["testing123"] = TEST_VERTEX(1,2,3);
				mymap["testing654"] = TEST_VERTEX(6,5,4);

				myvector.push_back(TEST_VERTEX(5,4,3));
				myvector.push_back(TEST_VERTEX(2,1,0));
				
				myvector2.push_back(TEST_VERTEX(9,8,7));

				player_description = "Hello there.\nHow are you??";
				alive = true;
				aiming = false;

				curtime = 0.123456789;
				
				somepair.first = "string";
				somepair.second = 4321;
			}
		}

		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,position);
			_SERIALIZE_(s,animframe);
			_SERIALIZE_(s,name);
			_SERIALIZE_(s,simplelist);
			_SERIALIZE_(s,complexlist);
			_SERIALIZE_(s,myvector);
			_SERIALIZE_(s,player_description);
			_SERIALIZE_(s,alive);
			_SERIALIZE_(s,aiming);
			_SERIALIZE_(s,curtime);
			_SERIALIZE_(s,mymap);
			_SERIALIZE_(s,x);
			_SERIALIZE_(s,myvector2);
			_SERIALIZE_(s,somepair);
			return true;
		}
};

QT_TEST(TextSerializer_test)
{
	std::stringstream serializestream;
	{
		TEST_PLAYER player1(true);
		TextOutputSerializer out(serializestream);
		QT_CHECK(player1.Serialize(out));
	}
	
	//cout << "Text output follows:" << endl;
	//cout << serializestream.str() << endl;
	
	TEST_PLAYER player2(false); //our blank slate
	
	{
		TextInputSerializer in;
		in.set_error_output(cerr);
		QT_CHECK(in.Parse(serializestream));
		//ifstream filestream("player.txt");
		//QT_CHECK(in.Parse(filestream));
		QT_CHECK(player2.Serialize(in));
		
		//TextOutputSerializer out(cout);
		//player2.Serialize(out);
	}
	
	//do our checks
	QT_CHECK_EQUAL(player2.position,TEST_VERTEX(1,1,1));
	QT_CHECK_EQUAL(player2.animframe,1337);
	QT_CHECK_CLOSE(player2.x,1.337, 0.000001);
	QT_CHECK_EQUAL(player2.name,"Test player");

	//create comparison values
	std::list <int> local_simplelist;
	local_simplelist.push_back(9);
	local_simplelist.push_back(8);
	std::list <TEST_VERTEX> local_complexlist;
	local_complexlist.push_back(TEST_VERTEX(7,6,5));
	local_complexlist.push_back(TEST_VERTEX(4,3,2));
	local_complexlist.push_back(TEST_VERTEX(1,0,-1));

	QT_CHECK_EQUAL(player2.simplelist.size(),local_simplelist.size());

	{
		std::list <int>::iterator i;
		std::list <int>::iterator n;
		for (i = player2.simplelist.begin(),
			 n = local_simplelist.begin();
			 i != player2.simplelist.end() && n != local_simplelist.end(); i++,n++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.complexlist.size(),local_complexlist.size());

	{
		std::list <TEST_VERTEX>::iterator i;
		std::list <TEST_VERTEX>::iterator n;
		int count;
		for (i = player2.complexlist.begin(),
			 n = local_complexlist.begin(),
										 count = 0;
										 i != player2.complexlist.end() && n != local_complexlist.end(); i++,n++,count++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.mymap.size(), 2);
	QT_CHECK_EQUAL(player2.mymap["testing123"], TEST_VERTEX(1,2,3));
	QT_CHECK_EQUAL(player2.mymap["testing654"], TEST_VERTEX(6,5,4));

	QT_CHECK_EQUAL(player2.myvector.size(), 2);
	if (player2.myvector.size() == 2)
	{
		QT_CHECK_EQUAL(player2.myvector[0], TEST_VERTEX(5,4,3));
		QT_CHECK_EQUAL(player2.myvector[1], TEST_VERTEX(2,1,0));
	}
	
	QT_CHECK_EQUAL(player2.myvector2.size(), 1);
	if (player2.myvector2.size() == 1) QT_CHECK_EQUAL(player2.myvector2[0], TEST_VERTEX(9,8,7));

	QT_CHECK_EQUAL(player2.player_description, "Hello there.\nHow are you??");

	QT_CHECK_EQUAL(player2.alive, true);
	QT_CHECK_EQUAL(player2.aiming, false);

	QT_CHECK_EQUAL(player2.curtime, 0.123456789);
	
	QT_CHECK_EQUAL(player2.somepair.first, "string");
	QT_CHECK_EQUAL(player2.somepair.second, 4321);
}

QT_TEST(BinarySerializer_test)
{
	std::stringstream serializestream;
	{
		TEST_PLAYER player1(true);
		BinaryOutputSerializer out(serializestream);
		QT_CHECK(player1.Serialize(out));
		
		/*ofstream binarydata("data/test/test.bin");
		BinaryOutputSerializer fileout(binarydata);
		QT_CHECK(player1.Serialize(fileout));*/
	}
	
	//cout << "Text output follows:" << endl;
	//cout << serializestream.str() << endl;
	
	TEST_PLAYER player2(false); //our blank slate
	
	{
		BinaryInputSerializer in(serializestream);
		//ifstream filestream("player.txt");
		//QT_CHECK(in.Parse(filestream));
		QT_CHECK(player2.Serialize(in));
		
		//TextOutputSerializer out(cout);
		//player2.Serialize(out);
	}
	
	//do our checks
	QT_CHECK_EQUAL(player2.position,TEST_VERTEX(1,1,1));
	QT_CHECK_EQUAL(player2.animframe,1337);
	QT_CHECK_CLOSE(player2.x,1.337, 0.000001);
	QT_CHECK_EQUAL(player2.name,"Test player");

	//create comparison values
	std::list <int> local_simplelist;
	local_simplelist.push_back(9);
	local_simplelist.push_back(8);
	std::list <TEST_VERTEX> local_complexlist;
	local_complexlist.push_back(TEST_VERTEX(7,6,5));
	local_complexlist.push_back(TEST_VERTEX(4,3,2));
	local_complexlist.push_back(TEST_VERTEX(1,0,-1));

	QT_CHECK_EQUAL(player2.simplelist.size(),local_simplelist.size());

	{
		std::list <int>::iterator i;
		std::list <int>::iterator n;
		for (i = player2.simplelist.begin(),
			 n = local_simplelist.begin();
			 i != player2.simplelist.end() && n != local_simplelist.end(); i++,n++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.complexlist.size(),local_complexlist.size());

	{
		std::list <TEST_VERTEX>::iterator i;
		std::list <TEST_VERTEX>::iterator n;
		int count;
		for (i = player2.complexlist.begin(),
			 n = local_complexlist.begin(),
										 count = 0;
										 i != player2.complexlist.end() && n != local_complexlist.end(); i++,n++,count++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.mymap.size(), 2);
	QT_CHECK_EQUAL(player2.mymap["testing123"], TEST_VERTEX(1,2,3));
	QT_CHECK_EQUAL(player2.mymap["testing654"], TEST_VERTEX(6,5,4));

	QT_CHECK_EQUAL(player2.myvector.size(), 2);
	if (player2.myvector.size() == 2)
	{
		QT_CHECK_EQUAL(player2.myvector[0], TEST_VERTEX(5,4,3));
		QT_CHECK_EQUAL(player2.myvector[1], TEST_VERTEX(2,1,0));
	}
	
	QT_CHECK_EQUAL(player2.myvector2.size(), 1);
	if (player2.myvector2.size() == 1) QT_CHECK_EQUAL(player2.myvector2[0], TEST_VERTEX(9,8,7));

	QT_CHECK_EQUAL(player2.player_description, "Hello there.\nHow are you??");

	QT_CHECK_EQUAL(player2.alive, true);
	QT_CHECK_EQUAL(player2.aiming, false);

	QT_CHECK_EQUAL(player2.curtime, 0.123456789);
	
	QT_CHECK_EQUAL(player2.somepair.first, "string");
	QT_CHECK_EQUAL(player2.somepair.second, 4321);
}

QT_TEST(ReflectionSerializer_test)
{
	ReflectionSerializer reflection;
	reflection.set_error_output(cerr);
	
	{
		TEST_PLAYER player1(true);
		QT_CHECK(reflection.ReadFromObject(player1));
	}
	
	//reflection.Print(cout);
	
	//do reflection interface checks
	{
		float testfloat;
		int testint;
		std::string teststr;
		double testdouble;
		
		QT_CHECK(reflection.Get(reflection.Explode("position.x"), testfloat));
		QT_CHECK_EQUAL(testfloat, 1);
		
		QT_CHECK(reflection.Get(reflection.Explode("simplelist.size"), testint));
		QT_CHECK_EQUAL(testint, 2);
		
		QT_CHECK(reflection.Get(reflection.Explode("player_description"), teststr));
		QT_CHECK_EQUAL(teststr, "Hello there.\nHow are you??");
		
		QT_CHECK(reflection.Get(reflection.Explode("curtime"), testdouble));
		QT_CHECK_EQUAL(testdouble, 0.123456789);
		
		reflection.TurnOffErrorOutput();
		
		QT_CHECK(!reflection.Get(reflection.Explode(""), teststr));
		QT_CHECK(reflection.Get(reflection.Explode(".curtime"), teststr));
		QT_CHECK(reflection.Get(reflection.Explode("curtime."), teststr));
		QT_CHECK(!reflection.Get(reflection.Explode("invalid"), teststr));
		QT_CHECK(!reflection.Get(reflection.Explode("curtime. ."), teststr));
		QT_CHECK(reflection.Get(reflection.Explode("curtime"), teststr));
		
		QT_CHECK(!reflection.Set(reflection.Explode("curtime.oops"), 0.987654321));
		QT_CHECK(reflection.Set(reflection.Explode("curtime"), 0.987654321));
		QT_CHECK(reflection.Get(reflection.Explode("curtime"), testdouble));
		QT_CHECK_CLOSE(testdouble, 0.987654321, 0.000001);
		
		reflection.set_error_output(cerr);
	}
	
	TEST_PLAYER player2(false); //our blank slate
	{
		QT_CHECK(reflection.WriteToObject(player2));
	}
	
	//do our checks
	QT_CHECK_EQUAL(player2.position,TEST_VERTEX(1,1,1));
	QT_CHECK_EQUAL(player2.animframe,1337);
	QT_CHECK_CLOSE(player2.x,1.337, 0.000001);
	QT_CHECK_EQUAL(player2.name,"Test player");

	//create comparison values
	std::list <int> local_simplelist;
	local_simplelist.push_back(9);
	local_simplelist.push_back(8);
	std::list <TEST_VERTEX> local_complexlist;
	local_complexlist.push_back(TEST_VERTEX(7,6,5));
	local_complexlist.push_back(TEST_VERTEX(4,3,2));
	local_complexlist.push_back(TEST_VERTEX(1,0,-1));
	
	QT_CHECK_EQUAL ( player2.simplelist.size(),local_simplelist.size() );
	
	{
		std::list <int>::iterator i;
		std::list <int>::iterator n;
		for ( i = player2.simplelist.begin(),
		        n = local_simplelist.begin();
		        i != player2.simplelist.end() && n != local_simplelist.end(); i++,n++ )
		{
			QT_CHECK_EQUAL ( *i,*n );
		}
	}
	
	QT_CHECK_EQUAL ( player2.complexlist.size(),local_complexlist.size() );
	
	{
		std::list <TEST_VERTEX>::iterator i;
		std::list <TEST_VERTEX>::iterator n;
		int count;
		for ( i = player2.complexlist.begin(),
		        n = local_complexlist.begin(),
		        count = 0;
		        i != player2.complexlist.end() && n != local_complexlist.end(); i++,n++,count++ )
		{
			QT_CHECK_EQUAL ( *i,*n );
		}
	}
	
	QT_CHECK_EQUAL ( player2.mymap.size(), 2 );
	QT_CHECK_EQUAL ( player2.mymap["testing123"], TEST_VERTEX ( 1,2,3 ) );
	QT_CHECK_EQUAL ( player2.mymap["testing654"], TEST_VERTEX ( 6,5,4 ) );
	
	QT_CHECK_EQUAL ( player2.myvector.size(), 2 );
	if ( player2.myvector.size() == 2 )
	{
		QT_CHECK_EQUAL ( player2.myvector[0], TEST_VERTEX ( 5,4,3 ) );
		QT_CHECK_EQUAL ( player2.myvector[1], TEST_VERTEX ( 2,1,0 ) );
	}
	
	QT_CHECK_EQUAL(player2.myvector2.size(), 1);
	if (player2.myvector2.size() == 1) QT_CHECK_EQUAL(player2.myvector2[0], TEST_VERTEX(9,8,7));
	
	QT_CHECK_EQUAL ( player2.player_description, "Hello there.\nHow are you??" );
	
	QT_CHECK_EQUAL ( player2.alive, true );
	QT_CHECK_EQUAL ( player2.aiming, false );
	
	QT_CHECK_CLOSE(player2.curtime, 0.987654321, 0.000001); //note: this is the one we modified!
	
	QT_CHECK_EQUAL(player2.somepair.first, "string");
	QT_CHECK_EQUAL(player2.somepair.second, 4321);
}

//check for compatibility of the binary format
QT_TEST(serialization_test_bin_compatibility)
{
	TEST_PLAYER player2(false); //our blank slate

	//read input
	{
		ifstream teststream("data/test/test.bin",ifstream::binary);
		if (!teststream)
			teststream.open("build/data/test/test.bin");
		QT_CHECK(teststream);
		BinaryInputSerializer in(teststream);
		QT_CHECK(player2.Serialize(in));
	}

	//do our checks
	QT_CHECK_EQUAL(player2.position,TEST_VERTEX(1,1,1));
	QT_CHECK_EQUAL(player2.animframe,1337);
	QT_CHECK_CLOSE(player2.x,1.337, 0.000001);
	QT_CHECK_EQUAL(player2.name,"Test player");

	//create comparison values
	std::list <int> local_simplelist;
	local_simplelist.push_back(9);
	local_simplelist.push_back(8);
	std::list <TEST_VERTEX> local_complexlist;
	local_complexlist.push_back(TEST_VERTEX(7,6,5));
	local_complexlist.push_back(TEST_VERTEX(4,3,2));
	local_complexlist.push_back(TEST_VERTEX(1,0,-1));

	QT_CHECK_EQUAL(player2.simplelist.size(),local_simplelist.size());

	{
		std::list <int>::iterator i;
		std::list <int>::iterator n;
		for (i = player2.simplelist.begin(),
			 n = local_simplelist.begin();
			 i != player2.simplelist.end() && n != local_simplelist.end(); i++,n++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.complexlist.size(),local_complexlist.size());

	{
		std::list <TEST_VERTEX>::iterator i;
		std::list <TEST_VERTEX>::iterator n;
		int count;
		for (i = player2.complexlist.begin(),
			 n = local_complexlist.begin(),
										 count = 0;
										 i != player2.complexlist.end() && n != local_complexlist.end(); i++,n++,count++)
		{
			QT_CHECK_EQUAL(*i,*n);
		}
	}

	QT_CHECK_EQUAL(player2.mymap.size(), 2);
	QT_CHECK_EQUAL(player2.mymap["testing123"], TEST_VERTEX(1,2,3));
	QT_CHECK_EQUAL(player2.mymap["testing654"], TEST_VERTEX(6,5,4));

	QT_CHECK_EQUAL(player2.myvector.size(), 2);
	if (player2.myvector.size() == 2)
	{
		QT_CHECK_EQUAL(player2.myvector[0], TEST_VERTEX(5,4,3));
		QT_CHECK_EQUAL(player2.myvector[1], TEST_VERTEX(2,1,0));
	}
	
	QT_CHECK_EQUAL(player2.myvector2.size(), 1);
	if (player2.myvector2.size() == 1) QT_CHECK_EQUAL(player2.myvector2[0], TEST_VERTEX(9,8,7));

	QT_CHECK_EQUAL(player2.player_description, "Hello there.\nHow are you??");

	QT_CHECK_EQUAL(player2.alive, true);
	QT_CHECK_EQUAL(player2.aiming, false);

	QT_CHECK_EQUAL(player2.curtime, 0.123456789);
	
	QT_CHECK_EQUAL(player2.somepair.first, "string");
	QT_CHECK_EQUAL(player2.somepair.second, 4321);
}

class TEST_WHEEL
{
	friend class joeserialize::Serializer;
	public:
		TEST_VERTEX pos;
		float staticdata;
		
		TEST_WHEEL() : staticdata(0) {}

		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s, pos);
			return true;
		}
};

class TEST_CAR
{
	friend class joeserialize::Serializer;
	public:
		std::vector <TEST_WHEEL> wheels;
		TEST_CAR() : wheels(4) {}

		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s, wheels);
			return true;
		}
};

QT_TEST(serialization_vector_bugs_test)
{
	TEST_CAR car;
	car.wheels[0].staticdata = 1337;
	
	std::stringstream serializestream;
	{
		std::stringstream firststream;
		BinaryOutputSerializer out1(firststream);
		QT_CHECK(car.Serialize(out1));
		
		QT_CHECK_EQUAL(car.wheels.size(),4);
		QT_CHECK_EQUAL(car.wheels[0].staticdata,1337);
		
		BinaryOutputSerializer out2(serializestream);
		QT_CHECK(car.Serialize(out2));
		
		QT_CHECK_EQUAL(car.wheels.size(),4);
		QT_CHECK_EQUAL(car.wheels[0].staticdata,1337);
	}
	
	std::stringstream stream2(serializestream.str());

	{
		BinaryInputSerializer in1(serializestream);
		BinaryInputSerializer in2(stream2);

		QT_CHECK(car.Serialize(in1));
		
		QT_CHECK_EQUAL(car.wheels.size(),4);
		QT_CHECK_EQUAL(car.wheels[0].staticdata,1337);
		
		QT_CHECK(car.Serialize(in2));
		
		QT_CHECK_EQUAL(car.wheels.size(),4);
		QT_CHECK_EQUAL(car.wheels[0].staticdata,1337);
	}
}

class TEST_SETTINGS
{
	public:
		std::string datapath;
		std::string filename;
		
		TEST_SETTINGS() : datapath("data/"), filename(string("image")+DEFAULT_TEXTURE_EXTENSION) {}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s, datapath);
			_SERIALIZE_(s, filename);
			return true;
		}
};

QT_TEST(serialization_high_level_object_read_write_test)
{
	std::stringstream nullout;
	
	{
		TEST_SETTINGS settings;
		settings.datapath = "";
		settings.filename = "";
		joeserialize::LoadObjectFromFileOrCreateDefault("test.txt", settings, nullout);
		QT_CHECK_EQUAL(settings.datapath,"data/");
		QT_CHECK_EQUAL(settings.filename,string("image")+DEFAULT_TEXTURE_EXTENSION);
		joeserialize::WriteObjectToFile("test.txt", settings, nullout);
	}
	
	{
		ofstream f("test.txt");
		f << string("filename = customfile")+DEFAULT_TEXTURE_EXTENSION;
	}
	
	{
		TEST_SETTINGS settings;
		settings.datapath = "";
		settings.filename = "";
		joeserialize::LoadObjectFromFileOrCreateDefault("test.txt", settings, nullout);
		QT_CHECK_EQUAL(settings.datapath,"data/");
		QT_CHECK_EQUAL(settings.filename,string("customfile")+DEFAULT_TEXTURE_EXTENSION);
	}
	
	remove("test.txt");
}
