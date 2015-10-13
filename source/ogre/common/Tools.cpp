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
	#include "CScene.h"
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
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
		Image ic,ia;
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


///............................................................................................................................
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
		bool modif = false;

		int l = 17-trk.length();  // align
		for (n=0; n < l; ++n)  trk += " ";

		///  sound
		if (sc.sReverbs=="")
			LogO("No reverb! "+trk);
		else
		{	int id = data->reverbs->revmap[sc.sReverbs]-1;
			if (id==-1)  LogO("Reverb not found! "+trk+"  "+sc.sReverbs);
		}		
		///  sky clrs
		string s;
		s += sc.lAmb.Check("amb");  s += sc.lDiff.Check("dif");  s += sc.lSpec.Check("spc");
		s += sc.fogClr.Check("fog");  s += sc.fogClr2.Check("fog2");  s += sc.fogClrH.Check("foh");
		if (!s.empty())
			LogO("CLR CHK! "+trk+"  "+s);
		
		///  terrain
		#if 0  // used
		for (n=0; n < sc.td.layers.size(); ++n)
		{	const TerLayer& l = sc.td.layersAll[sc.td.layers[n]];
		#else  // all
		for (n=0; n < TerData::ciNumLay; ++n)
		{	TerLayer& l = sc.td.layersAll[n];
		#endif
			bool e = l.texFile.empty();
			if (!e && !rg.resourceExistsInAnyGroup(l.texFile))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texFile);

			if (!l.texNorm.empty() && !rg.resourceExistsInAnyGroup(l.texNorm))
				LogO("Ter: " + trk + " Not Found !!!  " + l.texNorm);
				
			const PTer* p = data->pre->GetTer(l.texFile.substr(0, l.texFile.length()-4));
			if (!e && !p)
				LogO("Ter: " + trk + " Not Found in presets !!!  " + l.texFile);

			if (!e && l.surfName == "Default")
			{
				LogO("Ter: " + trk + " Default surface !!!  " + l.texFile);
				#if 0  //  fix from presets
				l.surfName = p->surfName;
				l.dust = p->dust;   l.dustS = p->dustS;
				l.mud = p->mud;  l.smoke = 0.f;  l.tclr = p->tclr;
				modif = true;
				LogO("Ter:  Fixed");
				#endif
			}
			#if 0
			if (!e && p && l.surfName != p->surfName)
				LogO("Ter: " + trk + " Different surface !  " + l.texFile + " " + l.surfName + " pre: " + p->surfName);
			#endif
		}
		
		///  road
		int iLch = 0;
		for (n=0; n < rd.mP.size(); ++n)
			if (rd.mP[n].chkR > 0.f && rd.mP[n].loop > 0)
				++iLch;
		//LogO("Road: " + trk + "  Lch " + toStr(iLch));
		if (iLch % 2 == 1)
			LogO("Road: " + trk + " Not even loop chks count !  ");

		for (n=0; n < MTRs; ++n)
		{
			String s = rd.sMtrRoad[n];
			//if (!s.empty() && cmbRoadMtr[0]->findItemIndexWith(s) == MyGUI::ITEM_NONE)
			//	LogO("Road: " + trk + " Not Found !!!  " + s);

			if (!s.empty() && !data->pre->GetRoad(s))
				LogO("Road: " + trk + " Not Found in presets !!!  " + s);

			s = rd.sMtrPipe[n];
			if (!s.empty() && cmbPipeMtr[0]->findItemIndexWith(s) == MyGUI::ITEM_NONE)
				LogO("Road: " + trk + " Not Found !!!  " + s);

			//if (!s.empty() && !data->pre->GetRoad(s))
			//	LogO("Pipe: " + trk + " Not Found in presets !!!  " + s);
	
			//sMtrWall,sMtrWallPipe, sMtrCol
			//sc.td.layerRoad
		}
		
		///  grass
		for (n=0; n < Scene::ciNumGrLay; ++n)
		{	const SGrassLayer& l = sc.grLayersAll[n];

			String s = l.material;
			if (!s.empty() && l.on && !data->pre->GetGrass(s))
				LogO("Grs: " + trk + " Not Found in presets !!!  " + s);
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

				if (l.on &&/**/ !data->pre->GetVeget(s.substr(0,s.length()-5)))
					LogO("Veg: " + trk + " Not Found in presets !!!  " + s);

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

		if (modif)
			sc.SaveXml(path +"scene.xml");  /// resave
		//SplineRoad rd(this);  rd.LoadFile(path+"road.xml");
		//rd.SaveFile(path+"road1.xml");  // resave
	}
	
	//LogO(String("::: Time ALL tracks: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	LogO("ALL tracks scene ---------");
}


#else
//--------  Game Tools  --------

///............................................................................................................................
///  _Tool_ ghosts times (user, to check times and final place)
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
	{	cars.push_back(data->cars->cars[c].id);
		plc.push_back(0.f);  }

	//  foreach track
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{	string trk = data->tracks->trks[i].name;
		if (trk.substr(0,4) == "Test" && trk.substr(0,5) != "TestC")  continue;

		//  records
		tim.Load(PATHMANAGER::Records()+"/"+ sim+"/"+ trk+".txt", 0.f);
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
		//  T= from tracks.ini
		float timeTrk = data->tracks->times[trk];// + 2;

		//float timeB = timeTrk * 1.1f;  // champs factor mostly 0.1
		//const float decFactor = 1.5f;
		//float score = max(0.f, (1.f + (timeB-timeES)/timeB * decFactor) * 100.f);
		//float place = app->GetRacePos(timeES,timeTrk,1.f,false);

		///  write
	#if 0
		//  format trk times (old)
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
	Replay2 ghost;  TrackGhost trg;
	for (int r=0; r < 2; ++r)
	{
		string sRev = r==1 ? "_r" : "";

		//  for each track
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
					const ReplayFrame2& fr = ghost.GetFrame0(i);
					TrackFrame tf;
					tf.time = fr.time;
					tf.pos = fr.pos;  tf.rot = fr.rot;
					tf.brake = fr.get(b_braking) > 0 ? 1 : 0;
					tf.steer = fr.steer * 127.f;
					//LogO(toStr(fr.braking)+ " st " +fToStr(fr.steer,2,5));

					#define Nth 2  // old was 1/3 from 160, new 1/2 from 80
					if (i % Nth == Nth-1)  /// write every n-th frame only
						trg.AddFrame(tf);

					//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
					if (i > 10 && i < num-1)  // ignore jumps at start or end
					{	float dist = (fr.pos - oldPos).MagnitudeSquared();
						if (dist > 16.f)  //1.f small
						{	
							LogO("!Jump at "+StrTime2(fr.time)+"  d "+fToStr(sqrt(dist),0)+"m");
							++jmp;
					}	}
					//  check vel at start
					if (i==50)
					{
						float dist = (fr.pos - oldPos).Magnitude();
						float vel = 3.6f * dist / (fr.time - oldTime);
						bool bad = vel > 30;
						if (bad)
							LogO("!Vel at "+StrTime(fr.time)+" kmh "+fToStr(vel,0) + (bad ? "  BAD":""));
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
				if (r==1)	LogO("!Rev Missing for track: " + track);
				else		LogO("!!   Missing for track: " + track);
		}
	}
}

///  _Tool_ check tracks ghosts
///............................................................................................................................
void CGui::ToolTestTrkGhosts()
{
	LogO("ALL tracks ghosts Test ---------");
	TrackGhost gho;
	
	//  foreach track
	String ss;
	for (int r=0; r < 2; ++r)
	{	
		string sRev = r==1 ? "_r" : "";
		for (int i=0; i < data->tracks->trks.size(); ++i)
		{	string track = data->tracks->trks[i].name;
			//if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
			
			//  load
			gho.Clear();
			string file = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
			if (!PATHMANAGER::FileExists(file))
			{	/*LogO("NOT found: "+file);/**/  }
			else
			{	LogO("---------  "+track+"  ---------");
				gho.LoadFile(file);
				
				//  test
				MATHVECTOR<float,3> oldPos;  float oldTime = 0.f;
				int num = gho.getNumFrames(), jmp = 0;
				for (int i=0; i < num; ++i)
				{
					const TrackFrame& fr = gho.getFrame0(i);

					//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
					if (i > 10 && i < num-1)  // ignore jumps at start or end
					{	float dist = (fr.pos - oldPos).MagnitudeSquared();
						if (dist > 6.f*6.f)  //par
						{	
							LogO("!Jump at "+StrTime2(fr.time)+"  d "+fToStr(sqrt(dist),0)+"m");
							++jmp;
					}	}
					//  check vel at start
					if (i==50/3)
					{
						float dist = (fr.pos - oldPos).Magnitude();
						float vel = 3.6f * dist / (fr.time - oldTime);
						bool bad = vel > 30;
						if (bad)
							LogO("!Vel at "+StrTime(fr.time)+" kmh "+fToStr(vel,0) + (bad ? "  BAD":""));
					}
					oldPos = fr.pos;  oldTime = fr.time;
				}
				if (jmp > 0)
				{	LogO("!Jumps: "+toStr(jmp));
					ss += "\n" + track;
			}	}
		}
	}
	LogO("!! Jumps on tracks:"+ss);
}

#endif
