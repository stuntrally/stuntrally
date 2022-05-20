#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/Axes.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <OgreTerrain.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_EditBox.h>
using namespace Ogre;


///...........................................................................................................................
//  check track, and report warnings
///...........................................................................................................................

const static String clrWarn[5] = {"#FF4040","#FFA040","#E0E040","#80F040","#60A0E0"};
const static String strWarn[5] = {"ERR   ","WARN  ","Info  ","Note  ","Txt   "};
void CGui::Warn(eWarn type, String text)
{
	if (logWarn)
		LogO(strWarn[type]+text);
	else
		edWarn->addText(clrWarn[type]+text+"\n");
	if (type == ERR || type == WARN)  ++cntWarn;  // count serious only
}
	
void CGui::WarningsCheck(const Scene* sc, const SplineRoad* road)
{
	if (!edWarn && !logWarn)  return;
	cntWarn = 0;
	if (!logWarn)
		edWarn->setCaption("");
	
	bool hqTerrain=0, hqGrass=0, hqVeget=0, hqRoad=0;  // high quality

	if (road && road->getNumPoints() > 2)
	{
		///-  start  -------------
		int cnt = road->getNumPoints();
		Vector3 stPos = Axes::toOgre(scn->sc->startPos[0]);  // todo: [1] too?
		Quaternion q1 = Axes::toOgre(scn->sc->startRot[0]);
		Vector3 vx,vy,vz;  q1.ToAxes(vx,vy,vz);  Vector3 stDir = -vx;
		Plane stPla(stDir, stPos);

		int num = road->getNumPoints();
		int iP1 = -1;  // find 1st chk id
		for (int i=0; i < num; ++i)
			if (road->mP[i].chk1st)
				iP1 = i;

		if (iP1 >= 0 && iP1 < cnt  && road->mP[iP1].chkR >= 1.f)
		{
			Vector3 ch1 = road->mP[iP1].pos;
			float d1 = stPla.getDistance(ch1);
			Warn(TXT,"Car start to 1st check distance: "+fToStr(d1,2,4));
			if (d1 < 0.f)
				Warn(WARN,"Car start isn't facing first checkpoint\n (wrong direction or first checkpoint), distance: "+fToStr(d1,2,4));
			//Warn(NOTE,"Check1 pos "+fToStr(ch0.x,2,5)+" "+fToStr(ch0.y,2,5)+" "+fToStr(ch0.z,2,5));

			//-  road dir  ----
			Vector3 pPrev = road->mP[(iP1 - road->iDir + cnt) % cnt].pos;
			float dPrev = stPla.getDistance(pPrev), diff = d1-dPrev;

			Warn(TXT,"Distance between 1st check and its prev point: "+fToStr(diff,2,4));
			if (diff < 0.f)
				Warn(WARN,"Road dir check wrong, road dir is likely opposite");
		}
		//Warn(TXT,"Start pos "+fToStr(stPos.x,2,5)+" "+fToStr(stPos.y,2,5)+" "+fToStr(stPos.z,2,5));
		//Warn(TXT,"Start dir "+fToStr(vx.x,3,5)+" "+fToStr(vx.y,3,5)+" "+fToStr(vx.z,3,5));


		//-  start pos  ----
		float tws = 0.5f * sc->td.fTerWorldSize;
		if (stPos.x < -tws || stPos.x > tws || stPos.z < -tws || stPos.z > tws)
			Warn(ERR,"Car start outside track area  Whoa :o");
		
		if (scn->terrain)  // won't work in tool..
		{	float yt = scn->terrain->getHeightAtWorldPosition(stPos), yd = stPos.y - yt - 0.5f;
			//Warn(TXT,"Car start to terrain distance "+fToStr(yd,1,4));
			if (yd < 0.f)   Warn(ERR, "Car start below terrain  Whoa :o");
			if (yd > 0.3f)  Warn(INFO,"Car start far above terrain\n (skip this if on bridge or in pipe), distance: "+fToStr(yd,1,4));
		}
		

		//-  other start places inside terrain (split screen)  ----
		if (scn->terrain)  // won't work in tool..
		for (int i=1; i<4; ++i)
		{
			Vector3 p = stPos + i * stDir * 6.f;  //par dist
			float yt = scn->terrain->getHeightAtWorldPosition(p), yd = p.y - yt - 0.5f;
			String si = toStr(i);
							Warn(TXT, "Car "+si+" start to ter dist "+fToStr(yd,1,4));
			//if (yd < 0.f)   Warn(WARN,"Car "+si+" start below terrain !");  // moved above in game
			if (yd > 0.3f)  Warn(INFO,"Car "+si+" start far above terrain");//\n (skips bridge/pipe), distance: "+fToStr(yd,1,4));
		}/**/
		
		
		//-  first chk  ----
		if (iP1 < 0 || iP1 >= cnt)
			Warn(ERR,"First checkpoint not set  (use ctrl-0)");
		else
		if (road->mP[iP1].chkR < 0.1f)
			Warn(ERR,"First checkpoint not set  (use ctrl-0)");

		
		///-  road, checkpoints  -------------
		int numChks = 0, iClosest=-1;  float stD = FLT_MAX;
		bool mtrUsed[4]={0,0,0,0};
		for (int i=0; i < road->mP.size(); ++i)
		{
			const SplinePoint& p = road->mP[i];
			if (p.chkR > 0.f && p.chkR < 1.f)
				Warn(WARN,"Too small checkpoint at road point "+toStr(i+1)+", chkR = "+fToStr(p.chkR,1,3));
			//.. in pipe > 2.f on bridge = 1.f
			
			if (p.chkR >= 1.f)
			{	++numChks;
				float d = stPos.squaredDistance(p.pos);
				if (d < stD) {  stD = d;  iClosest = i;  }
			}
			if (p.idMtr >= 0 && p.idMtr < 4)
				mtrUsed[p.idMtr] = true;
		}
		if (numChks==0)
			Warn(ERR,"No checkpoints set  (use K,L on some road points)");
		if (numChks < 3)
			Warn(INFO,"Too few checkpoints (add more), count "+toStr(numChks));

			
		//-  road materials used  ----
		int rdm = 0;
		for (int i=0; i<4; ++i)
			if (mtrUsed[i])  ++rdm;
		
		Warn(TXT,"Road materials used "+toStr(rdm));
		hqRoad = rdm >= 3;
		if (hqTerrain) Warn(INFO,"HQ Road");
		if (rdm <= 1)  Warn(INFO,"Too few road materials used");
		

		//-  start width, height  ----
		float width = road->vStBoxDim.z, height = road->vStBoxDim.y;

		float rdW = 100.f;
		if (iClosest >= 0)  {  rdW = road->mP[iClosest].width;
			Warn(TXT,"Closest road point width: "+fToStr(rdW,1,4)+",  distance "+fToStr(stPos.distance(road->mP[iClosest].pos),0,3));
		}
		if (width < 8.f || width < rdW * 1.4f)  //par, on bridge ok, pipe more..
			Warn(WARN,"Car start width small "+fToStr(width,0,2));
		if (height < 4.5f)
			Warn(WARN,"Car start height small "+fToStr(height,0,2));


		//-  rd, chk cnt  ----
		float ratio = float(numChks)/cnt;
		Warn(TXT,"Road points to checkpoints ratio: "+fToStr(ratio,2,4));
		if (ratio < 1.f/10.f)  //par
			Warn(WARN,"Extremely low checkpoints ratio, add more");
		else if (ratio < 1.f/5.f)  //par  1 chk for 5 points
			Warn(WARN,"Very few checkpoints ratio, add more");
		
		//  road points too far
		float len = road->st.Length;
		float ptLen = len/float(cnt);
		Warn(TXT,"Road length: "+fToStr(len,0,4)+ " points to length ratio: "+fToStr(ptLen,2,4));
		if (ptLen > 85.f)
			Warn(WARN,"Road points (on average) are very far\n (corners could behave sharp and straights become not straight).");
		else if (ptLen > 60.f)
			Warn(INFO,"Road points are far.");

		//  big road, merge len
		if (cnt > 200 && road->g_MergeLen < 600.f)
			Warn(INFO,"Road has over 200 points, use recommended merge length 600 or more.");
		else if (cnt > 120 && road->g_MergeLen < 300.f)
			Warn(INFO,"Road has over 120 points, use recommended merge length 300 or more.");
		else if (cnt > 50 && road->g_MergeLen < 80.f)
			Warn(INFO,"Road has over 50 points, use recommended merge length 80 or more.");
	}
	
	///-  heightmap  -------------
	int sz = sc->td.iVertsX * sc->td.iVertsX * sizeof(float) / 1024/1024;
	if (sc->td.iVertsX > 2000)
		Warn(ERR,"Using too big heightmap "+toStr(sc->td.iVertsX-1)+", file size is "+toStr(sz)+" MB");
	else
	if (sc->td.iVertsX > 1000)
		Warn(INFO,"Using big heightmap "+toStr(sc->td.iVertsX-1)+", file size is "+toStr(sz)+" MB");

	if (sc->td.iVertsX < 200)
		Warn(INFO,"Using too small heightmap "+toStr(sc->td.iVertsX-1));

	//-  tri size  ----
	if (sc->td.fTriangleSize < 0.9f)
		Warn(INFO,"Terrain triangle size is small "+fToStr(sc->td.fTriangleSize,2,4));

	if (sc->td.fTriangleSize > 1.9f)
		Warn(INFO,"Terrain triangle size is big "+fToStr(sc->td.fTriangleSize,2,4)+", not recommended");

		
	///-  ter layers  -------------
	int lay = sc->td.layers.size();  //SetUsedStr
	Warn(NOTE,"Terrain layers used: "+toStr(lay));
	hqTerrain = lay >= 4;
	if (hqTerrain) Warn(INFO,"HQ Terrain");
	if (lay >= 5)  Warn(ERR,"Too many terrain layers used, max 4 are supported.");
	if (lay <= 2)  Warn(INFO,"Too few terrain layers used");

	
	///-  vegetation  -------------
	int veg = sc->densTrees > 0.f ? sc->pgLayers.size() : 0;
	Warn(NOTE,"Vegetation models used: "+toStr(veg));
	hqVeget = veg >= 5;
	if (hqVeget)   Warn(INFO,"HQ Vegetation");
	if (veg >= 7)  Warn(WARN,"Too many models used, not recommended");
	if (veg <= 2)  Warn(INFO,"Too few models used");
	
	//-  density  ----
	if (sc->densTrees > 3.1f)
		Warn(ERR,"Vegetation use is huge, trees density is "+fToStr(sc->densTrees,1,3));
	else
	if (sc->densTrees > 2.f)
		Warn(WARN,"Using a lot of vegetation, trees density is "+fToStr(sc->densTrees,1,3));
	
	if (sc->grDensSmooth > 10)
		Warn(WARN,"Smooth grass density is high "+toStr(sc->grDensSmooth)+" saving will take long time");

	//-  grass  ----
	int gr=0;
	if (sc->densGrass > 0.01)  for (int i=0; i < sc->ciNumGrLay; ++i)  if (sc->grLayersAll[i].on)  ++gr;
	Warn(NOTE,"Grass layers used: "+toStr(gr));
	hqGrass = gr >= 4;
	if (hqGrass)  Warn(INFO,"HQ Grass");
	if (gr >= 5)  Warn(WARN,"Too many grasses used, not recommended");
	if (gr <= 2)  Warn(INFO,"Too few grasses used");

	//..  page size small, dist big
	

	///-  quality (optym, fps drop)  --------
	int hq=0;
	if (hqTerrain) ++hq;  if (hqGrass) ++hq;  if (hqVeget) ++hq;  if (hqRoad) ++hq;
	Warn(NOTE,"HQ Overall: "+toStr(hq));
	if (hq > 3)
		Warn(INFO,"Quality too high (possibly low Fps), try to reduce densities or layers/models/grasses count");
	else if (hq > 2)
		Warn(INFO,"Great quality, but don't forget about some optimisations");
	else if (hq == 0)
		Warn(INFO,"Low quality (ignore for deserts), try to add some layers/models/grasses");
	
	//..  scID light diff ?
	//..  objects count
	

	///-  end  ----------------
	if (logWarn)
	{
		LogO("Warnings: "+toStr(cntWarn)+"\n");
		return;
	}
	bool warn = cntWarn > 0;
	if (!warn)
		Warn(NOTE,"#A0C0FF""No warnings.");
	else  //  show warn overlay
		txWarn->setCaption(TR("#{Warnings}: ")+toStr(cntWarn)+"  (alt-J)");

	txWarn->setVisible(warn);
	imgInfo->setVisible(!warn);  imgWarn->setVisible(warn);
}
