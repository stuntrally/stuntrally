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
bool Scene::LoadStartPos(Ogre::String file)
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


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::LoadXml(String file, bool bTer)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)
	{	LogO("!Can't load scene.xml: "+file);  return false;  }
		
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
	XMLElement* eSt,*eSky,*eFog,*eFogH,*eLi,*eTer,*ePgd,*eCam,*eFls,*eObjs,*eCar;
	const char* a;


 	///  car setup
 	eCar = root->FirstChildElement("car");
	if (eCar)
	{
		a = eCar->Attribute("tires");		if (a)  asphalt = s2i(a) > 0;
		a = eCar->Attribute("damage");		if (a)  damageMul = s2r(a);

		a = eCar->Attribute("denyRev");		if (a)  denyReversed = s2i(a) > 0;
		a = eCar->Attribute("gravity");		if (a)  gravity = s2r(a);
	}

	///  car start
	eSt = root->FirstChildElement("start");
	if (eSt)
	{
		a = eSt->Attribute("pos");		if (a)  {  Vector3 v = s2v(a);   startPos = MATHVECTOR<float,3>(v.x,v.y,v.z);    }
		a = eSt->Attribute("rot");		if (a)  {  Vector4 v = s2v4(a);  startRot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
	}else
	{	LogO("!Old, loading start from track.txt");
		String s = StringUtil::replaceAll(file,"scene.xml","track.txt");
		if (!LoadStartPos(s))
			LogO("!! Can't load start from "+s);
	}

	///  sky
	eSky = root->FirstChildElement("sky");
	if (eSky)
	{	a = eSky->Attribute("material");	if (a)  skyMtr = String(a);
		a = eSky->Attribute("rainEmit");	if (a)  rainEmit = s2i(a);
		a = eSky->Attribute("rainName");	if (a)  rainName = String(a);
		a = eSky->Attribute("rain2Emit");	if (a)  rain2Emit = s2i(a);
		a = eSky->Attribute("rain2Name");	if (a)  rain2Name = String(a);
		a = eSky->Attribute("windAmt");		if (a)  windAmt = s2r(a);
		a = eSky->Attribute("skyYaw");		if (a)  skyYaw = s2r(a);
	}
	///  fog
	eFog = root->FirstChildElement("fog");
	if (eFog)
	{
		a = eFog->Attribute("linStart");	if (a)  fogStart = s2r(a);
		a = eFog->Attribute("linEnd");		if (a)  fogEnd = s2r(a);
		a = eFog->Attribute("color");		if (a)  fogClr = s2v4(a);
		a = eFog->Attribute("color2");		if (a)  fogClr2 = s2v4(a);  else  fogClr2 = fogClr;
	}
	///  fog H
	eFogH = root->FirstChildElement("fogH");
	if (eFogH)
	{
		a = eFogH->Attribute("color");		if (a)  fogClrH = s2v4(a);
		a = eFogH->Attribute("height");		if (a)  fogHeight = s2r(a);
		a = eFogH->Attribute("dens");		if (a)  fogHDensity = s2r(a);
		a = eFogH->Attribute("linStart");	if (a)  fogHStart = s2r(a);
		a = eFogH->Attribute("linEnd");		if (a)  fogHEnd = s2r(a);
	}

	///  light
	eLi = root->FirstChildElement("light");
	if (eLi)
	{
		a = eLi->Attribute("sceneryId");	if (a)  sceneryId = std::string(a);  ///
		a = eLi->Attribute("pitch");		if (a)  ldPitch = s2r(a);
		a = eLi->Attribute("yaw");			if (a)  ldYaw = s2r(a);
		a = eLi->Attribute("dir");			if (a)  {  lDir = s2v(a);
			Vector3 d(lDir.normalisedCopy());  // old _
			ldPitch = -asin(d.y) * 180.f/PI_d;
			ldYaw = (atan2(d.z, d.x) + PI_d/2.0) * 180.f/PI_d;  if (ldYaw > 180.f)  ldYaw -= 360.f;  }
		a = eLi->Attribute("ambient");		if (a)  lAmb = s2v(a);
		a = eLi->Attribute("diffuse");		if (a)  lDiff = s2v(a);
		a = eLi->Attribute("specular");		if (a)  lSpec = s2v(a);
	}
	
	///  fluids
	eFls = root->FirstChildElement("fluids");
	if (eFls)
	{
		XMLElement* eFl = eFls->FirstChildElement("fluid");
		while (eFl)
		{
			FluidBox fb;
			a = eFl->Attribute("name");		if (a)  fb.name = std::string(a);

			a = eFl->Attribute("pos");		if (a)  fb.pos = s2v(a);
			a = eFl->Attribute("rot");		if (a)  fb.rot = s2v(a);
			a = eFl->Attribute("size");		if (a)  fb.size = s2v(a);
			a = eFl->Attribute("tile");		if (a)  fb.tile = Ogre::StringConverter::parseVector2(a);

			fluids.push_back(fb);
			eFl = eFl->NextSiblingElement("fluid");
		}
	}
	
	///  terrain
	eTer = root->FirstChildElement("terrain");
	if (eTer)
	{
		a = eTer->Attribute("size");		if (a)  td.iVertsX = s2i(a);
		a = eTer->Attribute("triangle");	if (a)  td.fTriangleSize = s2r(a);
		a = eTer->Attribute("errNorm");		if (a)  td.errorNorm = s2r(a);

		a = eTer->Attribute("normSc");		if (a)  td.normScale = s2r(a);
		a = eTer->Attribute("emissive");	if (a)  td.emissive = s2i(a)>0;
		a = eTer->Attribute("specPow");		if (a)  td.specularPow = s2r(a);
		a = eTer->Attribute("specPowEm");	if (a)  td.specularPowEm = s2r(a);
		td.UpdVals();

		int il = 0;
		XMLElement* eTex = eTer->FirstChildElement("texture");
		while (eTex)
		{
			bool road = false;
			a = eTex->Attribute("road");	if (a)  if (s2i(a)==1)  road = true;
			
			TerLayer lay, *l = road ? &td.layerRoad : &lay;
			lay.nFreq[0] += (il-0.7f) * 4.f;  // default, can't be same, needs variation
			lay.nFreq[1] += (il-0.5f) * 3.f;

			a = eTex->Attribute("on");		if (a)  l->on = s2i(a)>0;  else  l->on = true;
			a = eTex->Attribute("file");	if (a)  l->texFile = String(a);
			a = eTex->Attribute("fnorm");	if (a)  l->texNorm = String(a);
			a = eTex->Attribute("scale");	if (a)  l->tiling = s2r(a);
			a = eTex->Attribute("surf");	if (a)  l->surfName = String(a);

			a = eTex->Attribute("dust");	if (a)  l->dust = s2r(a);
			a = eTex->Attribute("dustS");	if (a)  l->dustS = s2r(a);
			a = eTex->Attribute("mud");		if (a)  l->mud = s2r(a);
			a = eTex->Attribute("smoke");	if (a)  l->smoke = s2r(a);
			a = eTex->Attribute("tclr");	if (a)  l->tclr = s2c(a);

			a = eTex->Attribute("angMin");	if (a)  l->angMin = s2r(a);
			a = eTex->Attribute("angMax");	if (a)  l->angMax = s2r(a);
			a = eTex->Attribute("angSm");	if (a)  l->angSm = s2r(a);
			a = eTex->Attribute("hMin");	if (a)  l->hMin = s2r(a);
			a = eTex->Attribute("hMax");	if (a)  l->hMax = s2r(a);
			a = eTex->Attribute("hSm");		if (a)  l->hSm = s2r(a);

			a = eTex->Attribute("nOn");		if (a)  l->nOnly = s2i(a)>0;
			a = eTex->Attribute("triplanar");	if (a)  l->triplanar = true;  else  l->triplanar = false;

			a = eTex->Attribute("noise");	if (a)  l->noise = s2r(a);
			a = eTex->Attribute("n_1");		if (a)  l->nprev = s2r(a);
			a = eTex->Attribute("n2");		if (a)  l->nnext2 = s2r(a);

			XMLElement* eNoi = eTex->FirstChildElement("noise");
			if (eNoi)
			for (int n=0; n < 2; ++n)
			{	std::string sn = toStr(n), s;
				s = "frq"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nFreq[n]= s2r(a);
				s = "oct"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nOct[n] = s2i(a);
				s = "prs"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPers[n]= s2r(a);
				s = "pow"+sn;  a = eNoi->Attribute(s.c_str());  if (a)  l->nPow[n] = s2r(a);
			}
			if (!road && il < td.ciNumLay)
				td.layersAll[il++] = lay;
			eTex = eTex->NextSiblingElement("texture");
		}
		td.UpdLayers();

		XMLElement* ePar = eTer->FirstChildElement("par");
		if (ePar)
		{
			a = ePar->Attribute("dust");	if (a)  sParDust = String(a);
			a = ePar->Attribute("mud");		if (a)  sParMud = String(a);
			a = ePar->Attribute("smoke");	if (a)  sParSmoke = String(a);
		}
	}
	
	///  paged
 	ePgd = root->FirstChildElement("paged");
	if (ePgd)
	{
		a = ePgd->Attribute("densTrees");	if (a)  densTrees = s2r(a);
		a = ePgd->Attribute("densGrass");	if (a)  densGrass = s2r(a);
		//  grass
		a = ePgd->Attribute("grPage");		if (a)  grPage = s2r(a);
		a = ePgd->Attribute("grDist");		if (a)  grDist = s2r(a);
		a = ePgd->Attribute("grDensSmooth");	if (a)  grDensSmooth = s2i(a);
		//  trees
		a = ePgd->Attribute("trPage");		if (a)  trPage = s2r(a);
		a = ePgd->Attribute("trDist");		if (a)  trDist = s2r(a);
		a = ePgd->Attribute("trDistImp");	if (a)  trDistImp = s2r(a);
		a = ePgd->Attribute("trRdDist");	if (a)  trRdDist = s2i(a);

		int grl = 0;
		XMLElement* eGrL = ePgd->FirstChildElement("grass");
		while (eGrL)
		{
			SGrassLayer g;
			a = eGrL->Attribute("on");		if (a)  g.on = s2i(a);  else  g.on = 1;
			a = eGrL->Attribute("mtr");		if (a)  g.material = String(a);
			a = eGrL->Attribute("clr");		if (a)  g.colorMap = String(a);
			a = eGrL->Attribute("dens");	if (a)  g.dens = s2r(a);
			a = eGrL->Attribute("chan");	if (a)  g.iChan = s2i(a);

			a = eGrL->Attribute("minSx");	if (a)  g.minSx = s2r(a);
			a = eGrL->Attribute("maxSx");	if (a)  g.maxSx = s2r(a);
			a = eGrL->Attribute("minSy");	if (a)  g.minSy = s2r(a);
			a = eGrL->Attribute("maxSy");	if (a)  g.maxSy = s2r(a);

			a = eGrL->Attribute("swayDistr");	if (a)  g.swayDistr = s2r(a);
			a = eGrL->Attribute("swayLen");		if (a)  g.swayLen = s2r(a);
			a = eGrL->Attribute("swaySpeed");	if (a)  g.swaySpeed = s2r(a);
			
		#if 1  //  old < 2.3  (no channels)
		if (grl == 0)  {
			a = eGrL->Attribute("terMaxAng");	if (a)  grChan[0].angMax = s2r(a);
			a = eGrL->Attribute("terAngSm");	if (a)  grChan[0].angSm = s2r(a);

			a = eGrL->Attribute("terMinH");		if (a)  grChan[0].hMin = s2r(a);
			a = eGrL->Attribute("terMaxH");		if (a)  grChan[0].hMax = s2r(a);
			a = eGrL->Attribute("terHSm");		if (a)  grChan[0].hSm = s2r(a);  }
		#endif
			grLayersAll[grl++] = g;
			eGrL = eGrL->NextSiblingElement("grass");
		}

		int c;
		for (c=0; c < 4; c++)
			grChan[c].nFreq += c * 3.f;  // default variation
		c = 0;

		XMLElement* eGrCh = ePgd->FirstChildElement("gchan");
		while (eGrCh && c < 4)
		{
			SGrassChannel& g = grChan[c++];
			TiXmlElement gch("gchan");

			a = eGrCh->Attribute("amin");	if (a)  g.angMin = s2r(a);
			a = eGrCh->Attribute("amax");	if (a)  g.angMax = s2r(a);
			a = eGrCh->Attribute("asm");	if (a)  g.angSm = s2r(a);

			a = eGrCh->Attribute("hmin");	if (a)  g.hMin = s2r(a);
			a = eGrCh->Attribute("hmax");	if (a)  g.hMax = s2r(a);
			a = eGrCh->Attribute("hsm");	if (a)  g.hSm = s2r(a);

			a = eGrCh->Attribute("ns");		if (a)  g.noise = s2r(a);
			a = eGrCh->Attribute("frq");	if (a)  g.nFreq = s2r(a);
			a = eGrCh->Attribute("oct");	if (a)  g.nOct  = s2i(a);
			a = eGrCh->Attribute("prs");	if (a)  g.nPers = s2r(a);
			a = eGrCh->Attribute("pow");	if (a)  g.nPow  = s2r(a);

			a = eGrCh->Attribute("rd");		if (a)  g.rdPow = s2r(a);
			eGrCh = eGrCh->NextSiblingElement("gchan");
		}
		
		///  veget
		int pgl = 0;
		XMLElement* ePgL = ePgd->FirstChildElement("layer");
		while (ePgL)
		{
			PagedLayer l;
			a = ePgL->Attribute("on");			if (a)  l.on = s2i(a);  else  l.on = 1;
			a = ePgL->Attribute("name");		if (a)  l.name = String(a);
			a = ePgL->Attribute("dens");		if (a)  l.dens = s2r(a);
			a = ePgL->Attribute("minScale");	if (a)  l.minScale = s2r(a);
			a = ePgL->Attribute("maxScale");	if (a)  l.maxScale = s2r(a);

			a = ePgL->Attribute("ofsY");		if (a)  l.ofsY = s2r(a);
			a = ePgL->Attribute("addTrRdDist");	if (a)  l.addRdist = s2i(a);
			a = ePgL->Attribute("maxRdist");	if (a)  l.maxRdist = s2i(a);
			a = ePgL->Attribute("windFx");		if (a)  l.windFx = s2r(a);
			a = ePgL->Attribute("windFy");		if (a)  l.windFy = s2r(a);

			a = ePgL->Attribute("maxTerAng");	if (a)  l.maxTerAng = s2r(a);
			a = ePgL->Attribute("minTerH");		if (a)  l.minTerH = s2r(a);
			a = ePgL->Attribute("maxTerH");		if (a)  l.maxTerH = s2r(a);
			a = ePgL->Attribute("maxDepth");	if (a)  l.maxDepth = s2r(a);

			pgLayersAll[pgl++] = l;
			ePgL = ePgL->NextSiblingElement("layer");
		}
		UpdPgLayers();
	}
	
 	///  camera
 	eCam = root->FirstChildElement("cam");
	if (eCam)
	{
		a = eCam->Attribute("pos");		if (a)  camPos = s2v(a);
		a = eCam->Attribute("dir");		if (a)  camDir = s2v(a);
	}
	
	///  objects
	eObjs = root->FirstChildElement("objects");
	if (eObjs)
	{
		XMLElement* eObj = eObjs->FirstChildElement("o");
		while (eObj)
		{
			Object o;
			a = eObj->Attribute("name");	if (a)  o.name = std::string(a);

			a = eObj->Attribute("pos");		if (a)  {  Vector3 v = s2v(a);  o.pos = MATHVECTOR<float,3>(v.x,v.y,v.z);  }
			a = eObj->Attribute("rot");		if (a)  {  Vector4 v = s2v4(a);  o.rot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
			a = eObj->Attribute("sc");		if (a)  o.scale = s2v(a);

			objects.push_back(o);
			eObj = eObj->NextSiblingElement("o");
		}
	}
	
	UpdateFluidsId();

	UpdateSurfId();
	
	return true;
}


//  Save
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::SaveXml(String file)
{
	TiXmlDocument xml;	TiXmlElement root("scene");

	TiXmlElement car("car");
		car.SetAttribute("tires",	asphalt ? "1":"0");
		if (damageMul != 1.f)
			car.SetAttribute("damage",	toStrC( damageMul ));

		if (denyReversed)
			car.SetAttribute("denyRev",	"1");
		if (gravity != 9.81f)
			car.SetAttribute("gravity",	toStrC( gravity ));
	root.InsertEndChild(car);


	TiXmlElement st("start");
		std::string s = toStr(startPos[0])+" "+toStr(startPos[1])+" "+toStr(startPos[2]);
		st.SetAttribute("pos",	s.c_str());

		s = toStr(startRot[0])+" "+toStr(startRot[1])+" "+toStr(startRot[2])+" "+toStr(startRot[3]);
		st.SetAttribute("rot",	s.c_str());
	root.InsertEndChild(st);


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
		fog.SetAttribute("color",		toStrC( fogClr ));
		fog.SetAttribute("color2",		toStrC( fogClr2 ));
		fog.SetAttribute("linStart",	toStrC( fogStart ));
		fog.SetAttribute("linEnd",		toStrC( fogEnd ));
	root.InsertEndChild(fog);

	TiXmlElement fogH("fogH");
		fogH.SetAttribute("color",		toStrC( fogClrH ));
		fogH.SetAttribute("height",		toStrC( fogHeight ));
		fogH.SetAttribute("dens",		toStrC( fogHDensity ));
		fogH.SetAttribute("linStart",	toStrC( fogHStart ));
		fogH.SetAttribute("linEnd",		toStrC( fogHEnd ));
	root.InsertEndChild(fogH);

	TiXmlElement li("light");
		li.SetAttribute("sceneryId",	sceneryId.c_str() );
		li.SetAttribute("pitch",		toStrC( ldPitch ));
		li.SetAttribute("yaw",			toStrC( ldYaw ));
		li.SetAttribute("ambient",		toStrC( lAmb ));
		li.SetAttribute("diffuse",		toStrC( lDiff ));
		li.SetAttribute("specular",		toStrC( lSpec ));
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
				tex.SetAttribute("tclr",	toStrC( l->tclr ));
			setDmst();
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
			{	std::string sn = toStr(n), s;
				s = "frq"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nFreq[n] ));
				s = "oct"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nOct[n] ));
				s = "prs"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPers[n] ));
				s = "pow"+sn;  noi.SetAttribute(s.c_str(),  toStrC( l->nPow[n] ));
			}
			tex.InsertEndChild(noi);
			ter.InsertEndChild(tex);
		}
		l = &td.layerRoad;
		TiXmlElement tex("texture");
		tex.SetAttribute("road",	1);
		tex.SetAttribute("surf",	l->surfName.c_str());
		setDmst();
		ter.InsertEndChild(tex);
	
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

			std::string s = toStr(o->pos[0])+" "+toStr(o->pos[1])+" "+toStr(o->pos[2]);
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
	XMLElement* eTex,*eRd,*eVeg,*eGr;
	const char* a;

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
		a = eTex->Attribute("tr");	if (a)  l.tclr = s2c(a);

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
		a = eRd->Attribute("tr");	if (a)  l.tclr = s2c(a);

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
