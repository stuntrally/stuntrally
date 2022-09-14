#include "pch.h"
#include <algorithm>

using namespace std;
namespace bfs = boost::filesystem;
typedef vector<string> vecstr;

///---- INFO
//  This is a tool program that generates sr.pot for translations
//  It reads tags (strings) from *en_tag.xml
//  and searches for their references in .cpp sources and
//  in gui .layout (finds also widget hierarchy)
//
//  Search for  [Setup]  to setup paths and params
//  look for  //par  for tweaking params
//  and  //test  for adding more info in log
//-----


///  single string to translate, from xml tag
//---------------------------------------------------
struct Tag
{
	//  <!--  last comment before tag (group name or such)  -->
	string cmt;

	//  <Tag name="name">text</Tag>
	//  actual string (tag) and its english text
	string name, text;
	
	//  occurrences of string
	//  in gui .layout files
	string gui;
	//  in source files .cpp
	string src;
};
//---------------------------------------------------


//  in a xml line, get value of attribute
//  e.g. for line = "<Tag name="aa" />" and attr = "name" it will return "aa"
string GetAttr(const string& line, const string& attr, const string& end="\"")
{
	string ss;
	string::size_type p = line.find(attr), p2;
	if (p != string::npos)
	{	p += attr.length();  // len
		
		p2 = line.find(end, p);  // attr val
		if (p2 != string::npos)
			ss = line.substr(p, p2-p);
	}
	return ss;
}

//  string utils
inline bool found(const string& s, const string& ss)
{
	 return s.find(ss) != string::npos;
}

bool IsAlpha(const string& s)
{
	const static char* aA0 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	return s.find_first_of(aA0) != string::npos;
}

//  trim spaces and tabs from front and back
void Trim(string& s)
{
	string::size_type p1 = s.find_first_not_of("\t "), p2 = s.find_last_not_of("\t ");
	if (p1 != string::npos && p2 != string::npos)
		s = s.substr(p1,p2);
}

//  trim color eg. #80C0FF
void TrimClr(string& s)
{
	if (s.length() < 7)  return;
	if (s[0] != '#')  return;
	
	if (s.find_first_not_of("#0123456789ABCDEFabcdef") >= 7)
		s = s.substr(7);
}


//  list Files in dir 'path' and push to vector 'v'
//-------------------------------------------------------
bool GetFiles(string path, vecstr& v)
{
	bfs::directory_iterator it(path), end_it;
	if (it == end_it)
	{
		cout << "! Empty dir:" << path << endl;
		//log("Empty dir:" << path);
		return false;
	}
	for (; it != end_it; ++it)
	{
		string name = (*it).path().filename().string();
		//if (name != "." && name != "..")
		{
			bool isDir = bfs::is_directory(it->status());
			if (!isDir)  // file
			{
				if (!found(name,".h"))  //  headers don't have transl
					v.push_back(path + "/" + name);
				//cout << name.c_str() << endl;
			}
	}	}
	return true;
}


//  translate tags
///--------------------------------------------------------------------------------------------------------
map<string, string> transl;

string replaceTagsPass(const string& _line, bool& _replaceResult)
{
	//  from MyGUI
	_replaceResult = false;

	string line(_line);

	auto end = line.end();
	for (auto iter = line.begin(); iter != end; )
	{
		if (*iter == '#')
		{
			++iter;
			if (iter == end)
				return line;
			else
			{
				if (*iter != '{')
				{
					++iter;
					continue;
				}
				auto iter2 = iter;
				++iter2;

				while (true)
				{
					if (iter2 == end)
						return line;

					if (*iter2 == '}')
					{
						size_t start = iter - line.begin();
						size_t len = (iter2 - line.begin()) - start - 1;
						const string& tag = line.substr(start + 1, len);
						string replacement;

						bool find = true;
						// try to find in loaded from resources language strings
						auto replace = transl.find(tag);
						if (replace != transl.end())
							replacement = replace->second;
						else
							find = false;

						// try to ask user if event assigned or use #{_tag} instead
						if (!find)
						{
							iter = line.insert(iter, '#') + size_t(len + 2);
							end = line.end();
							break;
						}

						_replaceResult = true;

						iter = line.erase(iter - size_t(1), iter2 + size_t(1));
						size_t pos = iter - line.begin();
						line.insert(pos, replacement);
						iter = line.begin() + pos + replacement.length();
						end = line.end();
						if (iter == end)
							return line;
						break;
					}
					++iter2;
				}
		}	}
		else
			++iter;
	}
	return line;
}


//  translate, replaces all tags #{..} with transl map
string Transl(const string& _line)
{
	string str(_line);
	TrimClr(str);  //clr

	bool replace = false;
	do
	{
		str = replaceTagsPass(str, replace);
	}
	while (replace);

	//  remove spaces  (web display issue)
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

	//  replace spaces with _
	//for (int i=0; i < str.length(); ++i)
	//	if (str[i] == ' ')  str[i] = '_';
	return str;
}


///  main
///--------------------------------------------------------------------------------------------------------
#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	///  ----  [Setup]  ----
	string path = "../";  // data subdir location
	string px = "data/gui/core_language_en_tag.xml";
	string pot = "sr.pot";  // output file  // TODO: save in user dir?
	
	const int bar = 50;   // text progress length
	const bool bLog = 1;  // use log file
	string slog = "sr.log";  // log file
		

	//  log
	#define log(s)  if (bLog)  ofl << s << endl;
	ofstream ofl;
	if (bLog)
		ofl.open(slog.c_str(), ios_base::out);


	cout << "*** Start ***" << endl;

	//  get date, time
    time_t now = time(nullptr);
    tm ti = *localtime(&now);
	stringstream dt;
	dt << put_time(&ti, "%Y-%m-%d %H:%M");

	//  get timezone
	tm utc = *gmtime(&now);  utc.tm_isdst = -1;
	time_t ut = mktime(&utc);
	long z = (now - ut) / 60;  // in sec
	dt << (z > 0 ? "+" : "-");
	dt.width(2);  dt.fill('0');  dt << z / 60;
	dt.width(2);  dt.fill('0');  dt << z % 60;
    
	cout << endl << "Date: " << dt.str() << endl;
	log(endl << "Date: " << dt.str());


	const int si = 1020;  // max file line len
	char s[si+4];  // temp buf

	vector <Tag> tags;
	size_t i,ii;
	int tip=0,inp=0,und=0;
	ifstream fi;


	//  open xml with all tags
	//-----------------------------------------------------------------------------
	px = path + px;
	cout << "Reading xml: " << px << endl;
	log("Reading xml: " << px << endl);

	fi.open(px.c_str(), ios_base::in);
	if (fi.fail())
	{	cout << "Can't open !" << endl;
		log("Can't open !");
		return 0;
	}
	
	map<string, string> dupl;
	string sCmt;
	while (!fi.eof())
	{
		fi.getline(s,si);
		if (strlen(s) > 0)
		{
			string ss = s;
			//  comment  (will be group name for all below)
			if (found(ss, "<!--"))
			{
				//eg.  <!-- General -->
				string sc = GetAttr(ss, "<!--", "-->");
				
				if (IsAlpha(sc))
				{	Trim(sc);
					sCmt = sc;  // set
					//log(sCmt);  //test
			}	}
			else
			{	//  tag (string)
				//eg.  <Tag name="Yes">Yes</Tag>
				
				string name = GetAttr(ss, "<Tag name=\"", "\">");
				string text = GetAttr(ss, "\">", "</");
				
				if (!name.empty() && !text.empty())
				if (!found(name,"LANG_"))  ///  ignored tags  [Setup]
				if (name != "GameVersion" && name != "PageURL")
				{
					//log(/*sCmt.c_str() << "  " <<*/ name.c_str() << "  " << text.c_str());  //test
					//  add tag
					Tag t;
					t.name = name;
					t.text = text;
					t.cmt = sCmt;
					tags.push_back(t);
					transl[name] = text;  //+

					//  stats-
					if (name.substr(0,5) == "Input")  ++inp;
					if (name.substr(0,3) == "Tip")    ++tip;
					if (found(name,"_"))  ++und;
					
					if (dupl[text].empty())  // duplicates check
						dupl[text] = name;
					else
					{	string dut = dupl[text];
						if (!found(name, "Hint-"))
						{	log("WARN: duplicate tag text: " << text << endl <<
								"   for tag: " << name << endl << "   is in: " << dut);
					}	}
				}
				if (!name.empty() && text.empty())
					log("ERR: tag text empty !");
			}
	}	}
	fi.close();

	cout << endl << "Tags Count: " << tags.size()
		<< "   Input: " << inp << "  Tips: " << tip << "  _: " << und
		<< "  Normal: " << tags.size()-inp-tip-und  << endl << endl;
	if (tags.size()==0)
	{	cout << "No tags !" << endl;
		log("No tags !");
		return 0;
	}


	///-----------------------------------------------------------------------------
	//  get gui .layout files  [Setup]  //par
	///-----------------------------------------------------------------------------
	cout << "Listing source dirs" << endl;
	string pLay = path + "data/gui/";
	vecstr lay, lay_n;
	lay.push_back("Game.layout");    lay_n.push_back("Game");
	lay.push_back("Editor.layout");	 lay_n.push_back("Editor");


	//  sources dirs to search (where transl uses are)
	//-----------------------------------------------------------------------------
	string pSrc = path + "source/";
	vecstr dir;
	dir.push_back("ogre");
	dir.push_back("editor");
	dir.push_back("network");
	dir.push_back("ogre/common");
	//dir.push_back("ogre/common/data");  // not needed
	//dir.push_back("road");
	//dir.push_back("vdrift");


	//  get layout files  --------
	vecstr file_lay;
	for (i=0; i < lay.size(); ++i)
		file_lay.push_back(pLay + lay[i]);

	//  get source files  --------
	vecstr file_src;
	for (i=0; i < dir.size(); ++i)
	{
		string ps = pSrc + dir[i];
		GetFiles(ps, file_src);
	}
	cout << "Source files: " << file_src.size() << "  gui: " << file_lay.size() << endl;
	//getchar();

	//  shorter widget type names
	map<string,string> map_wt;
	map_wt["TextBox"] = "Text";    map_wt["TabItem"] = "Tab";
	map_wt["EditBox"] = "Edit";    map_wt["ImageBox"] = "Image";
	map_wt["ComboBox"] = "Combo";  map_wt["TabControl"] = "TabC";

	
	//  read  layout  file lines (all contents)
	///-----------------------------------------------------------------------------
	vector<vecstr*> lay_lin, lay_use;

	ii = file_lay.size();
	for (i=0; i < ii; ++i)
	{
		//cout << "read lay file: " << i << " " << int(100.f*float(i)/float(ii)) << "%" << endl;

		vecstr* ll = new vecstr();  // new
		lay_lin.push_back(ll);
		vecstr* lu = new vecstr();
		lay_use.push_back(lu);
		
		string sf = file_lay[i];
		//cout << sf << endl;

		ifstream fi;
		fi.open(sf.c_str(), ios_base::in);
		if (fi.fail())
			cout << "Can't open: " << sf.c_str() << endl;

		//  widgets captions hierarchy (parents)
		vecstr wh, wt;
		while (!fi.eof())
		{
			fi.getline(s,si);
			string ss = s;
			
			//  new widget
			if (found(ss,"<Widget type="))
			if (!found(ss,"/>"))  // but not 1 line widget
			{
				string type = GetAttr(ss, "type=\"");
				#if 0  //  not needed, too much
				string name = GetAttr(ss, "name=\"");
				//string ww = !name.empty() ? name : !type.empty() ? type : "w";
				//  use type if doesn't have a name
				string ww = !name.empty() ? name : type;
				#else
				string ww;
				#endif
				string ty = map_wt[type];  // shorter
				if (!ty.empty())
					type = ty;
				if (found(ss,"CheckBox") /*&& type == "Button"*/)
					type = "Check";
				wh.push_back(ww);  wt.push_back(type);
			}
			//  closing
			if (found(ss,"</Widget>"))
			{
				wh.pop_back();  wt.pop_back();
			}
			
			if (!wh.empty())
			{
				//  set caption  upd prev use info
				if (found(ss,"key=\"Caption\""))
				{
					//eg.  value="#804060#{HDRTab}"/>
					string v = GetAttr(ss, "value=\"");
					v = Transl(v);
					//if (v.length() > 14)  // max len
					//	v = v.substr(0,14);
					wh[wh.size()-1] = v;
				}
				//key="tip"
			}
			
			lay_lin[i]->push_back(s);

			//  combine hierarchy to 1 string
			string su,z;
			int nn = wh.size()/*-1 /*not last*/, n;
			for (n=0; n < nn; ++n)
			{
				string z = wh[n];  // captions
				//  type for last
				if (n == nn-1)
					z = wt[n];
				
				if (!z.empty())
				{	su += z;
					if (n < nn-1)
						su += ".";  ///par concat char
			}	}
			if (su.empty())
				su = "Root";

			lay_use[i]->push_back(su);

			//log(su);  //test
		}
		fi.close();
	}

	
	//  read  source  file lines (content of all files)
	//-----------------------------------------------------------------------------
	vector<vecstr*> src_lin;

	cout << endl << "Reading source files" << endl;
	//  progress var, bar
	ii = bar;
	float pc_add = 100.f / ii, pc_next = pc_add, pc = 0.f;
	for (i=1; i < ii; ++i)
		cout << ".";
	cout << endl;
	
	ii = file_src.size();
	for (i=0; i < ii; ++i)
	{
		//-  progress
		pc = 100.f * float(i) / float(ii);
		if (pc > pc_next)
		{
			cout << "-";
			pc_next += pc_add;
		}
		//cout << "Read file: " << i << " " << int(pc) << "%" << endl;

		vecstr* ll = new vecstr();  // new
		src_lin.push_back(ll);
		
		string sf = file_src[i];
		//cout << sf << endl;

		ifstream fi;
		fi.open(sf.c_str(), ios_base::in);
		if (fi.fail())
			cout << "Can't open: " << sf.c_str() << endl;

		vecstr vsu;
		while (!fi.eof())
		{
			fi.getline(s,si);
			src_lin[i]->push_back(s);
		}
		fi.close();
	}
	cout << endl;
	
	
	///-----------------------------------------------------------------------------
	//  for each  tag  search through all files for occurrences
	///-----------------------------------------------------------------------------
	cout << endl << "Searching tags occurrences in sources" << endl;
	size_t t, tt = tags.size(), l, ll;
	const vecstr* ln, *lu;  string tag, su_old;
	string::size_type p;
	
	//  progress
	ii = bar;
	pc_add = 100.f / ii;  pc_next = pc_add;  pc = 0.f;
	for (i=1; i < ii; ++i)
		cout << ".";
	cout << endl;
	
	//  for each tag
	for (t=0; t < tt; ++t)
	{
		//-  progress
		pc = 100.f * float(t) / float(tt);
		if (pc > pc_next)
		{
			cout << "|";
			pc_next += pc_add;
		}

		Tag& ta = tags[t];
		stringstream og, os;

		tag = "#{" + ta.name + "}";  // to search
		
		
		//  for each gui file
		//--------------------------------
		ii = file_lay.size();
		bool nLn = false;  su_old = "";
		for (i=0; i < ii; ++i)
		{
			ln = lay_lin[i];  lu = lay_use[i];
			bool fname = true;
			
			//  for each line
			ll = ln->size();
			for (l=0; l < ll; ++l)
			{
				///----  tag in  layout  -----
				if (found((*ln)[l], tag))
				{
					if (nLn)
					{	og << endl << "#: ";  }

					if (fname)
					{
						if (!nLn)
							og << "#: ";
						og << lay[i]+" ";  // lay file name
					}
					if (nLn)  nLn = false;
					fname = false;
											
					og << ":" << l;  // lay line number

					const string& su = (*lu)[l];  // hierarchy
					if (su != su_old)
					{	og << ".." << su;  nLn = true;  }  ///par concat, start
					su_old = su;
			}	}
			if (!fname)
				nLn = true;
		}
		//  save
		//log(og.str());  //test
		ta.gui = og.str();
		
		
		//  for each src file
		//--------------------------------
		nLn = false;
		ii = file_src.size();
		for (i=0; i < ii; ++i)
		{
			string sf = file_src[i].substr(pSrc.length());
			ln = src_lin[i];
			bool fname = true;
			
			//  for each line
			ll = ln->size();
			for (l=0; l < ll; ++l)
			{
				///----  tag in  source  -----
				if (found((*ln)[l], tag))
				{
					if (fname)
					{
						if (nLn)
						{	os << endl;  nLn = false;  }
						os << "#: " << sf;  // file name
					}
					fname = false;
					
					os << ":" << l;  // line number
			}	}
			if (!fname)
				nLn = true;
		}
		//  save
		//if (!os.str().empty())  log(os.str());  //test
		ta.src = os.str();
	}
	cout << endl;


	//  delete  ----
	ii = lay_lin.size();
	for (i=0; i < ii; ++i)
		delete lay_lin[i];

	ii = src_lin.size();
	for (i=0; i < ii; ++i)
		delete src_lin[i];

	
	///  write output .pot file
	//-----------------------------------------------------------------------------
	ofstream of;
	of.open(pot.c_str(), ios_base::out);
	stringstream oe, ol;

	//  header
	of << "# Stunt Rally translation.\n";
	of << "# .pot template by SR-Translator tool, 2022-04-09.\n";  // last tool update
	of << "#\n";
	of << "#, fuzzy\n";
	of << "msgid \"\"\n";
	of << "msgstr \"\"\n";
	of << "\"Project-Id-Version: PACKAGE VERSION\\n\"\n";
	of << "\"Report-Msgid-Bugs-To: \\n\"\n";
	//of << "\"POT-Creation-Date: 2022-01-01 00:00+0200\\n\"\n";
	of << "\"POT-Creation-Date: " << dt.str() << "\\n\"\n";
	of << "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n";
	of << "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n";
	of << "\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n";
	of << "\"Language: \\n\"\n";
	of << "\"MIME-Version: 1.0\\n\"\n";
	of << "\"Content-Type: text/plain; charset=UTF-8\\n\"\n";
	of << "\"Content-Transfer-Encoding: 8bit\\n\"\n";
	of << "\n";

	//  for each tag
	for (t=0; t < tt; ++t)
	{
		const Tag& ta = tags[t];
		const string& s = ta.name;

		if (ta.cmt.empty())
			log(s << "COMMENT empty for: " << s);

		of << "#. " << ta.cmt << endl;  // comment
		// of << "#: ";
		if (!ta.gui.empty())
		of << ta.gui << endl;  // occurrences
		if (!ta.src.empty())
		of << ta.src << endl;

		l = ta.gui.length() + ta.src .length();
		if (l > 300)  ///par 200
			ol << "  " << s << " " << l << endl;

		if (ta.gui.empty() && ta.src.empty())
		if (!found(s,"SC_") && !found(s,"CarDesc_") &&  // false, are used
			!found(s,"Hint-") && !found(s,"InputMap") &&
			!found(s,"LS_") && !found(s,"CarType_") &&
			!found(s,"MessageBox_") && !found(s,"Diff"))
			oe << "  " << s << endl;
		
		of << "msgctxt \"" << s << "\"" << endl;  // context, tag name in xml
		of <<   "msgid \"" << ta.text << "\"" << endl;  // english text

		of << "msgstr \"\"" << endl;  // empty, translation
		of << endl;
	}

	log(endl << "EMPTY tag occurrences for: ");
	log(oe.str());

	log("LONG tag occurrences for: ");
	log(ol.str());

	cout << endl << "*** End ***" << endl;
	return 0;
}
