#ifndef _JOEPACK_H
#define _JOEPACK_H

//#include <string>
//#include <map>
//#include <fstream>

class JOEPACK_FADATA
{
public:
	JOEPACK_FADATA() {offset = 0; length = 0;}
	unsigned int offset;
	unsigned int length;
};

class JOEPACK
{
private:
	std::map <std::string, JOEPACK_FADATA> fat;
	std::ifstream f;
	std::map <std::string, JOEPACK_FADATA>::iterator curfa;
	std::string packpath;
	const std::string versionstr;
	
public:
	JOEPACK() : versionstr("JPK01.00") {curfa = fat.end();}
	~JOEPACK() {ClosePack();}
	
	bool LoadPack(const std::string & fn);
	void ClosePack();
	
	bool Pack_fopen(std::string fn);
	void Pack_fclose();
	int Pack_fread(void * buffer, const unsigned int size, const unsigned int count);
	
	const std::map <std::string, JOEPACK_FADATA> & GetFAT() const {return fat;}
};

#endif
