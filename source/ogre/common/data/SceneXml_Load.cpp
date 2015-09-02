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
	
	UpdateFluidsId();

	UpdateSurfId();
	
	return true;
}


//  Save
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::SaveXml(String file)
{
	TiXmlDocument xml;	TiXmlElement root("scene");

	TiXmlElement ver("ver");
		int v = SET_VER;
		ver.SetAttribute("num",		toStrC( v ));
		ver.SetAttribute("baseTrk",	baseTrk.c_str());
		ver.SetAttribute("secEd",	toStrC( secEdited ));
	root.InsertEndChild(ver);


	TiXmlElement car("car");
		car.SetAttribute("tires",	asphalt ? "1":"0");
		if (damageMul != 1.f)
			car.SetAttribute("damage",	toStrC( damageMul ));
		if (!td.road1mtr)
			car.SetAttribute("road1mtr", td.road1mtr ? "1":"0");
		if (noWrongChks)
			car.SetAttribute("noWrongChks", noWrongChks ? "1":"0");

		if (denyReversed)
			car.SetAttribute("denyRev",	"1");
		if (gravity != 9.81f)
			car.SetAttribute("gravity",	toStrC( gravity ));
	root.InsertEndChild(car);


	TiXmlElement st("start");
		string s = toStr(startPos[0])+" "+toStr(startPos[1])+" "+toStr(startPos[2]);
		st.SetAttribute("pos",	s.c_str());

		s = toStr(startRot[0])+" "+toStr(startRot[1])+" "+toStr(startRot[2])+" "+toStr(startRot[3]);
		st.SetAttribute("rot",	s.c_str());
	root.InsertEndChild(st);


	TiXmlElement snd("sound");
		snd.SetAttribute("ambient",		sAmbient.c_str());
		snd.SetAttribute("reverbs",		sReverbs.c_str());
	root.InsertEndChild(snd);
	

	TiXmlElement sky("sky");
		sky.SetAttribute("material",	skyMtr.c_str());
		if (rainEmit > 0 && rainName != "")
		{	sky.SetAttribute("rainName",	rainName.c_str());
			sky.SetAttribute("rainEmit",	toStrC( rainEmit ));
		}
		if (rain2Emit > 0 && rain2Name != "")
		{	sky.SetAttribute("rain2Name",	rain2Name.c_str());
			sky.SetAttribute("rain2Emit",	toStrC( rain2Emit ));
		}
		if (windAmt != 0.f)
			sky.SetAttribute("windAmt",	toStrC( windAmt ));
		if (skyYaw != 0.f)
			sky.SetAttribute("skyYaw",	toStrC( skyYaw ));
	root.InsertEndChild(sky);

	TiXmlElement fog("fog");
		fog.SetAttribute("color",		fogClr.Save().c_str() );
		fog.SetAttribute("color2",		fogClr2.Save().c_str() );
		fog.SetAttribute("linStart",	toStrC( fogStart ));
		fog.SetAttribute("linEnd",		toStrC( fogEnd ));
	root.InsertEndChild(fog);

	TiXmlElement fogH("fogH");
		fogH.SetAttribute("color",		fogClrH.Save().c_str() );
		fogH.SetAttribute("height",		toStrC( fogHeight ));
		fogH.SetAttribute("dens",		toStrC( fogHDensity ));
		fogH.SetAttribute("linStart",	toStrC( fogHStart ));
		fogH.SetAttribute("linEnd",		toStrC( fogHEnd ));
		if (fHDamage > 0.f)
			fogH.SetAttribute("dmg",	toStrC( fHDamage ));
	root.InsertEndChild(fogH);

	TiXmlElement li("light");
		li.SetAttribute("pitch",		toStrC( ldPitch ));
		li.SetAttribute("yaw",			toStrC( ldYaw ));
		li.SetAttribute("ambient",		lAmb.Save().c_str() );
		li.SetAttribute("diffuse",		lDiff.Save().c_str() );
		li.SetAttribute("specular",		lSpec.Save().c_str() );
	root.InsertEndChild(li);
	

	TiXmlElement fls("fluids");
		for (int i=0; i < fluids.size(); ++i)
		{
			const FluidBox* fb = &fluids[i];
			TiXmlElement fe("fluid");
			fe.SetAttribute("name",		fb->name.c_str() );
			fe.SetAttribute("pos",		toStrC( fb->pos ));
			fe.SetAttribute("rot",		toStrC( fb->rot ));
			fe.SetAttribute("size",		toStrC( fb->size ));
			fe.SetAttribute("tile",		toStrC( fb->tile ));
			fls.InsertEndChild(fe);
		}
	root.InsertEndChild(fls);


	TiXmlElement ter("terrain");
		ter.SetAttribute("size",		toStrC( td.iVertsX ));
		ter.SetAttribute("triangle",	toStrC( td.fTriangleSize ));
		ter.SetAttribute("errNorm",		fToStr( td.errorNorm, 2,4 ).c_str());
		if (td.normScale != 1.f)
			ter.SetAttribute("normSc",		toStrC( td.normScale ));
		if (td.emissive)
			ter.SetAttribute("emissive",	td.emissive ? 1 : 0);
		if (td.specularPow != 32.f)
			ter.SetAttribute("specPow",		toStrC( td.specularPow ));
		if (td.specularPowEm != 2.f)
			ter.SetAttribute("specPowEm",	toStrC( td.specularPowEm ));

		const TerLayer* l;
		for (int i=0; i < 6; ++i)
		{
			l = &td.layersAll[i];
			TiXmlElement tex("texture");
			tex.SetAttribute("on",		l->on ? 1 : 0);
			tex.SetAttribute("file",	l->texFile.c_str());
			tex.SetAttribute("fnorm",	l->texNorm.c_str());
			tex.SetAttribute("scale",	toStrC( l->tiling ));
			tex.SetAttribute("surf",	l->surfName.c_str());
			#define setDmst()  \
				tex.SetAttribute("dust",	toStrC( l->dust ));  \
				tex.SetAttribute("dustS",	toStrC( l->dustS )); \
				tex.SetAttribute("mud",		toStrC( l->mud ));   \
				tex.SetAttribute("smoke",	toStrC( l->smoke )); \
				tex.SetAttribute("tclr",	l->tclr.Save().c_str() );
			setDmst();
			if (l->fDamage > 0.f)
				tex.SetAttribute("dmg",	toStrC( l->fDamage ));

			tex.SetAttribute("angMin",	toStrC( l->angMin ));
			tex.SetAttribute("angMax",	toStrC( l->angMax ));
			tex.SetAttribute("angSm",	toStrC( l->angSm ));
			tex.SetAttribute("hMin",	toStrC( l->hMin ));
			tex.SetAttribute("hMax",	toStrC( l->hMax ));
			tex.SetAttribute("hSm",		toStrC( l->hSm ));

			tex.SetAttribute("nOn",		l->nOnly ? 1 : 0);
			if (l->triplanar)  tex.SetAttribute("triplanar", 1);

			tex.SetAttribute("noise",	toStrC( l->noise ));
			tex.SetAttribute("n_1",		toStrC( l->nprev ));
			tex.SetAttribute("n2",		toStrC( l->nnext2 ));

			TiXmlElement noi("noise");
			for (int n=0; n < 2; ++n)
			{	string sn = toStr(n), s;
				s = "frq"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nFreq[n] ));
				s = "oct"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nOct[n] ));
				s = "prs"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPers[n] ));
				s = "pow"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPow[n] ));
			}
			tex.InsertEndChild(noi);
			ter.InsertEndChild(tex);
		}
		for (int i=0; i < 4; ++i)
		{
			l = &td.layerRoad[i];
			TiXmlElement tex("texture");
			tex.SetAttribute("road",	toStrC(i+1));
			tex.SetAttribute("surf",	l->surfName.c_str());
			setDmst();
			ter.InsertEndChild(tex);
		}
		
		TiXmlElement par("par");
			par.SetAttribute("dust",	sParDust.c_str());
			par.SetAttribute("mud",		sParMud.c_str());
			par.SetAttribute("smoke",	sParSmoke.c_str());
		ter.InsertEndChild(par);

	root.InsertEndChild(ter);
	

	TiXmlElement pgd("paged");
		pgd.SetAttribute("densGrass",	toStrC( densGrass ));
		pgd.SetAttribute("densTrees",	toStrC( densTrees ));
		//  grass
		pgd.SetAttribute("grPage",		toStrC( grPage ));
		pgd.SetAttribute("grDist",		toStrC( grDist ));
		pgd.SetAttribute("grDensSmooth",toStrC( grDensSmooth ));

		//  trees
		pgd.SetAttribute("trPage",		toStrC( trPage ));
		pgd.SetAttribute("trDist",		toStrC( trDist ));
		pgd.SetAttribute("trDistImp",	toStrC( trDistImp ));
		pgd.SetAttribute("trRdDist",	toStrC( trRdDist  ));

		int i;
		for (int i=0; i < ciNumGrLay; ++i)
		{
			const SGrassLayer& g = grLayersAll[i];
			TiXmlElement grl("grass");
			grl.SetAttribute("on",		g.on ? 1 : 0);
			grl.SetAttribute("mtr",		g.material.c_str());
			grl.SetAttribute("clr",		g.colorMap.c_str());
			grl.SetAttribute("dens",	toStrC( g.dens ));
			grl.SetAttribute("chan",	toStrC( g.iChan ));

			grl.SetAttribute("minSx",	toStrC( g.minSx ));
			grl.SetAttribute("maxSx",	toStrC( g.maxSx ));
			grl.SetAttribute("minSy",	toStrC( g.minSy ));
			grl.SetAttribute("maxSy",	toStrC( g.maxSy ));

			grl.SetAttribute("swayDistr",	toStrC( g.swayDistr ));
			grl.SetAttribute("swayLen",		toStrC( g.swayLen ));
			grl.SetAttribute("swaySpeed",	toStrC( g.swaySpeed ));
			pgd.InsertEndChild(grl);
		}

		for (i=0; i < 4; ++i)
		{
			const SGrassChannel& g = grChan[i];
			TiXmlElement gch("gchan");
			gch.SetAttribute("amin",	toStrC( g.angMin ));
			gch.SetAttribute("amax",	toStrC( g.angMax ));
			gch.SetAttribute("asm",		toStrC( g.angSm ));

			gch.SetAttribute("hmin",	toStrC( g.hMin ));
			gch.SetAttribute("hmax",	toStrC( g.hMax ));
			gch.SetAttribute("hsm",		toStrC( g.hSm ));

			gch.SetAttribute("ns",		toStrC( g.noise ));
			gch.SetAttribute("frq",		toStrC( g.nFreq ));
			gch.SetAttribute("oct",		toStrC( g.nOct ));
			gch.SetAttribute("prs",		toStrC( g.nPers ));
			gch.SetAttribute("pow",		toStrC( g.nPow ));

			gch.SetAttribute("rd",		toStrC( g.rdPow ));
			pgd.InsertEndChild(gch);
		}

		for (i=0; i < ciNumPgLay; ++i)
		{
			const PagedLayer& l = pgLayersAll[i];
			TiXmlElement pgl("layer");
			pgl.SetAttribute("on",			l.on ? 1 : 0);
			pgl.SetAttribute("name",		l.name.c_str());
			pgl.SetAttribute("dens",		toStrC( l.dens ));
			pgl.SetAttribute("minScale",	toStrC( l.minScale ));
			pgl.SetAttribute("maxScale",	toStrC( l.maxScale ));

			pgl.SetAttribute("ofsY",		toStrC( l.ofsY ));
			pgl.SetAttribute("addTrRdDist",	toStrC( l.addRdist ));
			pgl.SetAttribute("maxRdist",	toStrC( l.maxRdist ));
			pgl.SetAttribute("windFx",		toStrC( l.windFx ));
			pgl.SetAttribute("windFy",		toStrC( l.windFy ));

			pgl.SetAttribute("maxTerAng",	toStrC( l.maxTerAng ));
			pgl.SetAttribute("minTerH",		toStrC( l.minTerH ));
			pgl.SetAttribute("maxTerH",		toStrC( l.maxTerH ));
			pgl.SetAttribute("maxDepth",	toStrC( l.maxDepth ));
			pgd.InsertEndChild(pgl);
		}
	root.InsertEndChild(pgd);


	TiXmlElement cam("cam");
		cam.SetAttribute("pos",		toStrC( camPos ));
		cam.SetAttribute("dir",		toStrC( camDir ));
	root.InsertEndChild(cam);


	TiXmlElement objs("objects");
		for (i=0; i < objects.size(); ++i)
		{
			const Object* o = &objects[i];
			TiXmlElement oe("o");
			oe.SetAttribute("name",		o->name.c_str() );

			string s = toStr(o->pos[0])+" "+toStr(o->pos[1])+" "+toStr(o->pos[2]);
			oe.SetAttribute("pos",		s.c_str());

			s = toStr(o->rot[0])+" "+toStr(o->rot[1])+" "+toStr(o->rot[2])+" "+toStr(o->rot[3]);
			oe.SetAttribute("rot",		s.c_str());

			if (o->scale != Vector3::UNIT_SCALE)  // dont save default
				oe.SetAttribute("sc",	toStrC( o->scale ));
			objs.InsertEndChild(oe);
		}
	root.InsertEndChild(objs);


	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}


///  Load Presets
//--------------------------------------------------------------------------------------------------------------------------------------

bool Presets::LoadXml(string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)
	{	LogO("!Can't load presets.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	ter.clear();  iter.clear();
	rd.clear();  ird.clear();
	gr.clear();  igr.clear();
	veg.clear();  iveg.clear();

	//  read
	XMLElement* eSky,*eTex,*eRd,*eVeg,*eGr;
	const char* a;

	///  sky
	eSky = root->FirstChildElement("s");
	while (eSky)
	{
		PSky s;
		a = eSky->Attribute("m");	if (a)  s.mtr = String(a);
		a = eSky->Attribute("c");	if (a)  s.clr = String(a);

		a = eSky->Attribute("y");	if (a)  s.ldYaw = s2r(a);
		a = eSky->Attribute("p");	if (a)  s.ldPitch = s2r(a);

		sky.push_back(s);  isky[s.mtr] = sky.size();
		eSky = eSky->NextSiblingElement("s");
	}

	///  terrain
	eTex = root->FirstChildElement("t");
	while (eTex)
	{
		PTer l;
		a = eTex->Attribute("t");	if (a)  l.texFile = String(a);
		a = eTex->Attribute("n");	if (a)  l.texNorm = String(a);
		a = eTex->Attribute("s");	if (a)  l.tiling = s2r(a);
		a = eTex->Attribute("su");	if (a)  l.surfName = string(a);

		a = eTex->Attribute("sc");	if (a)  l.sc = String(a);
		a = eTex->Attribute("z");	if (a)  l.scn = string(a);

		a = eTex->Attribute("du");	if (a)  l.dust = s2r(a);
		a = eTex->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = eTex->Attribute("md");	if (a)  l.mud = s2r(a);
		a = eTex->Attribute("tr");	if (a)  l.tclr.Load(a);
		a = eTex->Attribute("d");	if (a)  l.dmg = s2r(a);

		a = eTex->Attribute("aa");	if (a)  l.angMin = s2r(a);
		a = eTex->Attribute("ab");	if (a)  l.angMax = s2r(a);
		a = eTex->Attribute("tp");	if (a)  l.triplanar = s2i(a)>0;

		ter.push_back(l);  iter[l.texFile] = ter.size();
		eTex = eTex->NextSiblingElement("t");
	}

	///  road
	eRd = root->FirstChildElement("r");
	while (eRd)
	{
		PRoad l;
		a = eRd->Attribute("m");	if (a)  l.mtr = String(a);
		a = eRd->Attribute("su");	if (a)  l.surfName = string(a);

		a = eRd->Attribute("sc");	if (a)  l.sc = String(a);
		a = eRd->Attribute("z");	if (a)  l.scn = string(a);

		a = eRd->Attribute("du");	if (a)  l.dust = s2r(a);
		a = eRd->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = eRd->Attribute("md");	if (a)  l.mud = s2r(a);
		a = eRd->Attribute("tr");	if (a)  l.tclr.Load(a);

		rd.push_back(l);  ird[l.mtr] = rd.size();
		eRd = eRd->NextSiblingElement("r");
	}
		
	///  grass
	eGr = root->FirstChildElement("g");
	while (eGr)
	{
		PGrass g;
		a = eGr->Attribute("g");	if (a)  g.mtr = String(a);
		a = eGr->Attribute("c");	if (a)  g.clr = String(a);

		a = eGr->Attribute("sc");	if (a)  g.sc = String(a);
		a = eGr->Attribute("z");	if (a)  g.scn = string(a);

		a = eGr->Attribute("xa");	if (a)  g.minSx = s2r(a);
		a = eGr->Attribute("xb");	if (a)  g.maxSx = s2r(a);
		a = eGr->Attribute("ya");	if (a)  g.minSy = s2r(a);
		a = eGr->Attribute("yb");	if (a)  g.maxSy = s2r(a);

		gr.push_back(g);  igr[g.mtr] = gr.size();
		eGr = eGr->NextSiblingElement("g");
	}
	
	///  veget
 	eVeg = root->FirstChildElement("v");
	while (eVeg)
	{
		PVeget l;
		a = eVeg->Attribute("p");	if (a)  l.name = String(a);
		a = eVeg->Attribute("sa");	if (a)  l.minScale = s2r(a);
		a = eVeg->Attribute("sb");	if (a)  l.maxScale = s2r(a);

		a = eVeg->Attribute("sc");	if (a)  l.sc = String(a);
		a = eVeg->Attribute("z");	if (a)  l.scn = string(a);

		a = eVeg->Attribute("wx");	if (a)  l.windFx = s2r(a);
		a = eVeg->Attribute("wy");	if (a)  l.windFy = s2r(a);

		a = eVeg->Attribute("ab");	if (a)  l.maxTerAng = s2r(a);
		a = eVeg->Attribute("rd");	if (a)  l.addRdist = s2i(a);
		a = eVeg->Attribute("fd");	if (a)  l.maxDepth = s2r(a);

		veg.push_back(l);  iveg[l.name] = veg.size();
		eVeg = eVeg->NextSiblingElement("v");
	}

	return true;
}
