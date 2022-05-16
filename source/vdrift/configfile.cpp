#include "pch.h"
//#include "Def_Str.h"

#include "configfile.h"
#include "unittest.h"

#include <fstream>
#include <sstream>
using namespace std;


CONFIGFILE::CONFIGFILE()
{
	filename = "";
	bFltFull = true;
}

CONFIGFILE::CONFIGFILE(string fname)
{
	Load(fname);
}

CONFIGFILE::~CONFIGFILE()
{
	Clear();
}

bool CONFIGFILE::GetParam(string param, int & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			++ppos;
			param = param.substr(ppos, param.length() - ppos);
	}	}
	
	const CONFIGVARIABLE* v = variables.Get(param);
	if (!v)
		return false;
	
	outvar = v->val_i;
	return true;
}

bool CONFIGFILE::GetParam(string param, bool & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			++ppos;
			param = param.substr(ppos, param.length() - ppos);
	}	}
	
	const CONFIGVARIABLE* v = variables.Get(param);
	if (!v)
		return false;
	
	outvar = v->val_b;
	return true;
}

bool CONFIGFILE::GetParam(string param, float & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			++ppos;
			param = param.substr(ppos, param.length() - ppos);
	}	}
	
	const CONFIGVARIABLE* v = variables.Get(param);
	if (!v)
		return false;
	
	outvar = v->val_f;
	return true;
}

bool CONFIGFILE::GetParam(string param, float * outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			++ppos;
			param = param.substr(ppos, param.length() - ppos);
	}	}
	
	const CONFIGVARIABLE* v = variables.Get(param);
	if (!v)
		return false;
	
	for (int i = 0; i < 3; ++i)
		outvar[i] = v->val_v[i];
	
	return true;
}

bool CONFIGFILE::GetParam(string param, string & outvar) const
{
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		if (param.substr(0, ppos).empty())
		{
			++ppos;
			param = param.substr(ppos, param.length() - ppos);
	}	}
	
	const CONFIGVARIABLE* v = variables.Get(param);
	if (!v)
		return false;
	
	outvar = v->val_s;
	return true;
}

void CONFIGFILE::GetPoints( const std::string & sectionname,
							const std::string & paramprefix,
							std::vector <std::pair <double, double> > & output_points)
{
	std::list <std::string> params;
	GetParamList(params, sectionname);
	for (auto i = params.begin(); i != params.end(); ++i)
	{
		if (i->find(paramprefix) == 0)
		{
			float point[3] = {0, 0, 0};
			if (GetParam(sectionname+"."+*i, point))
			{
				output_points.push_back(std::make_pair(point[0], point[1]));
			}
	}	}
}

void CONFIGFILE::Clear()
{
	filename.clear();
	variables.Clear();
}

void CONFIGFILE::Add(string & paramname, CONFIGVARIABLE & newvar)
{
	variables.Set(paramname, newvar);
}

CONFIGVARIABLE::CONFIGVARIABLE()
{
	val_s = "";
	val_i = 0;
	val_f = 0.f;
	val_b = false;
	int i;
	for (i = 0; i < 3; ++i)
		val_v[i] = 0.f;
	
	//next = NULL;
}

const string CONFIGVARIABLE::GetFullName() const
{
	string outstr = "";
	
	if (section != "")
		outstr = outstr + section + ".";
	outstr = outstr + name;
	
	return outstr;
}

void CONFIGVARIABLE::Set(string newval)
{
	newval = strTrim(newval);
	
	val_i = atoi(newval.c_str());
	val_f = atof(newval.c_str());
	val_s = newval;
	
	val_b = false;
	if (val_i == 0)		val_b = false;
	if (val_i == 1)		val_b = true;
	if (strLCase(newval) == "on")		val_b = true;
	if (strLCase(newval) == "off")		val_b = false;
	if (strLCase(newval) == "true")		val_b = true;
	if (strLCase(newval) == "false")	val_b = false;
	
	// now process as vector information
	int pos = 0;
	int arraypos = 0;
	string::size_type nextpos = newval.find(",", pos);
	string frag;
	
	while (nextpos < /*(int)*/ newval.length() && arraypos < 3)
	{
		frag = newval.substr(pos, nextpos - pos);
		val_v[arraypos] = atof(frag.c_str());
		
		pos = nextpos+1;
		++arraypos;
		nextpos = newval.find(",", pos);
	}
	
	// don't forget the very last one
	if (arraypos < 3)
	{
		frag = newval.substr(pos, newval.length() - pos);
		val_v[arraypos] = atof(frag.c_str());
	}
}

void CONFIGVARIABLE::DebugPrint(ostream & out)
{
	if (section != "")
		out << section << ".";
	out << name << endl;
	out << "string: " << val_s << endl;
	out << "int: " << val_i << endl;
	out << "float: " << val_f << endl;
	out << "vector: (" << val_v[0] << "," << val_v[1] << "," << val_v[2] << ")" << endl;
	out << "bool: " << val_b << endl;
	out << endl;
}

string CONFIGVARIABLE::strLTrim(string instr)
{
	return instr.erase(0, instr.find_first_not_of(" \t"));
}

string CONFIGVARIABLE::strRTrim(string instr)
{
	if (instr.find_last_not_of(" \t") != string::npos)
		return instr.erase(instr.find_last_not_of(" \t") + 1);
	else
		return instr;
}

string CONFIGVARIABLE::strTrim(string instr)
{
	return strLTrim(strRTrim(instr));
}

bool CONFIGFILE::Load(string fname)
{
	filename = fname;
	
	ifstream f;
	f.open(fname.c_str());
	
	if (!f)
	{
		//cout << "CONFIGFILE.Load: Couldn't find file:  " << fname << endl;
		return false;
	}	
	return Load(f);
}

bool CONFIGFILE::Load(std::istream & f)
{
	string cursection = "";
	const int MAXIMUMCHAR = 1024;
	char trashchar[MAXIMUMCHAR];
	
	while (f && !f.eof())
	{
		f.getline(trashchar, MAXIMUMCHAR, '\n');
		ProcessLine(cursection, trashchar);
	}
	return true;
}

string CONFIGFILE::Trim(string instr)
{
	CONFIGVARIABLE trimmer;
	string outstr = trimmer.strTrim(instr);
	return outstr;
}

void CONFIGFILE::ProcessLine(string & cursection, string linestr)
{
	linestr = Trim(linestr);
	linestr = Strip(linestr, '\r');
	linestr = Strip(linestr, '\n');
	
	//remove comments
	string::size_type commentpos = linestr.find("#", 0);
	if (commentpos < /*(int)*/ linestr.length())
	{
		linestr = linestr.substr(0, commentpos);
	}
	
	linestr = Trim(linestr);
	
	//only continue if not a blank line or comment-only line
	if (linestr.length() > 0)
	{
		if (linestr.find("=", 0) < linestr.length())
		{
			//find the name part
			string::size_type equalpos = linestr.find("=", 0);
			string name = linestr.substr(0, equalpos);
			++equalpos;
			string val = linestr.substr(equalpos, linestr.length() - equalpos);
			name = Trim(name);
			val = Trim(val);
			
			//only continue if valid
			if (name.length() > 0 && val.length() > 0)
			{
				CONFIGVARIABLE newvar;
				//newvar = new CONFIGVARIABLE;
				newvar.section = cursection;
				newvar.name = name;
				newvar.Set(val);
				
				string paramname = name;
				if (!cursection.empty())
					paramname = cursection + "." + paramname;
				
				Add(paramname, newvar);
			}
		}else
		{	//section header
			linestr = Strip(linestr, '[');
			linestr = Strip(linestr, ']');
			linestr = Trim(linestr);
			cursection = linestr;
		}
	}
}

string CONFIGFILE::Strip(string instr, char stripchar)
{
	string::size_type pos = 0;
	string outstr = "";
	int length = instr.length();

	while (pos < instr.length())
	{
		if (instr.c_str()[pos] == stripchar)
			break;
		//	outstr = outstr + instr.substr(pos, 1);
		++pos;
	}
	if (pos > 0)
		outstr = instr.substr(0, pos);

	if (pos+1 < length)
		outstr = outstr + instr.substr(pos+1, (length-pos)-1);

	return outstr;
}

void CONFIGFILE::DebugPrint(ostream & out)
{
	out << "*** " << filename << " ***" << endl << endl;
	
	std::list<CONFIGVARIABLE> vlist;
	
	for (auto i : variables)
		vlist.push_back(i);
	
	vlist.sort();
	
	for (auto& i : vlist)
		i.DebugPrint(out);
}

string CONFIGVARIABLE::strLCase(string instr)
{
	char tc[2];
	tc[1] = '\0';
	string outstr = "";
	
	string::size_type pos = 0;
	while (pos < instr.length())
	{
		if (instr.c_str()[pos] <= 90 && instr.c_str()[pos] >= 65)
		{
			tc[0] = instr.c_str()[pos] + 32;
			string tstr = tc;
			outstr = outstr + tc;
		}else
			outstr = outstr + instr.substr(pos, 1);
		
		++pos;
	}
	return outstr;
}

string CONFIGFILE::LCase(string instr)
{
	CONFIGVARIABLE lcaser;
	string outstr = lcaser.strLCase(instr);
	return outstr;
}

bool CONFIGFILE::SetParam(string param, int invar)
{
	char tc[256];
	sprintf(tc, "%i", invar);
	
	string tstr = tc;
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, bool invar)
{
	string tstr = "off";
	if (invar)
		tstr = "on";
	
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, float invar)
{
	char tc[256];
	sprintf(tc, "%f", invar);  //5.2f
	
	string tstr = tc;
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, float * invar)
{
	char tc[256];
	sprintf(tc, "%f,%f,%f", invar[0], invar[1], invar[2]);
	
	string tstr = tc;
	return SetParam(param, tstr);
}

bool CONFIGFILE::SetParam(string param, string invar)
{
	CONFIGVARIABLE newvar;
	
	newvar.name = param;
	newvar.section = "";
	string::size_type ppos;
	ppos = param.find(".", 0);
	if (ppos < param.length())
	{
		newvar.section = param.substr(0, ppos);
		++ppos;
		newvar.name = param.substr(ppos, param.length() - ppos);
	}
	
	newvar.Set(invar);
	
	Add(param, newvar);
	
	return true;
}

bool CONFIGFILE::Write(bool with_brackets)
{
	return Write(with_brackets, filename);
}

bool CONFIGFILE::Write(bool with_brackets, string save_as)
{
	ofstream f;
	f.open(save_as.c_str());
	
	if (!f)  return false;

	std::list <CONFIGVARIABLE> vlist;

	for (auto i : variables)
	{
		//cout << incsuccess << endl;
		// variables.IteratorGet()->DebugPrint();
		vlist.push_back(i);
	}

	vlist.sort();
	
	string cursection = "";
	for (auto cur : vlist)
	{
		if (cur.section == "")
		{
			f << cur.name << " = " << cur.val_s << endl;
		}else
		{
			if (cur.section != cursection)
			{
				f << endl;
				cursection = cur.section;
				
				if (with_brackets)
					f << "[ " << cur.section << " ]" << endl;
				else
					f << cur.section << endl;
			}
			f << cur.name << " = " << cur.val_s << endl;
		}
	}
	
	f.close();
	return true;
}

bool CONFIGFILE::Write()
{
	return Write(true);
}

bool CONFIGFILE::ClearParam(string param)
{
	return variables.Erase(param);
}

void CONFIGFILE::GetSectionList(std::list <string> & sectionlistoutput) const
{
	sectionlistoutput.clear();
	std::map <string, bool> templist;

	for (auto i : variables)
		templist[i.section] = true;
	
	for (auto i : templist)
		sectionlistoutput.push_back(i.first);
}

void CONFIGFILE::GetParamList(std::list <string> & paramlistoutput, string sel_section) const
{
	bool all = false;
	if (sel_section == "")
		all = true;
	
	paramlistoutput.clear();
	std::map <string, bool> templist;

	for (auto i : variables)
	{
		if (all)
			templist[i.section +"."+ i.name] = true;
		else
		if (i.section == sel_section)
			templist[i.name] = true;
	}

	for (auto i : templist)
	{
		paramlistoutput.push_back(i.first);
	}
}

CONFIGVARIABLE & CONFIGVARIABLE::CopyFrom(const CONFIGVARIABLE & other)
{
	section = other.section;
	name = other.name;
	val_s = other.val_s;
	val_i = other.val_i;
	val_f = other.val_f;
	val_b = other.val_b;
	
	for (int i = 0; i < 3; ++i)
		val_v[i] = other.val_v[i];
	
	return *this;
}

bool CONFIGVARIABLE::operator<(const CONFIGVARIABLE & other)
{
	return (section + "." + name < other.section + "." + other.name);
}

QT_TEST(configfile_test)
{
	std::stringstream instream;
	instream << "\n#comment on the FIRST LINE??\n\n"
		"variable outside of=a section\n\n"
		"test section numero UNO\n"
		"look at me = 23.4\n\n"
		"i'm so great=   BANANA\n"
		"#break!\n\n"
		"[ section    dos??]\n"
		"why won't you = breeeak #trying to break it\n\n"
		"what about ] # this malformed thing???\n"
		"nope works = fine.\n"
		"even vectors = 2.1,0.9,GAMMA\n"
		"this is a duplicate = 0\n"
		"this is a duplicate = 1\n"
		"random = intermediary\n"
		"this is a duplicate = 2\n";
	
	CONFIGFILE testconfig;
	testconfig.Load(instream);
	string tstr = "notfound";
	QT_CHECK(testconfig.GetParam("variable outside of", tstr));
	QT_CHECK_EQUAL(tstr, "a section");
	tstr = "notfound";
	QT_CHECK(testconfig.GetParam(".variable outside of", tstr));
	QT_CHECK_EQUAL(tstr, "a section");
	tstr = "notfound";
	QT_CHECK(testconfig.GetParam("section    dos??.why won't you", tstr));
	QT_CHECK_EQUAL(tstr, "breeeak");
	tstr = "notfound";
	QT_CHECK(testconfig.GetParam("variable outside of", tstr));
	QT_CHECK_EQUAL(tstr, "a section");
	tstr = "notfound";
	QT_CHECK(testconfig.GetParam(".variable outside of", tstr));
	QT_CHECK_EQUAL(tstr, "a section");
	tstr = "notfound";
	QT_CHECK(!testconfig.GetParam("nosection.novariable", tstr));
	QT_CHECK_EQUAL(tstr, "notfound");
	tstr = "notfound";
	float vec[3];
	QT_CHECK(testconfig.GetParam("what about.even vectors", vec));
	QT_CHECK_EQUAL(vec[0], 2.1f);
	QT_CHECK_EQUAL(vec[1], 0.9f);
	QT_CHECK_EQUAL(vec[2], 0.f);
	//testconfig.DebugPrint(std::cout);
	
	{
		std::list <string> slist;
		testconfig.GetSectionList(slist);
		slist.sort();
		auto i = slist.begin();
		QT_CHECK_EQUAL(*i, "");
		++i;
		QT_CHECK_EQUAL(*i, "section    dos??");
		++i;
		QT_CHECK_EQUAL(*i, "test section numero UNO");
		++i;
		QT_CHECK_EQUAL(*i, "what about");
		++i;
		QT_CHECK(i == slist.end());
	}
	{
		std::list <string> slist;
		testconfig.GetParamList(slist, "test section numero UNO");
		slist.sort();
		auto i = slist.begin();
		QT_CHECK_EQUAL(*i, "i'm so great");
		++i;
		QT_CHECK_EQUAL(*i, "look at me");
		++i;
		QT_CHECK(i == slist.end());
	}
}
