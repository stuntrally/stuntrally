#include "pch.h"
#include "../Def_Str.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"
#include <OgreSceneNode.h>
#include "../vdrift/game.h"  // for surfaces map
using namespace Ogre;
using namespace tinyxml2;


Scene::Scene()
	: pGame(0)
{
	pFluidsXml = 0;
	Default();
}
void Scene::Default()
{
	ter = true;  vdr = false;

	asphalt = false;  denyReversed = false;
	windAmt = 0.f;  damageMul = 1.f;
	gravity = 9.81f;

	skyMtr = "World/NoonSky";
	rainEmit = 0;  rainName = "";
	rain2Emit = 0;  rain2Name = "";

	fogStart = 600;  fogEnd = 1600;
	fogClr = fogClr2 = fogClrH = Vector4(0.73f, 0.86f, 1.0f, 1.f);
	fogHeight = -300.f;  fogHDensity = 100.f;  fogHStart = 0;  fogHEnd = 400;

	ldPitch = 50.f, ldYaw = 30.f;
	lDir  = Vector3(0.0f, -1.0f, 0.0f);	lAmb  = Vector3(0.45f, 0.45f, 0.45f);
	lDiff = Vector3(0.7f, 0.7f, 0.7f);	lSpec = Vector3(0.99f, 0.99f, 0.97f);

	sParDust = "Dust";  sParMud = "Mud";  sParSmoke = "Smoke";

	td.Default();

	densTrees=0;  densGrass=0;  grDensSmooth=6;
	grPage = 80;  grDist = 80;

	for (int i=0; i < ciNumGrLay; ++i)
	{
		SGrassLayer* gr = &grLayersAll[i];
		gr->on = i == 0;
		gr->material = "grassJungle";  gr->colorMap = "grClrJungle.png";
		gr->minSx = 0.6f;  gr->minSy = 0.6f;  gr->maxSx = 0.85f;  gr->maxSy = 0.9f;
		gr->swayDistr = 4.0f;  gr->swayLen = 0.2f;  gr->swaySpeed = 0.5f;
		gr->terMaxAng = 30.f;  gr->terMinH = -200.f;  gr->terMaxH = 200.f;
	}
	trPage = 200;  trDist = 200;  trDistImp = 800;  trRdDist = 3;

	camPos = Vector3(10.f,20.f,10.f);  camDir = Vector3(0.f,-0.3f,1.f);
	sceneryId = "0";
	fluids.clear();  //
	objects.clear();  //
}

PagedLayer::PagedLayer()
{
	on = 0;  name = "";  dens = 0.1f;
	windFx = 0.0f;  windFy = 0.0f;  addRdist = 0;  maxRdist = 100;
	minScale = 0.1f;  maxScale = 0.25f;  ofsY = 0.f;
	maxTerAng = 50.f;  minTerH = -100.f;  maxTerH = 100.f;
	maxDepth = 5.f;
}

SGrassLayer::SGrassLayer()
{
	on = false;
	dens = 0.1f;
	minSx = 0.6f; minSy = 0.6f;  maxSx = 0.85f; maxSy = 0.9f;
	swayDistr = 4.f;  swayLen = 0.2f; swaySpeed = 0.5f;
	terMaxAng = 30.f;  terAngSm = 20.f;
	terMinH = -100.f;  terMaxH = 100.f;  terHSm = 20.f;
	material = "grassForest";  colorMap = "grClrForest.png";
}

FluidBox::FluidBox()
	:cobj(0), id(-1), idParticles(0)
	,pos(Vector3::ZERO), rot(Vector3::ZERO)
	,size(Vector3::ZERO), tile(0.01,0.01)
{	}

Object::Object()
	:nd(0),ent(0),ms(0),co(0),rb(0), dyn(false)
	,pos(0,0,0),rot(0,-1,0,0)
	,scale(Vector3::UNIT_SCALE)
{	}


///  bullet to ogre  ----------
Quaternion Object::qrFix(  0.707107, 0, 0.707107, 0);  //SetAxisAngle( PI_d/2.f, 0,1,0);
Quaternion Object::qrFix2(-0.707107, 0, 0.707107, 0);  //SetAxisAngle(-PI_d/2.f, 0,1,0);

void Object::SetFromBlt()
{
	if (!nd)  return;
	Vector3 posO = Vector3(pos[0],pos[2],-pos[1]);

	Quaternion q(rot[0],-rot[3],rot[1],rot[2]);
	Quaternion rotO = q * qrFix;

	nd->setPosition(posO);
	nd->setOrientation(rotO);
}


void Scene::UpdateFluidsId()
{
	if (!pFluidsXml)  return;
	
	//  set fluids id from name
	for (int i=0; i < fluids.size(); ++i)
	{
		int id = pFluidsXml->flMap[fluids[i].name]-1;
		fluids[i].id = id;
		fluids[i].idParticles = id == -1 ? -1 : pFluidsXml->fls[id].idParticles;
		if (id == -1)
			LogO("! Scene fluid name: " + fluids[i].name + " not found in xml !");
	}
}

void Scene::UpdateSurfId()
{
	if (!pGame)  return;
	//  update surfId from surfName
	//  terrain
	for (int i=0; i < td.ciNumLay; ++i)
	{
		const std::string& s = td.layersAll[i].surfName;
		int id = pGame->surf_map[s]-1;
		if (id == -1)
		{	id = 0;  // default if not found
			LogO("! Warning: Surface not found (terrain): "+s);
		}
		td.layersAll[i].surfId = id;  // cached
	}
	//  road
	const std::string& s = td.layerRoad.surfName;
	int id = pGame->surf_map[s]-1;
	if (id == -1)
	{	id = 0;
		LogO("! Warning: Surface not found (road): "+s);
	}
	td.layerRoad.surfId = id;
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
	XMLElement* eSky,*eFog,*eFogH,*eLi,*eTer,*ePgd,*eCam,*eFls,*eObjs,*eCar;
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

	///  sky
	eSky = root->FirstChildElement("sky");
	if (eSky)	{
		a = eSky->Attribute("material");	if (a)  skyMtr = String(a);
		a = eSky->Attribute("rainEmit");	if (a)  rainEmit = s2i(a);
		a = eSky->Attribute("rainName");	if (a)  rainName = String(a);
		a = eSky->Attribute("rain2Emit");	if (a)  rain2Emit = s2i(a);
		a = eSky->Attribute("rain2Name");	if (a)  rain2Name = String(a);
		a = eSky->Attribute("windAmt");		if (a)  windAmt = s2r(a);	}
		
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

		//a = eTer->Attribute("shadowMap");	if (a)  td. = s2i(a);
		//a = eTer->Attribute("blendMap");	if (a)  td. = s2i(a);
		//a = eTer->Attribute("compositeMap");	if (a)  td. = s2i(a);
		td.UpdVals();

		int il = 0;
		XMLElement* eTex = eTer->FirstChildElement("texture");
		while (eTex)
		{
			bool road = false;
			a = eTex->Attribute("road");	if (a)  if (s2i(a)==1)  road = true;
			
			TerLayer lay, *l = road ? &td.layerRoad : &lay;

			a = eTex->Attribute("on");		if (a)  l->on = s2i(a);  else  l->on = 1;
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

			a = eTex->Attribute("noise");	if (a)  l->noise = s2r(a);
			a = eTex->Attribute("n_1");		if (a)  l->nprev = s2r(a);
			a = eTex->Attribute("n2");		if (a)  l->nnext2 = s2r(a);

			a = eTex->Attribute("nOnly");	if (a)  l->bNoiseOnly = s2i(a) > 0;  else  l->bNoiseOnly = true;
			a = eTex->Attribute("triplanar");	if (a)  l->triplanar = true;  else  l->triplanar = false;

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

	#if 1  // old scene.xml (SR ver <= 1.8), 1 grass layer
		SGrassLayer* gr = &grLayersAll[0];  gr->dens = 1.f;
		a = ePgd->Attribute("grMtr");		if (a)  gr->material = String(a);
		a = ePgd->Attribute("grClr");		if (a)  gr->colorMap = String(a);
		//  grass par
		a = ePgd->Attribute("grMinSx");		if (a)  gr->minSx = s2r(a);
		a = ePgd->Attribute("grMinSy");		if (a)  gr->minSy = s2r(a);
		a = ePgd->Attribute("grMaxSx");		if (a)  gr->maxSx = s2r(a);
		a = ePgd->Attribute("grMaxSy");		if (a)  gr->maxSy = s2r(a);

		a = ePgd->Attribute("grSwayDistr");	if (a)  gr->swayDistr = s2r(a);
		a = ePgd->Attribute("grSwayLen");	if (a)  gr->swayLen = s2r(a);
		a = ePgd->Attribute("grSwaySpeed");	if (a)  gr->swaySpeed = s2r(a);

		a = ePgd->Attribute("grTerMaxAngle");	if (a)  gr->terMaxAng = s2r(a);
		a = ePgd->Attribute("grTerMinHeight");	if (a)  gr->terMinH = s2r(a);
		a = ePgd->Attribute("grTerMaxHeight");	if (a)  gr->terMaxH = s2r(a);
	#endif

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

			a = eGrL->Attribute("minSx");	if (a)  g.minSx = s2r(a);
			a = eGrL->Attribute("maxSx");	if (a)  g.maxSx = s2r(a);
			a = eGrL->Attribute("minSy");	if (a)  g.minSy = s2r(a);
			a = eGrL->Attribute("maxSy");	if (a)  g.maxSy = s2r(a);

			a = eGrL->Attribute("swayDistr");	if (a)  g.swayDistr = s2r(a);
			a = eGrL->Attribute("swayLen");		if (a)  g.swayLen = s2r(a);
			a = eGrL->Attribute("swaySpeed");	if (a)  g.swaySpeed = s2r(a);
			
			a = eGrL->Attribute("terMaxAng");	if (a)  g.terMaxAng = s2r(a);
			a = eGrL->Attribute("terAngSm");	if (a)  g.terAngSm = s2r(a);

			a = eGrL->Attribute("terMinH");		if (a)  g.terMinH = s2r(a);
			a = eGrL->Attribute("terMaxH");		if (a)  g.terMaxH = s2r(a);
			a = eGrL->Attribute("terHSm");		if (a)  g.terHSm = s2r(a);

			grLayersAll[grl++] = g;
			eGrL = eGrL->NextSiblingElement("grass");
		}

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
			a = eObj->Attribute("rot");		if (a)  {  Vector4 v = Ogre::StringConverter::parseVector4(a);  o.rot = QUATERNION<float>(v.x,v.y,v.z,v.w);  }
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
		//terErr, shadowMap, blendMap, compositeMap

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

			tex.SetAttribute("noise",	toStrC( l->noise ));
			tex.SetAttribute("n_1",		toStrC( l->nprev ));
			tex.SetAttribute("n2",		toStrC( l->nnext2 ));

			tex.SetAttribute("nOnly",	l->bNoiseOnly ? 1 : 0);
			if (l->triplanar)  tex.SetAttribute("triplanar", 1);
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

		for (int i=0; i < ciNumGrLay; ++i)
		{
			const SGrassLayer& g = grLayersAll[i];
			TiXmlElement grl("grass");
			grl.SetAttribute("on",		g.on ? 1 : 0);
			grl.SetAttribute("mtr",		g.material.c_str());
			grl.SetAttribute("clr",		g.colorMap.c_str());
			grl.SetAttribute("dens",	toStrC( g.dens ));

			grl.SetAttribute("minSx",	toStrC( g.minSx ));
			grl.SetAttribute("maxSx",	toStrC( g.maxSx ));
			grl.SetAttribute("minSy",	toStrC( g.minSy ));
			grl.SetAttribute("maxSy",	toStrC( g.maxSy ));

			grl.SetAttribute("swayDistr",	toStrC( g.swayDistr ));
			grl.SetAttribute("swayLen",		toStrC( g.swayLen ));
			grl.SetAttribute("swaySpeed",	toStrC( g.swaySpeed ));
			
			grl.SetAttribute("terMaxAng",	toStrC( g.terMaxAng ));
			if (g.terAngSm != 20.f)
			grl.SetAttribute("terAngSm",	toStrC( g.terAngSm ));

			grl.SetAttribute("terMinH",		toStrC( g.terMinH ));
			grl.SetAttribute("terMaxH",		toStrC( g.terMaxH ));
			if (g.terHSm != 20.f)
			grl.SetAttribute("terHSm",		toStrC( g.terHSm ));

			pgd.InsertEndChild(grl);
		}

		for (int i=0; i < ciNumPgLay; ++i)
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
		for (int i=0; i < objects.size(); ++i)
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

//--------------------------------------------------------------------------------------------------------------------------------------


TerData::TerData()
{
	hfHeight = NULL;
	Default();
}
void TerData::Default()
{
	iVertsX = 512*2 +1;
	fTriangleSize = 1.f;  // scale
	triplanar1Layer = 8;  // off
	errorNorm = 1.7;

	for (int i=0; i < ciNumLay; ++i)
	{	
		TerLayer& l = layersAll[i];  l.tiling = 4.5f;
		l.on = i==0;
		l.texFile = "jungle_0.dds";  l.texNorm = "jungle_0_nh.dds";
		l.dust = 0.f;  l.mud = 1.f;  l.smoke = 0.f;
		l.tclr = ColourValue(0.2f,0.2f,0.f,1.f);
	}
	layerRoad.dust = 0.f;  layerRoad.mud = 0.f;  /*layerRoad.smoke = 1.f;*/
	layerRoad.tclr = ColourValue(0,0,0,1);
	
	UpdVals();  UpdLayers();
	//4097-! 2049  1025+ 513  257 -33t  verts
	//layers:  1- 230 fps  2- 180 fps  3- 140 fps
}

TerLayer::TerLayer() : on(true), tiling(4.f), triplanar(false),
	dust(0.f),dustS(0.2f), mud(0.f), smoke(0.f), tclr(ColourValue::Black),
	angMin(0.f),angMax(90.f), angSm(20.f),
	hMin(-300.f),hMax(300.f), hSm(20.f),
	noise(1.f), nprev(0.f), nnext2(0.f),
	bNoiseOnly(1),
	surfName("Default"), surfId(0)  //!
{	}

void TerData::UpdVals()
{
	iVertsY = iVertsX;  //square only-[]
	iTerSize = iVertsX;
	fTerWorldSize = (iTerSize-1)*fTriangleSize;
}


//  fill only active layers
//------------------------------------------
void TerData::UpdLayers()
{
	layers.clear();  int li = 0;  triplanar1Layer = 8;
	for (int i=0; i < ciNumLay; ++i)
	{
		if (layersAll[i].on)
		{
			if (layersAll[i].triplanar)  triplanar1Layer = li;
			++li;
			layers.push_back(i);
		}
	}
}

void Scene::UpdPgLayers()
{
	pgLayers.clear();
	for (int i=0; i < ciNumPgLay; ++i)
	{
		if (pgLayersAll[i].on)
			pgLayers.push_back(i);
	}
}
