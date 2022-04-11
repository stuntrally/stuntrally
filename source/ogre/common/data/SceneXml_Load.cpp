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


//  old
bool Scene::LoadStartPos(String file)
{
	CONFIGFILE param;
	if (!param.Load(file))
		return false;

	float f3[3], f1;
	QUATERNION <float> fixer;  fixer.Rotate(3.141593, 0,0,1);
	
	param.GetParam("start position 0", f3);
	MATHVECTOR <float, 3> pos(f3[2], f3[0], f3[1]);

	if (!param.GetParam("start orientation-xyz 0", f3))
		return false;

	if (!param.GetParam("start orientation-w 0", f1))
		return false;

	QUATERNION <float> rot(f3[2], f3[0], f3[1], f1);
	rot = fixer * rot;

	startPos = pos;
	startRot = rot;
	return true;
}


///  Color  . . . . . . . .
SColor::SColor()
	:h(0.f), s(0.f), v(0.f), a(0.f), n(0.f)
{	}
SColor::SColor(float h1, float s1, float v1, float a1, float n1)
	:h(h1), s(s1), v(v1), a(a1), n(n1)
{	}

//  tool check err
string SColor::Check(string t)
{
	string e;
	if (h > 1.f)  e += " h>1";  if (h < 0.f)  e += " h<0";
	if (s > 1.f)  e += " s>1";  if (s < 0.f)  e += " s<0";
	if (v > 3.f)  e += " v>3";  if (v < 0.f)  e += " v<0";
	if (a > 2.f)  e += " a>2";  if (a < 0.f)  e += " a<0";
	if (n > 1.f)  e += " n>1";  if (n < 0.f)  e += " n<0";
	if (!e.empty())  e += "  " + t + "  " + Save();
	return e;
}

//  load from old rgb
void SColor::LoadRGB(Vector3 rgb)
{
	Vector3 u = rgb;
    float vMin = std::min(u.x, std::min(u.y, u.z));
    n = vMin < 0.f ? -vMin : 0.f;  // neg = minimum  only for negative colors
    if (vMin < 0.f)  u += Vector3(n,n,n);  // cancel, normalize to 0

    float vMax = std::max(u.x, std::max(u.y, u.z));  // get max for above 1 colors
    v = vMax;
    if (vMax > 1.f)  u /= v;  // above, normalize to 1

    ColourValue cl(u.x, u.y, u.z);  // get hue and sat
    float vv;  // not important or 1
    cl.getHSB(&h, &s, &vv);
}	

//  get clr
Vector3 SColor::GetRGB1() const
{
	ColourValue cl;
	cl.setHSB(h, s, 1.f);
	float vv = std::min(1.f, v);  // * (1.f - n)
	return Vector3(
		cl.r * vv,
		cl.g * vv,
		cl.b * vv);
}
Vector3 SColor::GetRGB() const
{
	ColourValue cl;
	cl.setHSB(h, s, 1.f);
	return Vector3(
		cl.r * v -n,
		cl.g * v -n,
		cl.b * v -n);
}
ColourValue SColor::GetClr() const
{
	Vector3 c = SColor::GetRGB();
	return ColourValue(c.x, c.y, c.z);
}
Vector4 SColor::GetRGBA() const
{
	Vector3 c = GetRGB();
	return Vector4(c.x, c.y, c.z, a);
}

//  string
void SColor::Load(const char* ss)
{
	h=0.f; s=1.f; v=1.f; a=1.f; n=0.f;
	int i = sscanf(ss, "%f %f %f %f %f", &h,&s,&v,&a,&n);
	if (i == 5)
		return;  // new

	if (i < 3 || i > 5)
	{	LogO("NOT 3..5 components color!");  }

	//  rgb old < 2.4
	//float r=h,g=s,b=v,u=a;
	LoadRGB(Vector3(h,s,v));

	//  test back
	/*Vector4 c = GetRGBA();
	float d = fabs(c.x-r) + fabs(c.y-g) + fabs(c.z-b) + fabs(c.w-u);
	LogO(String("CLR CHK ")+ss);
	LogO("CLR CHK r "+fToStr(c.x-r,2,5)+" g "+fToStr(c.y-g,2,5)+" b "+fToStr(c.z-b,2,5)+" a "+fToStr(c.w-u,2,5)
		+"  d "+fToStr(d,2,4) + (d > 0.01f ? " !!! BAD " : " ok"));/**/
}

string SColor::Save() const
{
	string ss = fToStr(h,3,5)+" "+fToStr(s,3,5)+" "+fToStr(v,3,5)+" "+fToStr(std::min(3.f,a),3,5)+" "+fToStr(n,3,5);
	return ss;
}


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
	{	a = e->Attribute("pos");		if (a)  {  Vector3 v = s2v(a);   startPos = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = e->Attribute("rot");		if (a)  {  Vector4 v = s2v4(a);  startRot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
	}else
	{	LogO("!Old, loading start from track.txt");
		String s = StringUtil::replaceAll(file,"scene.xml","track.txt");
		if (!LoadStartPos(s))
			LogO("!Error: Can't load start from "+s);
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
			a = u->Attribute("size");	if (a)  p.size = s2v(a);
			a = u->Attribute("up");		if (a)  p.up = s2v(a);
			a = u->Attribute("rot");	if (a)  p.rot = s2r(a);
			a = u->Attribute("rate");	if (a)  p.rate = s2r(a);

			emitters.push_back(p);
			u = u->NextSiblingElement("e");
	}	}

	UpdateFluidsId();

	UpdateSurfId();
	
	return true;
}
