#include "pch.h"
#include "Def_Str.h"
#include "GuiCom.h"
#include "../common/data/SceneXml.h"
#include "../common/data/CData.h"
#include "../common/data/TracksXml.h"
#include "../common/data/BltObjects.h"
#include "../vdrift/pathmanager.h"
#include "../../road/Road.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CHud.h"
	#include "../CGui.h"
	#include "../vdrift/timer.h"
	#include "../vdrift/game.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
#endif
#include <OgreTimer.h>
#include <OgreResourceGroupManager.h>
#include <MyGUI_ComboBox.h>
using namespace Ogre;
using namespace std;
using namespace MyGUI;


#ifdef SR_EDITOR

//--------  Editor Tools  --------

///  _Tool_ tex ..........................
//  (split 1 rgba terrain texture to 2, 1st rgb and 2nd with alpha in red)
void CGui::ToolTexAlpha()
{
	strlist li;
	string data = PATHMANAGER::Data()+"/terrain";
	PATHMANAGER::DirList(data, li);
	//PATHMANAGER::DirList(data+"2", li);

	int ii=0;
	for (strlist::iterator i = li.begin(); i != li.end(); ++i,++ii)
	if (/*ii < 3 &&*/ !StringUtil::match(*i, "*.txt", false))
	{
		String n = *i;
		Image im;
		im.load(n, "General");

		PixelBox pb = im.getPixelBox();
		//pb.setConsecutive();
		int w = pb.getWidth(), h = pb.getHeight();

		uchar* rgb = new uchar[w*h*3];
		uchar* aa = new uchar[w*h];
		register int i,j,a=0,b=0;
		for (j=0; j < h; ++j)
		for (i=0; i < w; ++i)
		{
			//pb.data
			ColourValue c = pb.getColourAt(i,j,0);
			rgb[a++] = c.b * 255.f;
			rgb[a++] = c.g * 255.f;
			rgb[a++] = c.r * 255.f;
			aa[b++] = c.a * 255.f;
		}
		Ogre::Image ic,ia;
		ic.loadDynamicImage(rgb, w,h, PF_R8G8B8);
		ic.save(PATHMANAGER::Data()+"/"+n+"_d.png");
		ia.loadDynamicImage(aa, w,h, PF_L8);
		ia.save(PATHMANAGER::Data()+"/"+n+"_s.png");
		delete[]rgb;  delete[]aa;
	}
}


///  _Tool_	Warnings  ......................................................
///  check all tracks for warnings
///  Warning: takes about 16 sec
void CGui::ToolTracksWarnings()
{
	Ogre::Timer ti;
	LogO("ALL tracks warnings ---------\n");
	logWarn = true;

	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	//  foreach track
		string trk = data->tracks->trks[i].name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		/**/if (!(trk[0] >= 'A' && trk[0] <= 'Z'))  continue;
		/**/if (StringUtil::startsWith(trk,"test"))  continue;

		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(app);  rd.LoadFile(path +"road.xml");
		app->LoadStartPos(path, true);  // uses App vars-
		
		LogO("Track: "+trk);
		WarningsCheck(&sc,&rd);
	}
	LogO(String("::: Time ALL tracks: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	LogO("ALL tracks warnings ---------");
}


///  _Tool_ brushes prv  ......................................................
//  update all Brushes png
void CGui::ToolBrushesPrv()
{
	Image im;
	for (int i=0; i < app->brSetsNum; ++i)
	{
		app->SetBrushPreset(i);
		app->brushPrvTex->convertToImage(im);
		im.save("data/editor/brush"+toStr(i)+".png");
		// todo: ?brush presets in xml, auto upd prvs-
	}

	#if 1
	///---- combine all images into one ----
	const int ii = 86;
	Image ir;  ir.load("brushes-e.png","General");
	for (int i=0; i <= ii; ++i)
	{
		String s = "brush" + toStr(i) + ".png";
		im.load(s,"General");

		PixelBox pb = im.getPixelBox();
		int xx = pb.getWidth(), yy = pb.getHeight();
		
		//void * pb.data
		int a = (i%16)*128, b = (i/16)*128;
		register int x,y;  ColourValue c;
		for (y = 0; y < yy; ++y)
		for (x = 0; x < xx; ++x)
		{
			c = im.getColourAt(x,y,0);
			ir.setColourAt(c,a+x,b+y,0);
		}
	}
	ir.save("brushes.png");
	#endif
}


///  _Tool_ scene
///  check/resave all tracks scene.xml 
///............................................................................................................................
void CGui::ToolSceneXml()
{
	//Ogre::Timer ti;
	LogO("ALL tracks scene ---------");
	std::map<string, int> noCol,minSc;
	ResourceGroupManager& rg = ResourceGroupManager::getSingleton();

	int i,n;
	for (i=0; i < data->tracks->trks.size(); ++i)
	{	//  foreach track
		string trk = data->tracks->trks[i].name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(app);  rd.LoadFile(path +"road.xml");

		int l = 17-trk.length();  // align
		for (n=0; n < l; ++n)  trk += " ";

		///  terrain
		#if 0  // used
		for (n=0; n < sc.td.layers.size(); ++n)
		{	const TerLayer& l = sc.td.layersAll[sc.td.layers[n]];
		#else  // all
		for (n=0; n < TerData::ciNumLay; ++n)
		{	const TerLayer& l = sc.td.layersAll[n];
		#endif
			if (!l.texFile.empty() && !rg.resourceExistsInAnyGroup(l.texFile))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texFile);

			if (!l.texNorm.empty() && !rg.resourceExistsInAnyGroup(l.texNorm))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texNorm);
				
			//if (!l.texFile.empty() && l.surfName == "Default")
			//	LogO("Ter: " + trk + " Default surface !!!  " + l.texFile);
		}
		
		///  road
		for (n=0; n < MTRs; ++n)
		{
			String s = rd.sMtrRoad[n];
			if (!s.empty() && cmbRoadMtr[0]->findItemIndexWith(s) == MyGUI::ITEM_NONE)
				LogO("Road: " + trk + " Not Found !!!  " + s);

			s = rd.sMtrPipe[n];
			if (!s.empty() && cmbRoadMtr[0]->findItemIndexWith(s) == MyGUI::ITEM_NONE)
				LogO("Road: " + trk + " Not Found !!!  " + s);

			//sMtrWall,sMtrWallPipe, sMtrCol
			//sc.td.layerRoad
		}
		
		///  grass
		for (n=0; n < Scene::ciNumGrLay; ++n)
		{	const SGrassLayer& l = sc.grLayersAll[n];

			String s = l.material;
			if (!s.empty() && l.on &&
				cmbGrassMtr->findItemIndexWith(s) == MyGUI::ITEM_NONE)
				LogO("Grs: " + trk + " Not Found !!!  " + s);
		}

		///  veget
		for (n=0; n < Scene::ciNumPgLay; ++n)
		{
			const PagedLayer& l = sc.pgLayersAll[n];
			const String& s = l.name;  //.mesh
				
			//  checks
			if (!s.empty())
			{
				if (l.on && !rg.resourceExistsInAnyGroup(s))
					LogO("Veg: " + trk + " Not Found !!!  " + s);

				#if 0
				if (l.on && !data->objs->Find(s) && noCol[s]==0)
				if (!(s.length() > 4 && s.substr(0,4) == "rock"))
				{	noCol[s] = 1;
					LogO("Veg: " + trk + " no collision.xml for  " + s);
				}
				#endif

				if (l.minScale < 0.3f && minSc[s]==0)
				{	minSc[s] = 1;
					LogO("Veg: " + trk + " scale < 0.3  model  " + s + "  val " + fToStr(l.minScale,2,4) +" "+ fToStr(l.maxScale,2,4));
				}
				//if (lay.maxScale > 4.f)   LogO("All: " + trk + "  scale > 4  model  "   + s + "  val " + fToStr(lay.maxScale,2,4));
				
				//  rescale for pagedgeom
				/**if (s.substr(0,3)=="fir")
				{
					lay.minScale *= 10.f;  lay.maxScale *= 10.f;
					lay.windFx *= 0.1f;  lay.windFy *= 0.1f;
				}/**/
		}	}
		//sc.SaveXml(path +"scene1.xml");  /// resave
		//SplineRoad rd(this);  rd.LoadFile(path+"road.xml");
		//rd.SaveFile(path+"road1.xml");  // resave
	}
	
	//LogO(String("::: Time ALL tracks: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	LogO("ALL tracks scene ---------");
	exit(0);
}


///  _Tool_ sceneryID and difference
///............................................................................................................................
bool SortT0 (const TrkL& t2, const TrkL& t1){  return t1.name < t2.name;  }

float clrD(Vector3 sc, Vector3 si)
{
	Vector3 d(fabs(sc.x - si.x), fabs(sc.y - si.y), fabs(sc.z - si.z));
	float m1 = std::max(d.x, d.y), m = std::max(m1, d.z);
	return m * 100.f;
}

void CGui::ToolSceneryID()
{
	LogO("ALL tracks ---------");

	std::list<TrkL> liTrk2 = gcom->liTrk;
	liTrk2.sort(SortT0);  liTrk2.reverse();

	//  foreach track
	for (std::list<TrkL>::iterator it = liTrk2.begin(); it != liTrk2.end(); ++it)
	if ((*it).name[0] >= 'A' && (*it).name[0] <= 'Z')  // sr only
	{
		string trk = (*it).name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");
		
		///  find
		string scId = sc.sceneryId + "-";
		int i=0, len = scId.length();
		bool found = false, orig = false;
		Scene si;  string ss;

		//  same scId as track, has original lighting
		if (trk.substr(0,len) == scId)
			orig = true;
		else
		{	//  find the track this scId points to
			while (i < data->tracks->trks.size() && ss.empty())
			{
				if (data->tracks->trks[i].name.substr(0,len) == scId)
					ss = data->tracks->trks[i].name;
				++i;
			}
			found = !ss.empty();
			if (!found)
				LogO("   Not found scId track: " + scId + " for: "+ trk);
			else
				si.LoadXml(gcom->pathTrk[0] +"/"+ ss +"/scene.xml");
		}
		bool w = !orig && found;
		ostringstream s;
		s << fixed << left << setw(18) << trk;
		s << " " << setw(3) << sc.sceneryId;
		if (orig)  s << "+";  else  if (!found)  s << "!";  else  s << " ";
		float dP,dY,dA,dD,dS;

		s << "  pi " << fToStr(sc.ldPitch,0,2);
		dP = fabs(sc.ldPitch - si.ldPitch) / 90.f * 100.f;
		if (w)  s <<" "<< fToStr(dP, 0,3)<<"%";  else  s << "  ---";

		s << "  yaw"    << fToStr(sc.ldYaw,0,4);
		dY = fabs(sc.ldYaw - si.ldYaw)    / 180.f * 100.f;
		if (w)  s <<" "<< fToStr(dY, 0,3)<<"%";  else  s << "  ---";

		s << "  amb "  << fToStr(sc.lAmb.x, 2,4)<<" "<<fToStr(sc.lAmb.y, 2,4)<<" "<<fToStr(sc.lAmb.z, 2,4);
		dA = clrD(sc.lAmb, si.lAmb);
		if (w)  s <<" "<< fToStr(dA, 0,3) <<"%";  else  s << "     ";

		s << "  diff " << fToStr(sc.lDiff.x,2,4)<<" "<<fToStr(sc.lDiff.y,2,4)<<" "<<fToStr(sc.lDiff.z,2,4);
		dD = clrD(sc.lDiff,si.lDiff);
		if (w)  s <<" "<< fToStr(dD, 0,3)<<"%";  else  s << "     ";

		s << "  spec " << fToStr(sc.lSpec.x,2,4)<<" "<<fToStr(sc.lSpec.y,2,4)<<" "<<fToStr(sc.lSpec.z,2,4);
		dS = clrD(sc.lSpec,si.lSpec);
		if (w)
		{	s <<" "<< fToStr(dS, 0,3)<<"%";
			float m0 = std::max(dP,dY), m1 = std::max(dA,dD), m2 = std::max(m0,m1);
			s << "  max " << fToStr(m2, 0,3)<<"%";  if (m2 > 40)  s << " !!! bad";
		}
		LogO(s.str());
	}
	LogO("ALL tracks ---------");
}

#else
//--------  Game Tools  --------

///............................................................................................................................
///  _Tool_ ghosts times
///............................................................................................................................
void CGui::ToolGhosts()
{
	LogO("ALL ghosts ---------");
	using namespace std;
	const string sim = 1 /**/ ? "normal" : "easy";
	String msg="\n";  const float tMax = 10000.f;
	TIMER tim;
	
	//  all cars
	std::vector<string> cars;
	std::vector<float> plc;
	for (int c=0; c < data->cars->cars.size(); ++c)
	{	cars.push_back(data->cars->cars[c].name);
		plc.push_back(0.f);  }

	//  foreach track
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	string trk = data->tracks->trks[i].name;
		if (trk.substr(0,4) == "Test" && trk.substr(0,5) != "TestC")  continue;

		//  records
		tim.Load(PATHMANAGER::Records()+"/"+ sim+"/"+ trk+".txt", 0.f, pGame->error_output);
		float timeES=tMax, timeBest=tMax;
		for (int c=0; c < cars.size(); ++c)
		{
			tim.AddCar(cars[c]);
			float t = tim.GetBestLap(c, false);  //not reverse
			plc[c] = t;
			if (t == 0.f)  continue;

			if (t < timeBest)  timeBest = t;
			if (cars[c] == "ES" || cars[c] == "S1")
				if (t < timeES)  timeES = t;
		}
		if (timeES==tMax)  timeES=0.f;
		if (timeBest==tMax)  timeBest=0.f;
		//  times.xml
		float timeTrk = data->tracks->times[trk];// + 2;

		//float timeB = timeTrk * 1.1f;  // champs factor mostly 0.1
		//const float decFactor = 1.5f;
		//float score = max(0.f, (1.f + (timeB-timeES)/timeB * decFactor) * 100.f);
		float place = app->GetRacePos(timeES,timeTrk,1.f,false);

		///  write
	#if 0
		//  format directly like times.xml
		ostringstream s;
		s << "\t<track name=\""+trk+"\"";
		for (int i=0; i < 18-trk.length(); ++i)
			s << " ";  //align
		s << "time=\""+fToStr(timeES,1)+"\" />";
		msg += s.str()+"\n";
	#else
		//  stats ..
		ostringstream s;
		s << fixed << left << setw(18) << trk;  //align
		#if 0
		s << "  E " << CHud::GetTimeShort(timeES);  // Expected car ES or S1
		s << "  T " << CHud::GetTimeShort(timeTrk);  // trk time from .xml
		s << "  b " << CHud::GetTimeShort(timeES == timeBest ? 0.f : timeBest);
		s << "  E-b " << (timeES > 0.f && timeES != timeBest ?
						fToStr(timeES - timeBest ,0,2) : "  ");
		s << "  T-E " << (timeES > 0.f ?
						fToStr(timeTrk - timeES  ,0,2) : "  ");
		s << "  pET " << (timeES > 0.f ? fToStr(place,1,3) : "   ");
		#endif
		
		//  race pos for all cars from cur ghosts
		for (int c=0; c < cars.size(); ++c)
		{
			float t = plc[c];
			float cmul = app->GetCarTimeMul(cars[c], sim);
			float pl = app->GetRacePos(t,timeTrk, cmul,false);
			s << cars[c] << " " << (t > 0.f ? (pl > 20 ? " ." : fToStr(pl,0,2)) : "  ") << " ";
		}										  //90
		
		//s << (score > 135.f ? " ! " : "   ");
		msg += s.str()+"\n";
	#endif
	}
	LogO(msg);
	//LogO("ALL ghosts ---------");
}

///............................................................................................................................
///  _Tool_ convert ghosts to track's ghosts (less size and frame data)
//  put original ghosts into  data/ghosts/original/*_ES.rpl
//  (ES, normal sim, 1st lap, no boost, use rewind with _Tool_ go back time)
//  time should be like in tracks.ini or less (last T= )
///............................................................................................................................
void CGui::ToolGhostsConv()
{
	LogO("ALL ghosts Convert ---------");
	Replay ghost;  TrackGhost trg;
	bool reverse = false;  string sRev = reverse ? "_r" : "";
	//for both dir sRev..
	
	//  foreach track
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	string track = data->tracks->trks[i].name;
		if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
		
		//  load
		ghost.Clear();  trg.Clear();
		string file = PATHMANAGER::TrkGhosts()+"/original/"+ track + sRev + "_ES.rpl";
		if (!PATHMANAGER::FileExists(file))
		{}	//LogO("NOT found: "+file);
		else
		{	LogO("---------  "+track+"  ---------");
			ghost.LoadFile(file);
			
			//  convert
			MATHVECTOR<float,3> oldPos;  float oldTime = 0.f;
			int num = ghost.GetNumFrames(), jmp = 0;
			
			for (int i=0; i < num; ++i)
			{
				const ReplayFrame& fr = ghost.GetFrame0(i);
				TrackFrame tf;
				tf.time = fr.time;
				tf.pos = fr.pos;
				tf.rot = fr.rot;  //tf.rot[0] = fr.rot[0] * 32767.f;  //..
				tf.brake = fr.braking > 0 ? 1 : 0;
				tf.steer = fr.steer * 127.f;
				//LogO(toStr(fr.braking)+ " st " +fToStr(fr.steer,2,5));

				#define Nth 3
				if (i % Nth == Nth-1)  /// write every n-th frame only
					trg.AddFrame(tf);

				//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
				if (i > 10 && i < num-1)  // ignore jumps at start or end
				{	float dist = (fr.pos - oldPos).MagnitudeSquared();
					if (dist > 16.f)  //1.f small
					{	
						LogO("!Jump at "+CHud::StrTime2(fr.time)+"  d "+fToStr(sqrt(dist),0)+"m");
						++jmp;
				}	}
				//  check vel at start
				if (i==50)
				{
					float dist = (fr.pos - oldPos).Magnitude();
					float vel = 3.6f * dist / (fr.time - oldTime);
					bool bad = vel > 30;
					if (bad)
						LogO("!Vel at "+CHud::StrTime(fr.time)+" kmh "+fToStr(vel,0) + (bad ? "  BAD":""));
				}
				oldPos = fr.pos;  oldTime = fr.time;
			}
			if (jmp > 0)
				LogO("!Jumps: "+toStr(jmp));
		
			//  save
			string fsave = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
			trg.header.ver = 1;
			trg.SaveFile(fsave);
		}
	}

	//  check missing
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	string track = data->tracks->trks[i].name;
		if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
		
		string fsave = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
		if (!PATHMANAGER::FileExists(fsave))
			LogO("MISSING for track: "+track);
	}
}

#endif

#ifdef SR_EDITOR

//  ed presets
///............................................................................................................................

struct TerP{   public:	TerLayer t;     string trk;  };
struct RoadP{  public:	TerLayer t;     string trk, mtr;  };
struct GrassP{ public:	SGrassLayer t;  string trk;  };
struct VegetP{ public:	PagedLayer t;   string trk;  };

//  sort by
bool compT(const TerP& a,   const TerP& b){    return a.t.texFile < b.t.texFile;  }
//bool compR(const RoadP& a,  const RoadP& b){   return a.t.surfName < b.t.surfName;  }
bool compR(const RoadP& a,  const RoadP& b){   return a.mtr < b.mtr;  }
bool compG(const GrassP& a, const GrassP& b){  return a.t.material < b.t.material;  }
bool compV(const VegetP& a, const VegetP& b){  return a.t.name < b.t.name;  }

string getScn(const std::set<char>& sc)
{
	string s;
	for (std::set<char>::const_iterator it = sc.begin(); it != sc.end(); ++it)
		s += *it;
	return s;
}

void CGui::ToolPresets()
{
	LogO("ALL PRESETS ---------");
	const bool all = 1;  ///par
	const int tc = 14;  // trk chars

	std::map<string, int> it, ir, ig, ip;
	std::list<TerP> vt;    std::list<TerP>::iterator vti;
	std::list<RoadP> vr;   std::list<RoadP>::iterator vri;
	std::list<GrassP> vg;  std::list<GrassP>::iterator vgi;
	std::list<VegetP> vp;  std::list<VegetP>::iterator vpi;
	

	int i,n;
	for (i=0; i < data->tracks->trks.size(); ++i)
	{	///  foreach track
		string trk = data->tracks->trks[i].name, path = gcom->pathTrk[0] +"/"+ trk +"/";
		/**/if (!(trk[0] >= 'A' && trk[0] <= 'Z'))  continue;
		/**/if (StringUtil::startsWith(trk,"test"))  continue;

		Scene sc;  sc.LoadXml(path+ "scene.xml");
		SplineRoad rd(app);  rd.LoadFile(path+ "road.xml");

		//  terrain
		for (n=0; n < TerData::ciNumLay; ++n)
		{
			const TerLayer& t = sc.td.layersAll[n];
			if (t.on)
			{	TerP p;  p.t = t;  p.trk = trk;
				int id = it[t.texFile];
				if (all || id == 0)
				{	vt.push_back(p);  it[t.texFile] = vt.size();  }
		}	}

		//  road
		TerLayer& r = sc.td.layerRoad;
		int id = ir[r.surfName];
		if (all || id == 0)
		{	RoadP p;  p.t = r;  p.trk = trk;  p.mtr = rd.sMtrRoad[0];
			vr.push_back(p);  ir[r.surfName] = vr.size();
		}

		//  grass
		for (n=0; n < Scene::ciNumGrLay; ++n)
		{
			const SGrassLayer& t = sc.grLayersAll[n];
			if (t.on)
			{	GrassP p;  p.t = t;  p.trk = trk;
				int id = ig[t.material];
				if (all || id == 0)
				{	vg.push_back(p);  ig[t.material] = vg.size();  }
		}	}

		//  veget
		for (n=0; n < Scene::ciNumPgLay; ++n)
		{
			const PagedLayer& t = sc.pgLayersAll[n];
			if (t.on)
			{	VegetP p;  p.t = t;  p.trk = trk;
				int id = ip[t.name];
				if (all || id == 0)
				{	vp.push_back(p);  ip[t.name] = vp.size();  }
		}	}
	}

	///  sort  . . . .
	vt.sort(compT);  vr.sort(compR);  vg.sort(compG);  vp.sort(compV);

	//  write out
	std::stringstream o;  string z;
	o << fixed;  o << left;  o << endl;
	o << "<presets>\n";
	std::set<char> sc;

	///  terrain
	//<texture on="1" file="adesert_rocky_d.jpg" fnorm="desert_rocky_n.jpg" scale="7.06531" surf="DesertFast" dust="0.8" dustS="1" mud="0.4" smoke="0" tclr="0.48 0.26 0.08 0.7" angMin="0" angMax="8.80626" angSm="4.29007" hMin="-300" hMax="300" hSm="20" nOn="0" noise="1" n_1="0" n2="0">
	for (vti = vt.begin(); vti != vt.end(); ++vti)
	{
		TerLayer& t = (*vti).t;
		string n = (*vti).trk;
		if (t.texFile.substr(t.texFile.length()-4)==".jpg")  t.texFile = t.texFile.substr(0, t.texFile.length()-4);
		if (t.texNorm.substr(t.texNorm.length()-4)==".jpg")  t.texNorm = t.texNorm.substr(0, t.texNorm.length()-4);

		if (z != t.texFile)  {  o << endl;  sc.clear();  }  //
		o << "<t a=\"";
		o.width(tc);  o << n.substr(0,tc);
		o << "\" sc=\"" << n[0] << "\" ";  sc.insert(n[0]);
		o << " t=";  o.width(20);  o << "\""+t.texFile+"\"";  z = t.texFile; //
		o << " n=";  o.width(20);  o << "\""+t.texNorm+"\"";
		o << " s=";  o.width(7);  o << "\""+fToStr(t.tiling,2,4)+"\"";
		o << " su=";  o.width(16);  o << "\""+t.surfName+"\"";  o.width(3);
		o << " du=";  o << "\""+fToStr(t.dust ,1,3)+"\"";
		o << " ds=";  o << "\""+fToStr(t.dustS,1,3)+"\"";
		o << " md=";  o << "\""+fToStr(t.mud  ,1,3)+"\"";
		//of << "\" sm=";  of << "\""+fToStr(t.smoke,1,4);
		o << " tr=";  o << "\""+fToStr(t.tclr.r,2,4)+" "+fToStr(t.tclr.g,2,4)+" "+fToStr(t.tclr.b,2,4)+" "+fToStr(t.tclr.a,1,3)+"\"";
		o << "  aa=";  o.width(4);  o << "\""+fToStr(int(t.angMin),0,1)+"\"";
		o << " ab=";   o.width(4);  o << "\""+fToStr(int(t.angMax),0,1)+"\"";
		o << " z=\""+getScn(sc)+"\"";
		o << " />\n";
	}
	o << endl;  z = "";  sc.clear();

	///  road
	//<texture road="1" surf="roadAdesert" dust="0.6" dustS="1" mud="0" smoke="0" tclr="0.54 0.3 0.22 0.7" />
	for (vri = vr.begin(); vri != vr.end(); ++vri)
	{
		const TerLayer& t = (*vri).t;
		string n = (*vri).trk, m = (*vri).mtr;

		if (z != m)  {  o << endl;  sc.clear();  }  //
		o << "<r a=\"";
		o.width(tc);  o << n.substr(0,tc);  o.width(3);
		o << "\" sc=\"" << n[0] << "\" ";   sc.insert(n[0]);
		o << " m=";  o.width(16);  o << "\""+m+"\"";  z = m; //
		o << "su=";  o.width(16);  o << "\""+t.surfName+"\"";  o.width(3);
		o << " du=";  o << "\""+fToStr(t.dust ,1,3)+"\"";
		o << " ds=";  o << "\""+fToStr(t.dustS,1,3)+"\"";
		o << " md=";  o << "\""+fToStr(t.mud  ,1,4)+"\"";
		o << " tr=";  o << "\""+fToStr(t.tclr.r,2,4)+" "+fToStr(t.tclr.g,2,4)+" "+fToStr(t.tclr.b,2,4)+" "+fToStr(t.tclr.a,1,3)+"\"";
		o << " z=\""+getScn(sc)+"\"";
		o << " />\n";
	}
	o << endl;  z = "";  sc.clear();

	///  grass
	//<grass on="1" mtr="grass16r" clr="grClrWinter.png" dens="0.141625" chan="0" minSx="1.99746" maxSx="2.3799" minSy="1.56875" maxSy="1.92518" swayDistr="4" swayLen="0.2" swaySpeed="0.5" />
	for (vgi = vg.begin(); vgi != vg.end(); ++vgi)
	{
		SGrassLayer& g = (*vgi).t;
		string n = (*vgi).trk;
		if (g.colorMap.substr(g.colorMap.length()-4)==".png")  g.colorMap = g.colorMap.substr(0, g.colorMap.length()-4);

		if (z != g.material)  {  o << endl;  sc.clear();  }  //
		o << "<g a=\"";
		o.width(tc);  o << n.substr(0,tc);
		o << "\" sc=\"" << n[0] << "\" ";  sc.insert(n[0]);
		o << " g=";  o.width(19);  o << "\""+g.material+"\"";  z = g.material; //
		o << " c=";  o.width(19);  o << "\""+g.colorMap+"\"";  o.width(4);
		o << " xa=\"";  o << fToStr(g.minSx,2,4);
		o << "\" xb=\"";  o << fToStr(g.maxSx,2,4);
		o << "\" ya=\"";  o << fToStr(g.minSy,2,4);
		o << "\" yb=\"";  o << fToStr(g.maxSy,2,4);
		o << "\" z=\""+getScn(sc)+"\"";
		o << " />\n";
	}
	o << endl;  z = "";  sc.clear();

	///  veget
	//<layer on="0" name="farn2.mesh" dens="0.102113" minScale="0.149999" maxScale="0.249999" ofsY="0" addTrRdDist="2" maxRdist="5" windFx="7.29999" windFy="0.0599962" maxTerAng="40.2636" minTerH="-100" maxTerH="100" maxDepth="5" />
	for (vpi = vp.begin(); vpi != vp.end(); ++vpi)
	{
		PagedLayer& t = (*vpi).t;
		string n = (*vpi).trk;
		if (t.name.substr(t.name.length()-5)==".mesh")  t.name = t.name.substr(0, t.name.length()-5);

		if (z != t.name)  {  o << endl;  sc.clear();  }  //
		o << "<v a=\"";
		o.width(tc);  o << n.substr(0,tc);
		o << "\" sc=\"" << n[0] << "\" ";  sc.insert(n[0]);
		o << " p=";  o.width(21);  o << "\""+t.name+"\"";  z = t.name; //
		o << "  sa="; o << "\""+fToStr(t.minScale,2,4)+"\"";
		o << " sb=";  o << "\""+fToStr(t.maxScale,2,4)+"\"";
		o << "  wx="; o << "\""+fToStr(t.windFx,2,4)+"\"";
		o << " wy=";  o << "\""+fToStr(t.windFy,3,5)+"\"";
		o << "  ab=";  o << "\""+fToStr(t.maxTerAng,2,5)+"\"";
		o << " z=\""+getScn(sc)+"\"";
		o << " />\n";
	}
	o << "</presets>";
	
	#if 0
	//  save file
	ofstream f;
	string p = PATHMANAGER::DataUser() + "/presets.xml";
	f.open(p.c_str());
	f << o.str();
	f.close();
	LogO("Saved: "+p);
	#endif

	///  check presets  ......................................................
	LogO("ALL PRESETS check ---------");
	
	//  tex not in preset ..
	for (i=0; i < data->pre->ter.size(); ++i)
	{
		const PTer& pt = data->pre->ter[i];
		size_t id = cmbTexDiff->findItemIndexWith(pt.texFile+".jpg");
		if (id == ITEM_NONE)
			LogO("NO ter !! "+pt.texFile);
	}
	
	LogO("ALL PRESETS ---------");
}

#endif
