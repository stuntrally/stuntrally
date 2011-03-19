#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H

//#include <string>
//#include <map>
//#include <list>
//#include <iostream>

#include "bucketed_hashmap.h"


class CONFIGVARIABLE
{
public:
	CONFIGVARIABLE();
	CONFIGVARIABLE(const CONFIGVARIABLE & other) {CopyFrom(other);}
	CONFIGVARIABLE & operator=(const CONFIGVARIABLE & other) {return CopyFrom(other);}
	bool operator<(const CONFIGVARIABLE & other);
	
	CONFIGVARIABLE & CopyFrom(const CONFIGVARIABLE & other);

	std::string section;
	std::string name;

	const std::string GetFullName() const;

	std::string val_s;
	int val_i;
	float val_f;
	float val_v[3];
	bool val_b;

	CONFIGVARIABLE * next;

	void Set(std::string newval);

	void DebugPrint(std::ostream & out);

	std::string strLTrim(std::string instr);
	std::string strRTrim(std::string instr);
	std::string strTrim(std::string instr);
	std::string strLCase(std::string instr);

	bool written;
};

static bool operator<(const CONFIGVARIABLE & first, const CONFIGVARIABLE & other)
{
	//return first.GetFullName() < other.GetFullName();
	return (first.section + "." + first.name < other.section + "." + other.name);
}

class CONFIGFILE
{
private:
	std::string filename;
	bucketed_hashmap <std::string, CONFIGVARIABLE> variables;
	//std::map <std::string, CONFIGVARIABLE> variables;
	void Add(std::string & paramname, CONFIGVARIABLE & newvar);
	std::string Trim(std::string instr);
	void ProcessLine(std::string & cursection, std::string linestr);
	std::string Strip(std::string instr, char stripchar);
	std::string LCase(std::string instr);
	bool SUPPRESS_ERROR;

public:
	CONFIGFILE();
	CONFIGFILE(std::string fname);
	~CONFIGFILE();
	bool bFltFull;
	
	bool Load(std::string fname);
	bool Load(std::istream & f);
	void Clear();
	std::string LoadedFile() const {return filename;}
	
	//returns true if the param was found
	bool ClearParam(std::string param);
	
	//returns true if the param was found
	bool GetParam(std::string param, std::string & outvar) const;
	bool GetParam(std::string param, int & outvar) const;
	bool GetParam(std::string param, float & outvar) const;
	//bool GetParam(std::string param, double & outvar) const;
	bool GetParam(std::string param, float * outvar) const; //for float[3]
	bool GetParam(std::string param, bool & outvar) const;
	
	///a higher level helper function to read from a configfile or print out an error
	template <typename T>
	bool GetParam(const std::string & param, T & output, std::ostream & error_output) const
	{
		if (!GetParam(param, output))
		{
			error_output << "Couldn't get parameter \"" << param << "\" from file \"" << LoadedFile() << "\"" << std::endl;
			return false;
		}
		return true;
	}
	
	// read points from configfile section
	void GetPoints(const std::string & sectionname, const std::string & paramprefix, std::vector <std::pair <double, double> > & output_points);
	
	///a higher level helper function to read to/write from a configfile based on the passed directional boolean.
	///if set_direction is true, then the param will be set to value.
	///if set_direction is false, then value will be set to the param.
	template <typename T>
	bool GetSetParam(const std::string & param, T & value, bool set_direction)
	{
		if (set_direction)
			return SetParam(param, value);
		else
			return GetParam(param, value);
	}

	//always returns true at the moment
	bool SetParam(std::string param, std::string invar);
	bool SetParam(std::string param, int invar);
	bool SetParam(std::string param, float invar);
	//bool SetParam(std::string param, double invar);
	bool SetParam(std::string param, float * invar);
	bool SetParam(std::string param, bool invar);
	
	void GetSectionList(std::list <std::string> & sectionlistoutput) const;
	void GetParamList(std::list <std::string> & paramlistoutput) const {GetParamList(paramlistoutput, "");}
	void GetParamList(std::list <std::string> & paramlistoutput, std::string section) const; ///< returns param names only, not their sections
	
	void ChangeSectionName(std::string oldname, std::string newname);

	void DebugPrint(std::ostream & out);

	bool Write();
	bool Write(bool with_brackets);
	bool Write(bool with_brackets, std::string save_as);
	
	void SuppressError(bool newse) {SUPPRESS_ERROR = newse;}
	
	unsigned int GetNumParams() {return variables.GetTotalObjects();}
	//unsigned int GetNumParams() {return variables.size();}
	
	//CONFIGVARIABLE * GetHead() {return vars;}
};

#endif /* _CONFIGFILE_H */
