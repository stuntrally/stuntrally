#include "pch.h"
#include "joepack.h"

using std::string;
using std::map;
using std::ios_base;
#include <cassert>

#include "endian_utility.h"
#include "unittest.h"


bool JOEPACK::LoadPack(const string & fn)
{
	ClosePack();
	
	f.open(fn.c_str(), ios_base::in | ios_base::binary);
	
	if (f)
	{
		packpath = fn;
		
		//load header
		char * versioncstr = new char[versionstr.length() + 1];
		f.read(versioncstr, versionstr.length());
		versioncstr[versionstr.length()] = '\0';
		string fversionstr = versioncstr;
		delete [] versioncstr;
		if (fversionstr != versionstr)
		{
			//write out an error?
			return false;
		}
		
		unsigned int numobjs = 0;
		assert(sizeof(unsigned int) == 4);
		f.read((char*)(&numobjs), sizeof(unsigned int));
		numobjs = ENDIAN_SWAP_32(numobjs);
		
		//DPRINT(numobjs << " objects");
		
		unsigned int maxstrlen = 0;
		f.read((char*)(&maxstrlen), sizeof(unsigned int));
		maxstrlen = ENDIAN_SWAP_32(maxstrlen);
		
		//DPRINT(maxstrlen << " max string length");
		
		char * fnch = new char[maxstrlen+1];
		
		//load FAT
		for (unsigned int i = 0; i < numobjs; ++i)
		{
			JOEPACK_FADATA fa;
			f.read((char*)(&(fa.offset)), sizeof(unsigned int));
			fa.offset = ENDIAN_SWAP_32(fa.offset);
			f.read((char*)(&(fa.length)), sizeof(unsigned int));
			fa.length = ENDIAN_SWAP_32(fa.length);
			f.read(fnch, maxstrlen);
			fnch[maxstrlen] = '\0';
			string filename = fnch;
			fat[filename] = fa;
			
			//DPRINT(filename << ": offest " << fa.offset << " length " << fa.length);
		}
		
		delete [] fnch;
		return true;
	}
	else
	{
		//write an error?
		return false;
	}
}

void JOEPACK::ClosePack()
{
	Pack_fclose();
	fat.clear();
	if (f.is_open())
	{
		f.close();
	}
	packpath.clear();
	curfa = fat.end();
}

void JOEPACK::Pack_fclose()
{
	curfa = fat.end();
}

bool JOEPACK::Pack_fopen(string fn)
{
	if (fn.find(packpath, 0) < fn.length())
	{
		string newfn = fn.substr(packpath.length()+1);
		//DPRINT(fn << " -> " << newfn);
		fn = newfn;
	}
	
	//DPRINT("Opening " << fn << " by seeking to " << fat[fn].offset);
	
	//curfa = &(fat[fn]);
	curfa = fat.find(fn);
	
	if (curfa == fat.end())
	{
		//write an error?
		return false;
	}
	else
	{
		f.seekg(curfa->second.offset);
		return true;
	}
}

int JOEPACK::Pack_fread(void * buffer, const unsigned int size, const unsigned int count)
{
	if (curfa != fat.end())
	{
		unsigned int abspos = f.tellg();
		assert(abspos >= curfa->second.offset);
		unsigned int relpos = abspos - curfa->second.offset;
		//DPRINT("relpos: " << relpos);
		assert(curfa->second.length >= relpos);
		unsigned int fileleft = curfa->second.length - relpos;
		unsigned int requestedread = size*count;
		
		assert(size != 0);
		
		if (requestedread > fileleft)
		{
			//overflow
			requestedread = fileleft/size;
		}
		
		//DPRINT("JOEPACK fread: " << abspos << "," << relpos << "," << fileleft << "," << requestedread);
		f.read((char *)buffer, requestedread);
		return f.gcount()/size;
	}
	else
	{
		//write error?
		return 0;
	}
}

QT_TEST(joepack_test)
{
	JOEPACK p;
	QT_CHECK(p.LoadPack("data/test/test1.jpk"));
	QT_CHECK(p.Pack_fopen("testlist.txt"));
	char buf[1000];
	unsigned int chars = p.Pack_fread(buf, 1, 999);
	QT_CHECK_EQUAL(chars, 16);
	buf[chars] = '\0';
	string comparisonstr = "This is\na test.\n";
	string filestr = buf;
	QT_CHECK_EQUAL(buf,comparisonstr);
}
