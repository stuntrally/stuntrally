#include "pch.h"
#include "../Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"
#include <OgreSceneNode.h>
#include "../vdrift/game.h"  // for surfaces map
using namespace std;
using namespace Ogre;
using namespace tinyxml2;


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::LoadXml(String file, bool bTer)
{
	XMLDocument doc;
	XMLError er = doc.LoadFile(file.c_str());
	if (er != XML_SUCCESS)
	{	LogO("!Error: Can't load scene.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	// clear  --
	Default();

	//  terrain, asphalt set, vdr outside
	asphalt = !bTer;
	ter = bTer;

	//td.layers.clear();
	//pgLayers.clear();

	// read
	XMLElement* e, *u;
	const char* a;


 	///  ed version
 	int ver = 2300;  // old
 	e = root->FirstChildElement("ver");
	if (e)
	{	a = e->Attribute("num");		if (a)  ver = s2i(a);
		a = e->Attribute("baseTrk");	if (a)  baseTrk = string(a);
		a = e->Attribute("secEd");		if (a)  secEdited = s2i(a);
	}
	
 	///  car setup
 	e = root->FirstChildElement("car");
	if (e)
	{	a = e->Attribute("tires");		if (a)  asphalt = s2i(a) > 0;
		a = e->Attribute("damage");		if (a)  damageMul = s2r(a);
		a = e->Attribute("road1mtr");	if (a)  td.road1mtr = s2i(a) > 0;

		a = e->Attribute("denyRev");	if (a)  denyReversed = s2i(a) > 0;
		a = e->Attribute("gravity");	if (a)  gravity = s2r(a);
		a = e->Attribute("noWrongChks"); if (a)  noWrongChks = s2i(a) > 0;
	}

	///  car start
	e = root->FirstChildElement("start");
	if (e)
	{	a = e->Attribute("pos");		if (a)  {  Vector3 v = s2v(a);   startPos[0] = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = e->Attribute("rot");		if (a)  {  Vector4 v = s2v4(a);  startRot[0] = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
		a = e->Attribute("pos2");		if (a)  {  Vector3 v = s2v(a);   startPos[1] = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = e->Attribute("rot2");		if (a)  {  Vector4 v = s2v4(a);  startRot[1] = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
	}

	///  sound
	e = root->FirstChildElement("sound");
	if (e)
	{	a = e->Attribute("ambient");	if (a)  sAmbient = string(a);
		a = e->Attribute("reverbs");	if (a)  sReverbs = string(a);
		UpdRevSet();
	}

	///  sky
	e = root->FirstChildElement("sky");
	if (e)
	{	a = e->Attribute("material");	if (a)  skyMtr = String(a);
		a = e->Attribute("rainEmit");	if (a)  rainEmit = s2i(a);
		a = e->Attribute("rainName");	if (a)  rainName = String(a);
		a = e->Attribute("rain2Emit");	if (a)  rain2Emit = s2i(a);
		a = e->Attribute("rain2Name");	if (a)  rain2Name = String(a);
		a = e->Attribute("windAmt");	if (a)  windAmt = s2r(a);
		a = e->Attribute("skyYaw");		if (a)  skyYaw = s2r(a);
	}
	///  fog
	e = root->FirstChildElement("fog");
	if (e)
	{	a = e->Attribute("linStart");	if (a)  fogStart = s2r(a);
		a = e->Attribute("linEnd");		if (a)  fogEnd = s2r(a);
		a = e->Attribute("color");		if (a)  fogClr.Load(a);
		a = e->Attribute("color2");		if (a)  fogClr2.Load(a);  else  fogClr2 = fogClr;
	}
	///  fog H
	e = root->FirstChildElement("fogH");
	if (e)
	{	a = e->Attribute("color");		if (a)  fogClrH.Load(a);
		a = e->Attribute("height");		if (a)  fogHeight = s2r(a);
		a = e->Attribute("dens");		if (a)  fogHDensity = s2r(a);
		a = e->Attribute("linStart");	if (a)  fogHStart = s2r(a);
		a = e->Attribute("linEnd");		if (a)  fogHEnd = s2r(a);
		a = e->Attribute("dmg");		if (a)  fHDamage = s2r(a);
	}

	///  light
	e = root->FirstChildElement("light");
	if (e)
	{	a = e->Attribute("pitch");		if (a)  ldPitch = s2r(a);
		a = e->Attribute("yaw");		if (a)  ldYaw = s2r(a);

		a = e->Attribute("ambient");	if (a)  lAmb.Load(a);
		a = e->Attribute("diffuse");	if (a)  lDiff.Load(a);
		a = e->Attribute("specular");	if (a)  lSpec.Load(a);
	}
	
	
	///  fluids
	e = root->FirstChildElement("fluids");
	if (e)
	{	u = e->FirstChildElement("fluid");
		while (u)
		{
			FluidBox fb;
			a = u->Attribute("name");	if (a)  fb.name = string(a);

			a = u->Attribute("pos");	if (a)  fb.pos = s2v(a);
			a = u->Attribute("rot");	if (a)  fb.rot = s2v(a);
			a = u->Attribute("size");	if (a)  fb.size = s2v(a);
			a = u->Attribute("tile");	if (a)  fb.tile = s2v2(a);

			fluids.push_back(fb);
			u = u->NextSiblingElement("fluid");
		}
	}
	
	///  terrain
	e = root->FirstChildElement("terrain");
	if (e)
	{	a = e->Attribute("size");		if (a)  td.iVertsX = s2i(a);
		a = e->Attribute("triangle");	if (a)  td.fTriangleSize = s2r(a);
		a = e->Attribute("errNorm");	if (a)  td.errorNorm = s2r(a);

		a = e->Attribute("normSc");		if (a)  td.normScale = s2r(a);
		a = e->Attribute("emissive");	if (a)  td.emissive = s2i(a)>0;
		a = e->Attribute("specPow");	if (a)  td.specularPow = s2r(a);
		a = e->Attribute("specPowEm");	if (a)  td.specularPowEm = s2r(a);
		td.UpdVals();

		int il = 0;
		u = e->FirstChildElement("texture");
		while (u)
		{
			int road = -1;
			a = u->Attribute("road");	if (a)  road = s2i(a)-1;
			bool ter = road == -1;
			
			TerLayer lay, *l = ter ? &lay : &td.layerRoad[road];
			lay.nFreq[0] += (il-0.7f) * 4.f;  // default, can't be same, needs variation
			lay.nFreq[1] += (il-0.5f) * 3.f;

			a = u->Attribute("on");		if (a)  l->on = s2i(a)>0;  else  l->on = true;
			a = u->Attribute("file");	if (a)  l->texFile = String(a);
			a = u->Attribute("fnorm");	if (a)  l->texNorm = String(a);
			a = u->Attribute("scale");	if (a)  l->tiling = s2r(a);
			a = u->Attribute("surf");	if (a)  l->surfName = String(a);

			a = u->Attribute("dust");	if (a)  l->dust = s2r(a);
			a = u->Attribute("dustS");	if (a)  l->dustS = s2r(a);
			a = u->Attribute("mud");	if (a)  l->mud = s2r(a);
			a = u->Attribute("smoke");	if (a)  l->smoke = s2r(a);
			a = u->Attribute("tclr");	if (a){ l->tclr.Load(a);  l->tcl = l->tclr.GetRGBA();  }
			a = u->Attribute("dmg");	if (a)  l->fDamage = s2r(a);

			a = u->Attribute("angMin");	if (a)  l->angMin = s2r(a);
			a = u->Attribute("angMax");	if (a)  l->angMax = s2r(a);
			a = u->Attribute("angSm");	if (a)  l->angSm = s2r(a);
			a = u->Attribute("hMin");	if (a)  l->hMin = s2r(a);
			a = u->Attribute("hMax");	if (a)  l->hMax = s2r(a);
			a = u->Attribute("hSm");	if (a)  l->hSm = s2r(a);

			a = u->Attribute("nOn");		if (a)  l->nOnly = s2i(a)>0;
			a = u->Attribute("triplanar");	if (a)  l->triplanar = true;  else  l->triplanar = false;

			a = u->Attribute("noise");	if (a)  l->noise = s2r(a);
			a = u->Attribute("n_1");	if (a)  l->nprev = s2r(a);
			a = u->Attribute("n2");		if (a)  l->nnext2 = s2r(a);

			XMLElement* eNoi = u->FirstChildElement("noise");
			if (eNoi)
			for (int n=0; n < 2; ++n)
			{	string sn = toStr(n), s;
				s = "frq"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nFreq[n]= s2r(a);
				s = "oct"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nOct[n] = s2i(a);
				s = "prs"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPers[n]= s2r(a);
				s = "pow"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPow[n] = s2r(a);
			}
			if (ter && il < td.ciNumLay)
				td.layersAll[il++] = lay;
			u = u->NextSiblingElement("texture");
		}
		td.UpdLayers();

		u = e->FirstChildElement("par");
		if (u)
		{	a = u->Attribute("dust");	if (a)  sParDust = String(a);
			a = u->Attribute("mud");	if (a)  sParMud = String(a);
			a = u->Attribute("smoke");	if (a)  sParSmoke = String(a);
		}
	}
	
	///  paged
 	e = root->FirstChildElement("paged");
	if (e)
	{	a = e->Attribute("densTrees");		if (a)  densTrees = s2r(a);
		a = e->Attribute("densGrass");		if (a)  densGrass = s2r(a);
		//  grass
		a = e->Attribute("grPage");			if (a)  grPage = s2r(a);
		a = e->Attribute("grDist");			if (a)  grDist = s2r(a);
		a = e->Attribute("grDensSmooth");	if (a)  grDensSmooth = s2i(a);
		//  trees
		a = e->Attribute("trPage");			if (a)  trPage = s2r(a);
		a = e->Attribute("trDist");			if (a)  trDist = s2r(a);
		a = e->Attribute("trDistImp");		if (a)  trDistImp = s2r(a);
		a = e->Attribute("trRdDist");		if (a)  trRdDist = s2i(a);

		int grl = 0;
		u = e->FirstChildElement("grass");
		while (u)
		{
			SGrassLayer g;
			a = u->Attribute("on");			if (a)  g.on = s2i(a);  else  g.on = 1;
			a = u->Attribute("mtr");		if (a)  g.material = String(a);
			a = u->Attribute("clr");		if (a)  g.colorMap = String(a);
			a = u->Attribute("dens");		if (a)  g.dens = s2r(a);
			a = u->Attribute("chan");		if (a)  g.iChan = s2i(a);
										
			a = u->Attribute("minSx");		if (a)  g.minSx = s2r(a);
			a = u->Attribute("maxSx");		if (a)  g.maxSx = s2r(a);
			a = u->Attribute("minSy");		if (a)  g.minSy = s2r(a);
			a = u->Attribute("maxSy");		if (a)  g.maxSy = s2r(a);

			a = u->Attribute("swayDistr");	if (a)  g.swayDistr = s2r(a);
			a = u->Attribute("swayLen");	if (a)  g.swayLen = s2r(a);
			a = u->Attribute("swaySpeed");	if (a)  g.swaySpeed = s2r(a);
			
		#if 1  //  old < 2.3  (no channels)
		if (grl == 0)  {
			a = u->Attribute("terMaxAng");	if (a)  grChan[0].angMax = s2r(a);
			a = u->Attribute("terAngSm");	if (a)  grChan[0].angSm = s2r(a);

			a = u->Attribute("terMinH");	if (a)  grChan[0].hMin = s2r(a);
			a = u->Attribute("terMaxH");	if (a)  grChan[0].hMax = s2r(a);
			a = u->Attribute("terHSm");		if (a)  grChan[0].hSm = s2r(a);  }
		#endif
			grLayersAll[grl++] = g;
			u = u->NextSiblingElement("grass");
		}

		int c;
		for (c=0; c < 4; c++)
			grChan[c].nFreq += c * 3.f;  // default variation
		c = 0;

		u = e->FirstChildElement("gchan");
		while (u && c < 4)
		{
			SGrassChannel& g = grChan[c++];
			TiXmlElement gch("gchan");

			a = u->Attribute("amin");	if (a)  g.angMin = s2r(a);
			a = u->Attribute("amax");	if (a)  g.angMax = s2r(a);
			a = u->Attribute("asm");	if (a)  g.angSm = s2r(a);

			a = u->Attribute("hmin");	if (a)  g.hMin = s2r(a);
			a = u->Attribute("hmax");	if (a)  g.hMax = s2r(a);
			a = u->Attribute("hsm");	if (a)  g.hSm = s2r(a);

			a = u->Attribute("ns");		if (a)  g.noise = s2r(a);
			a = u->Attribute("frq");	if (a)  g.nFreq = s2r(a);
			a = u->Attribute("oct");	if (a)  g.nOct  = s2i(a);
			a = u->Attribute("prs");	if (a)  g.nPers = s2r(a);
			a = u->Attribute("pow");	if (a)  g.nPow  = s2r(a);

			a = u->Attribute("rd");		if (a)  g.rdPow = s2r(a);
			u = u->NextSiblingElement("gchan");
		}
		
		///  veget
		int pgl = 0;
		u = e->FirstChildElement("layer");
		while (u)
		{
			PagedLayer l;
			a = u->Attribute("on");			if (a)  l.on = s2i(a);  else  l.on = 1;
			a = u->Attribute("name");		if (a)  l.name = String(a);
			a = u->Attribute("dens");		if (a)  l.dens = s2r(a);
			a = u->Attribute("minScale");	if (a)  l.minScale = s2r(a);
			a = u->Attribute("maxScale");	if (a)  l.maxScale = s2r(a);

			a = u->Attribute("ofsY");		if (a)  l.ofsY = s2r(a);
			a = u->Attribute("addTrRdDist");if (a)  l.addRdist = s2i(a);
			a = u->Attribute("maxRdist");	if (a)  l.maxRdist = s2i(a);
			a = u->Attribute("windFx");		if (a)  l.windFx = s2r(a);
			a = u->Attribute("windFy");		if (a)  l.windFy = s2r(a);

			a = u->Attribute("maxTerAng");	if (a)  l.maxTerAng = s2r(a);
			a = u->Attribute("minTerH");	if (a)  l.minTerH = s2r(a);
			a = u->Attribute("maxTerH");	if (a)  l.maxTerH = s2r(a);
			a = u->Attribute("maxDepth");	if (a)  l.maxDepth = s2r(a);

			pgLayersAll[pgl++] = l;
			u = u->NextSiblingElement("layer");
		}
		UpdPgLayers();
	}
	
 	///  camera
 	e = root->FirstChildElement("cam");
	if (e)
	{	a = e->Attribute("pos");		if (a)  camPos = s2v(a);
		a = e->Attribute("dir");		if (a)  camDir = s2v(a);
	}
	
	///  objects
	e = root->FirstChildElement("objects");
	if (e)
	{	u = e->FirstChildElement("o");
		while (u)
		{
			Object o;
			a = u->Attribute("name");	if (a)  o.name = string(a);

			a = u->Attribute("pos");	if (a)  {  Vector3 v = s2v(a);  o.pos = MATHVECTOR<float,3>(v.x,v.y,v.z);  }
			a = u->Attribute("rot");	if (a)  {  Vector4 v = s2v4(a);  o.rot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
			a = u->Attribute("sc");		if (a)  o.scale = s2v(a);

			objects.push_back(o);
			u = u->NextSiblingElement("o");
	}	}

	///  emitters
	e = root->FirstChildElement("emitters");
	if (e)
	{	u = e->FirstChildElement("e");
		while (u)
		{
			SEmitter p;
			a = u->Attribute("name");	if (a)  p.name = string(a);

			a = u->Attribute("pos");	if (a)  p.pos = s2v(a);
			a = u->Attribute("sc");		if (a)  p.size = s2v(a);
			a = u->Attribute("up");		if (a)  p.up = s2v(a);
			a = u->Attribute("rot");	if (a)  p.rot = s2r(a);
			a = u->Attribute("rate");	if (a)  p.rate = s2r(a);
			a = u->Attribute("st");		if (a)  p.stat = s2i(a);

			emitters.push_back(p);
			u = u->NextSiblingElement("e");
	}	}

	UpdateFluidsId();

	UpdateSurfId();
	
	return true;
}
