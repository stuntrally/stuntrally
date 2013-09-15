#include "pch.h"
#include "Defines.h"
#include "../vdrift/pathmanager.h"
#include "../../road/Road.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CHud.h"
	#include "../CGui.h"
	#include "../vdrift/timer.h"
	#include "../vdrift/game.h"
#else
	#include "../../editor/OgreApp.h"
#endif
using namespace Ogre;
using namespace std;


#ifdef SR_EDITOR

//--------  Editor Tools  --------
///........................................................................................................

///  _Tool_ tex ..........................
//  (remove alpha channel for ter tex prv img)
void App::ToolTexAlpha()
{
	Ogre::Image im;
	im.load("jungle_5d.png", "General");

	PixelBox pb = im.getPixelBox();
	int w = pb.getWidth(), h = pb.getHeight();
	for(int j=0; j < h; ++j)
	for(int i=0; i < w; ++i)
	{
		ColourValue c = pb.getColourAt(i,j,0);
		c.a = 1.f;
		pb.setColourAt(c,i,j,0);
	}
	im.save(PATHMANAGER::Data()+"/prv.png");
}


///  _Tool_ scene ...........................
///  check/resave all tracks scene.xml 
void App::ToolSceneXml()
{
	QTimer ti;  ti.update();  /// time
	LogO("ALL tracks scene ---------");
	std::map<string, int> noCol,minSc;

	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	//  foreach track
		string trk = tracksXml.trks[i].name, path = pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");
		for (int l=0; l < Scene::ciNumPgLay; ++l)
		{
			PagedLayer& lay = sc.pgLayersAll[l];
			const String& s = lay.name;  //.mesh
				
			//  checks
			if (!s.empty())
			{
				//  rescale for pagedgeom
				/**if (s.substr(0,3)=="fir")
				{
					lay.minScale *= 10.f;  lay.maxScale *= 10.f;
					lay.windFx *= 0.1f;  lay.windFy *= 0.1f;
				}/**/

				if (lay.on && !objs.Find(s) && noCol[s]==0)
				{	noCol[s] = 1;
					LogO("All: " + trk + "  no collision.xml for  " + s);
				}
				if (lay.on && !ResourceGroupManager::getSingleton().resourceExistsInAnyGroup(s))
					LogO("All: " + trk + "  Not Found !!!  " + s);

				if (lay.minScale < 0.3f && minSc[s]==0)
				{	minSc[s] = 1;
					LogO("All: " + trk + "  scale < 0.3  model  " + s + "  val " + fToStr(lay.minScale,2,4) +" "+ fToStr(lay.maxScale,2,4));
				}
				//if (lay.maxScale > 4.f)   LogO("All: " + trk + "  scale > 4  model  "   + s + "  val " + fToStr(lay.maxScale,2,4));
		}	}
		//sc.SaveXml(path +"scene1.xml");  /// resave
		//SplineRoad rd(this);  rd.LoadFile(path+"road.xml");
		//rd.SaveFile(path+"road1.xml");  // resave
	}
	
	ti.update();  float dt = ti.dt * 1000.f;  /// time
	LogO(String("::: Time ALL tracks: ") + fToStr(dt,0,3) + " ms");
	LogO("ALL tracks scene ---------");
	exit(0);
}


///  _Tool_ write sceneryID
#define sArg  const TrkL& t2, const TrkL& t1
#define sortDef  bool t = false/*t1.test < t2.test/**/;  if (!t1.ti || !t2.ti)  return t1.name > t2.name || t;
/* 0  name    */  bool SortT0 (sArg){  sortDef  return t1.name  < t2.name   || t;  }

void App::ToolListSceneryID()
{
	LogO("ALL tracks ---------");

	std::list<TrkL> liTrk2 = liTrk;
	liTrk2.sort(SortT0);  liTrk2.reverse();

	//  foreach track
	for (std::list<TrkL>::iterator it = liTrk2.begin(); it != liTrk2.end(); ++it)
	{
		string trk = (*it).name, path = pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");

		ostringstream s;
		s << fixed << left << setw(18) << trk;
		s << " scID: " << setw(3) << sc.sceneryId;
		s << "  pitch " << fToStr(sc.ldPitch,0,2);
		s << "  yaw" << fToStr(sc.ldYaw,0,4);
		s << "  amb "<<fToStr(sc.lAmb.x, 2,4)<<" "<<fToStr(sc.lAmb.y, 2,4)<<" "<<fToStr(sc.lAmb.z, 2,4);
		s << "  diff "<<fToStr(sc.lDiff.x,2,4)<<" "<<fToStr(sc.lDiff.y,2,4)<<" "<<fToStr(sc.lDiff.z,2,4);
		s << "  spec "<<fToStr(sc.lSpec.x,2,4)<<" "<<fToStr(sc.lSpec.y,2,4)<<" "<<fToStr(sc.lSpec.z,2,4);
		LogO(s.str());
	}
	LogO("ALL tracks ---------");
}


///  _Tool_	Warnings ...........................
///  check all tracks for warnings
///  Warning: takes about 16 sec
void App::ToolTracksWarnings()
{
	QTimer ti;  ti.update();  /// time
	LogO("ALL tracks warnings ---------\n");
	logWarn = true;

	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	//  foreach track
		string trk = tracksXml.trks[i].name, path = pathTrk[0] +"/"+ trk +"/";
		/**/if (!(trk[0] >= 'A' && trk[0] <= 'Z'))  continue;
		/**/if (StringUtil::startsWith(trk,"test"))  continue;

		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(this);  rd.LoadFile(path +"road.xml");
		LoadStartPos(path, true);  // uses App vars-
		
		LogO("Track: "+trk);
		WarningsCheck(&sc,&rd);
	}
	ti.update();  float dt = ti.dt * 1000.f;  /// time
	LogO(String("::: Time ALL tracks: ") + fToStr(dt,0,3) + " ms");
	LogO("ALL tracks warnings ---------");
}


///  _Tool_ brushes prv ...........................
//  update all Brushes png
void App::ToolBrushesPrv()
{
	Image im;
	for (int i=0; i < brSetsNum; ++i)
	{
		SetBrushPreset(i);
		brushPrvTex->convertToImage(im);
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
	//exit(0);
	#endif
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
	for (int c=0; c < carsXml.cars.size(); ++c)
	{	cars.push_back(carsXml.cars[c].name);
		plc.push_back(0.f);  }

	//  foreach track
	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	string trk = tracksXml.trks[i].name;
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
		float timeTrk = tracksXml.times[trk];// + 2;

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
	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	string track = tracksXml.trks[i].name;
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
	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	string track = tracksXml.trks[i].name;
		if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
		
		string fsave = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
		if (!PATHMANAGER::FileExists(fsave))
			LogO("MISSING for track: "+track);
	}
}

//  ed presets
///............................................................................................................................
void CGui::ToolPresets()
{
	QTimer ti;  ti.update();  /// time
	LogO("ALL tracks presets ---------\n");

	std::map<Ogre::String, TerLayer> ter;
	for (int i=0; i < tracksXml.trks.size(); ++i)
	{	//  foreach track
		string trk = tracksXml.trks[i].name, path = pathTrk[0] +"/"+ trk +"/";
		/**/if (!(trk[0] >= 'A' && trk[0] <= 'Z'))  continue;
		/**/if (StringUtil::startsWith(trk,"test"))  continue;

		Scene sc;  sc.LoadXml(path +"scene.xml");
		SplineRoad rd(pGame);  rd.LoadFile(path +"road.xml");
		LogO("Track: "+trk);

		for (int l=0; l < sc.td.layers.size(); ++l)
		{
			const TerLayer& la = sc.td.layersAll[sc.td.layers[l]];
			LogO(la.texFile+"  dust "+fToStr(la.dust,2,4)+" "+fToStr(la.dustS,2,4)+"  mud "+fToStr(la.mud,2,4)+
				"  trl "+fToStr(la.tclr.r,2,4)+" "+fToStr(la.tclr.g,2,4)+" "+fToStr(la.tclr.b,2,4)+" "+fToStr(la.tclr.a,2,4));
			ter[la.texFile] = la;
		}
		//sc.layerRoad.texFile
	}
	LogO("ALL ter ---------");
	LogO(toStr(ter.size()));
	ti.update();  float dt = ti.dt * 1000.f;  /// time
	LogO(String("::: Time ALL tracks: ") + fToStr(dt,0,3) + " ms");
	LogO("ALL tracks presets ---------");
}

#endif