#include "pch.h"
#include "../Def_Str.h"
#include "TracksXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"
#include <set>
#include "../vdrift/pathmanager.h"
#include "../../ReplayTrk.h"  // check
#include "../../CHud.h"  // StrTime
using namespace std;
using namespace tinyxml2;
using Ogre::uchar;


Date s2dt(const char* a)
{
	Date d;  d.day=0; d.month=0; d.year=0;
	TIXML_SSCANF(a,"%2d-%2d-%2d",&d.day,&d.month,&d.year);
	return d;
}
string dt2s(const Date& dt)
{
	return fToStr(dt.day,0,2)+"-"+fToStr(dt.month,0,2)+"-"+fToStr(dt.year,0,2);
}


///  User Load
///------------------------------------------------------------------------------------

bool UserXml::LoadXml(string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	trks.clear();  trkmap.clear();

	//  tracks
	const char* a;  //int i=1;  //0 = none
	XMLElement* eTrk = root->FirstChildElement("track");
	while (eTrk)
	{
		UserTrkInfo t;
		a = eTrk->Attribute("n");		if (a)  t.name = string(a);

		a = eTrk->Attribute("date");	if (a)  t.last = s2dt(a);
		a = eTrk->Attribute("rate");	if (a)  t.rating = s2i(a);
		a = eTrk->Attribute("laps");	if (a)  t.laps = s2i(a);

		trks.push_back(t);
		trkmap[t.name] = trks.size();  //i++;
		eTrk = eTrk->NextSiblingElement("track");
	}
	return true;
}


///  User Save
///------------------------------------------------------------------------------------

bool UserXml::SaveXml(string file)
{
	TiXmlDocument xml;	TiXmlElement root("tracks");

	for (int i=0; i < trks.size(); ++i)
	{
		const UserTrkInfo& t = trks[i];
		TiXmlElement trk("t");
		trk.SetAttribute("n",		t.name.c_str());

		trk.SetAttribute("date",	dt2s(t.last).c_str());
		trk.SetAttribute("rate",	toStrC( t.rating ));
		trk.SetAttribute("laps",	toStrC( t.laps ));
		
		root.InsertEndChild(trk);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------
//  Load Ini
//--------------------------------------------------------------------------------------------------------------------------------------
bool TracksXml::LoadIni(string file, bool check)
{
	//  clear
	trks.clear();  trkmap.clear();  times.clear();  cntAll = 0;

	int i=1;  // 0 = not found
	char s[256], name[32],scenery[24],author[32];
	float time=0.f;
	TrackInfo t;

	set<string> scns;  //stats
	map<string, int> scn_trks;

	ifstream fs(file.c_str());
	if (fs.fail())  return false;
	
	while (fs.good())
	{
		fs.getline(s,254);

		//  starting with digit
		if (strlen(s) > 0 && s[0] >= '0' && s[0] <= '9')
		{
		//114,Fin1-Lakes  v=2.0 06/04/13 07/04/13 :Finland  |o0 w1 ~1 J2 L0 P0 /1 s1 l2 !2 *4  T=107.8  a:CH
			sscanf(s,
			"%d,%s v%f %d/%d/%d %d/%d/%d :%s |o%d c%d w%d ~%d J%d L%d P%d /%d s%d n%d l%d !%d *%d  T=%f a:%s"
				,&t.n, name, &t.crtver
					,&t.created.day, &t.created.month, &t.created.year
					,&t.modified.day, &t.modified.month, &t.modified.year
				,scenery
				,&t.objects, &t.obstacles
				,&t.fluids, &t.bumps, &t.jumps, &t.loops, &t.pipes
				,&t.banked, &t.frenzy, &t.narrow, &t.longn, &t.diff, &t.rating
				,&time, author);

			t.name = name;
			t.scenery = scenery;
			t.author = author;
			t.ver = std::ceil(t.crtver*10.f);
			
			if (check)
			{	scns.insert(scenery);
				++scn_trks[scenery];
			}
			t.sum = t.objects +t.obstacles + t.fluids +t.bumps +
				 t.jumps +t.loops +t.pipes + t.banked +t.frenzy;

			string shrt;  //-  name short  (without prefix)
			size_t p = t.name.find("-");
			if (p != string::npos /*&& !(name[0]=='T' && name[1]=='e')*/)
				shrt = t.name.substr(p+1);  // short name
			else
				shrt = t.name;
			t.nshrt = shrt;
			
			//  get number from name, id for scenery
			t.nn = 0;
			size_t p2 = t.name.find('-'), p1;
			if (p2 != string::npos)
			{
				p1 = t.name.find_first_of("1234567890");
				if (p1 != string::npos && p2-p1 > 0)
				{
					string ss = t.name.substr(p1, p2-p1);
					t.nn = atoi(ss.c_str());
					t.nn += uchar(t.name[0])<<16;  // meh
					t.nn += uchar(t.name[1])<<8;
					//LogO(t.name+"  "+ss+"  "+toStr(t.nn));
			}	}
			
				t.testC = t.name.length() > 5 ? t.name.substr(0,5)=="TestC" : false;
			if (t.testC)  t.test = false;  else
				t.test  = t.name.length() > 4 ? t.name.substr(0,4)=="Test"  : false;
			if (!t.testC && !t.test)
				++cntAll;

			trks.push_back(t);
			trkmap[name] = i++;
			times[name] = time;
		}
	}
	
	///  *  checks and  Log sceneries with stats  *
	//--------------------------------------------------------------
	if (check)
	{
		LogO("))) Checking: tracks.ini");
		i=1;  int c,n, nn = trks.size()-1;

		stringstream ss;  ss << fixed;
		ss << "))) Sceneries stats:\n""Num - Name - tracks count - last track in it (num trks back)""\n";

		for (auto it = scns.begin(); it != scns.end(); ++it,++i)
		{
			//  find last trk in this scn
			n = nn;  bool srch = true;
			while (srch && n >= 0)
			{
				if (trks[n].scenery == *it)
					srch = false;
				else  --n;
			}
			int d = nn-n;  c = scn_trks[*it];
			ss << right << setw(2) << i << "  " << setw(13) << left << *it << "  "
			   << setw(3) << c << (c==1 ? "!!" : c==2 ? "! " : "  ") << " "
			   << setw(4) << d << (d>35?"!":"") << (d>50?"!":"") << (d>90?"!":"") << endl;
		}
		LogO(ss.str());
		
		LogO("))) Checking for all tracks ghosts");
		nn = trks.size();
		for (i=0; i < nn; ++i)
		{
			const TrackInfo& ti = trks[i];
			const string& s = ti.name;
			for (int r=0; r < 2; ++r)
			{
				string sRev = r==1 ? "_r" : "";
				string file = PATHMANAGER::TrkGhosts()+"/"+ s + sRev + ".gho";
				if (!ti.test && !ti.testC)
				if (!PATHMANAGER::FileExists(file))
				{	if (r==1)
					{	if (s != "Des14-JumpCrazy")  //denyReversed)
							LogO("!Rev Missing trk gho for: " + s);
					}else	LogO("! Missing trk gho for: " + s);
				}else
				{	//  check time  can take few sec
					TrackGhost gho;  gho.LoadFile(file, false);
					float tgh = gho.GetTimeLength();
					float ti = times[s]*1.02f, td = tgh - ti;
					if (fabs(td) > 20.f)
					{	ss.str("");
						ss << "time diff big: " << setw(19) << s+sRev;
						ss << " time "+StrTime(tgh)+" trk "+StrTime(ti)+" d "+fToStr(td,0,3);
						LogO(ss.str());
				}	}
			}
		}
		LogO("");
		
		//  not in tracks.ini
		strlist li;
		PATHMANAGER::DirList(PATHMANAGER::Tracks(), li);
		for (auto s : li)
		{
			if (trkmap[s]==0 && s.find('-') != string::npos)
				LogO("!! Track not in ini: " + s);
		}
		LogO("");
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------



///  Load  cars.xml
//-------------------------------------------------------------------------------------

bool CarsXml::LoadXml(string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;

	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	cars.clear();  carmap.clear();  colormap.clear();

	
	//  global time mul factors
	float fEasy = 1.f, fNorm = 1.f;
	magic = 0.010f;

	const char* a;
	XMLElement* eGlobal = root->FirstChildElement("global");
	if (eGlobal)
	{
		a = eGlobal->Attribute("easy");  if (a)  fEasy = s2r(a);
		a = eGlobal->Attribute("norm");  if (a)  fNorm = s2r(a);
		a = eGlobal->Attribute("magic"); if (a)  magic = s2r(a);
	}

	///  cars
	int i=1;  //0 = none
	XMLElement* eCar = root->FirstChildElement("c");
	while (eCar)
	{
		CarInfo c;
		a = eCar->Attribute("id"); if (a)  c.id = string(a);
		a = eCar->Attribute("m");  if (a)  c.name = string(a);
		a = eCar->Attribute("t");  if (a)  c.type = string(a);

		a = eCar->Attribute("s");  if (a)  c.speed = s2r(a);
		a = eCar->Attribute("y");  if (a)  c.year = s2i(a);
		a = eCar->Attribute("o");  if (a)  c.wheels = s2i(a);
		
		a = eCar->Attribute("r");  if (a)  c.rating = s2i(a);
		a = eCar->Attribute("w");  if (a)  c.width = s2i(a);
		a = eCar->Attribute("d");  if (a)  c.diff = s2i(a);

		a = eCar->Attribute("e");  if (a)  c.easy = fEasy * s2r(a);
		a = eCar->Attribute("n");  if (a)  c.norm = fNorm * s2r(a);
		
		a = eCar->Attribute("a");  if (a)  c.author = string(a);

		a = eCar->Attribute("b");  if (a)  c.bumps = s2i(a);
		a = eCar->Attribute("j");  if (a)  c.jumps = s2i(a);
		a = eCar->Attribute("l");  if (a)  c.loops = s2i(a);
		a = eCar->Attribute("p");  if (a)  c.pipes = s2i(a);

		cars.push_back(c);
		carmap[c.id] = i++;
		eCar = eCar->NextSiblingElement("c");
	}

	//  type colors
	XMLElement* eColor = root->FirstChildElement("color");
	while (eColor)
	{
		string type, clr;
		a = eColor->Attribute("type");	if (a)  type = string(a);
		a = eColor->Attribute("color");	if (a)  clr = string(a);

		if (!type.empty() && !clr.empty())
			colormap[type] = clr;
		eColor = eColor->NextSiblingElement("color");
	}
	return true;
}



//  Load car colors.ini
//--------------------------------------------------------------------------------------------------------------------------------------
bool ColorsXml::LoadIni(string file)
{
	v.clear();

	char s[256];

	ifstream fs(file.c_str());
	if (fs.fail())  return false;
	
	while (fs.good())
	{
		fs.getline(s,254);
		
		if (strlen(s) > 0 && s[0] != '#' && s[0] != '/' && s[0] != ' ')  //  comment
		{
			string t = s;  //  params
			     if (t.substr(0,6) == "perRow")   perRow =  s2i(t.substr(6));
			else if (t.substr(0,7) == "imgSize")  imgSize = s2i(t.substr(7));
			else
			//  color, starting with digit
			if (s[0] >= '0' && s[0] <= '9')
			{
				CarColor c;
				sscanf(s, "%f %f %f %f %f",
					&c.hue, &c.sat, &c.val, &c.gloss, &c.refl);

				v.push_back(c);
			}
	}	}
	return true;
}


///  Load  reverbs.xml
//-------------------------------------------------------------------------------------

bool ReverbsXml::LoadXml(string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;

	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	revs.clear();  revmap.clear();


	//  base
	XMLElement* b = root->FirstChildElement("base");
	if (b)
		GetParams(b, base);

	///  revs
	int i=1;  //0 = none
	XMLElement* n = root->FirstChildElement("rev");
	while (n)
	{
		ReverbSet r;
		GetParams(n, r);

		revs.push_back(r);
		revmap[r.name] = i++;
		n = n->NextSiblingElement("rev");
	}
	return true;
}

void ReverbsXml::GetParams(XMLElement* e, ReverbSet& r)
{
	const char* a;
	a = e->Attribute("name");		if (a)  r.name = string(a);
	a = e->Attribute("descr");		if (a)  r.descr = string(a);

	a = e->Attribute("normal");		if (a)  r.normal = string(a);
	a = e->Attribute("cave");		if (a)  r.cave = string(a);
	a = e->Attribute("cavebig");	if (a)  r.cavebig = string(a);
	a = e->Attribute("pipe");		if (a)  r.pipe = string(a);
	a = e->Attribute("pipebig");	if (a)  r.pipebig = string(a);
	a = e->Attribute("influid");	if (a)  r.influid = string(a);
}
