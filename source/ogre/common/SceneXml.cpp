#include "pch.h"
#include "Defines.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "tinyxml.h"
#include <OgreSceneNode.h>

using namespace Ogre;


Scene::Scene()
{
	pFluidsXml = 0;
	Default();
}
void Scene::Default()
{
	skyMtr = "World/NoonSky";
	rainEmit = 0;  rainName = "";
	rain2Emit = 0;  rain2Name = "";

	fogMode = FOG_LINEAR;  fogStart = 600;  fogEnd = 1600;
	fogClr = Vector3(0.73f, 0.86f, 1.0f);  fogExp = 0;

	ldPitch = 45.f, ldYaw = 90.f;
	lDir  = Vector3(0.0f, -1.0f, 1.0f);	lAmb  = Vector3(0.45f, 0.45f, 0.45f);
	lDiff = Vector3(1.0f, 1.0f, 0.98f);	lSpec = Vector3(0.99f, 0.99f, 0.97f);

	sParDust = "Dust";  sParMud = "Mud";  sParSmoke = "Smoke";

	ter = true;

	td.Default();
	td.layerRoad.smoke = !ter ? 1.f : 0.f;  //`

	densTrees=0;  densGrass=0;  grDensSmooth=6;
	grassMtr = "grassJungle";  grassColorMap = "grClrJungle.png";
	grPage = 80;  grDist = 80;
	grMinSx = 0.6f;  grMinSy = 0.6f;  grMaxSx = 0.85f;  grMaxSy = 0.9f;
	grSwayDistr = 4.0f;  grSwayLen = 0.2f;  grSwaySpeed = 0.5f;
	trPage = 200;  trDist = 200;  trDistImp = 800;  trRdDist = 3;
	grTerMaxAngle = 30.f;  grTerMaxHeight = 200.f;

	camPos = Vector3(10.f,20.f,10.f);  camDir = Vector3(0.f,-0.3f,1.f);
	sceneryId = 0;
	fluids.clear();  //
	objects.clear();  //
}

PagedLayer::PagedLayer()
{
	on = 0;  name = "";  dens = 0.1f;
	windFx = 0.0f;  windFy = 0.0f;  addTrRdDist = 0;
	minScale = 0.1f;  maxScale = 0.25f;  ofsY = 0.f;
	maxTerAng = 50.f;  minTerH = -100.f;  maxTerH = 100.f;
	maxDepth = 5.f;
}

FluidBox::FluidBox()
	:cobj(0), id(-1), idParticles(0)
	,pos(Vector3::ZERO), rot(Vector3::ZERO)
	,size(Vector3::ZERO), tile(0.01,0.01)
{	}

Object::Object()
	:nd(0),ent(0),ms(0),cobj(0)
	,pos(0,0,0),rot(0,-1,0,0)
	,scale(Vector3::UNIT_SCALE)
{	}


///  bullet to ogre  ----------
Quaternion Object::qrFix(0.707107, 0, 0.707107, 0);  //SetAxisAngle(PI_d/2.f, 0,1,0);
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


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::LoadXml(String file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	// clear  --
	Default();
	//td.layers.clear();
	//pgLayers.clear();

	// read
	TiXmlElement* eSky,*eFog,*eLi,*eTer,*ePgd,*eCam,*eFls,*eObjs;
	const char* a;


	///  sky
	eSky = root->FirstChildElement("sky");
	if (eSky)	{
		a = eSky->Attribute("material");	if (a)  skyMtr = String(a);
		a = eSky->Attribute("rainEmit");	if (a)  rainEmit = s2i(a);
		a = eSky->Attribute("rainName");	if (a)  rainName = String(a);
		a = eSky->Attribute("rain2Emit");	if (a)  rain2Emit = s2i(a);
		a = eSky->Attribute("rain2Name");	if (a)  rain2Name = String(a);  }
		
	///  fog
	eFog = root->FirstChildElement("fog");
	if (eFog)
	{
		a = eFog->Attribute("mode");		if (a)  {	String s = a;
			if (s == "lin")   fogMode = FOG_LINEAR;  else
			if (s == "exp")   fogMode = FOG_EXP;    else
			if (s == "exp2")  fogMode = FOG_EXP2;  else
			if (s == "none")  fogMode = FOG_NONE;
		}
		a = eFog->Attribute("exp");			if (a)  fogExp = s2r(a);
		a = eFog->Attribute("linStart");	if (a)  fogStart = s2r(a);
		a = eFog->Attribute("linEnd");		if (a)  fogEnd = s2r(a);
		a = eFog->Attribute("color");		if (a)  fogClr = s2v(a);
	}

	///  light
	eLi = root->FirstChildElement("light");
	if (eLi)
	{
		a = eLi->Attribute("sceneryId");	if (a)  sceneryId = s2i(a);  ///
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
		TiXmlElement* eFl = eFls->FirstChildElement("fluid");
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

		//a = eTer->Attribute("terErr");	if (a)  td. = s2i(a);  //...
		//a = eTer->Attribute("shadowMap");	if (a)  td. = s2i(a);
		//a = eTer->Attribute("blendMap");	if (a)  td. = s2i(a);
		//a = eTer->Attribute("compositeMap");	if (a)  td. = s2i(a);
		td.UpdVals();

		int il = 0;
		TiXmlElement* eTex = eTer->FirstChildElement("texture");
		while (eTex)
		{
			bool road = false;
			a = eTex->Attribute("road");	if (a)  if (s2i(a)==1)  road = true;
			
			TerLayer lay, *l = road ? &td.layerRoad : &lay;

			a = eTex->Attribute("on");		if (a)  l->on = s2i(a);  else  l->on = 1;
			a = eTex->Attribute("scale");	if (a)  l->tiling = s2r(a);
			a = eTex->Attribute("file");	if (a)  l->texFile = String(a);
			a = eTex->Attribute("fnorm");	if (a)  l->texNorm = String(a);

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
			a = eTex->Attribute("nOnly");	if (a)  l->bNoiseOnly = s2i(a) > 0;  else  l->bNoiseOnly = true;

			if (!road && il < td.ciNumLay)
				td.layersAll[il++] = lay;
			eTex = eTex->NextSiblingElement("texture");
		}
		td.UpdLayers();

		TiXmlElement* ePar = eTer->FirstChildElement("par");
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
		a = ePgd->Attribute("grMtr");		if (a)  grassMtr = String(a);
		a = ePgd->Attribute("grClr");		if (a)  grassColorMap = String(a);
		//  grass par
		a = ePgd->Attribute("grPage");		if (a)  grPage = s2r(a);
		a = ePgd->Attribute("grDist");		if (a)  grDist = s2r(a);

		a = ePgd->Attribute("grMinSx");		if (a)  grMinSx = s2r(a);
		a = ePgd->Attribute("grMinSy");		if (a)  grMinSy = s2r(a);
		a = ePgd->Attribute("grMaxSx");		if (a)  grMaxSx = s2r(a);
		a = ePgd->Attribute("grMaxSy");		if (a)  grMaxSy = s2r(a);

		a = ePgd->Attribute("grSwayDistr");	if (a)  grSwayDistr = s2r(a);
		a = ePgd->Attribute("grSwayLen");	if (a)  grSwayLen = s2r(a);
		a = ePgd->Attribute("grSwaySpeed");	if (a)  grSwaySpeed = s2r(a);
		a = ePgd->Attribute("grDensSmooth");	if (a)  grDensSmooth = s2i(a);

		a = ePgd->Attribute("grTerMaxAngle");	if (a)  grTerMaxAngle = s2r(a);
		a = ePgd->Attribute("grTerMaxHeight");	if (a)  grTerMaxHeight = s2r(a);
		//  trees
		a = ePgd->Attribute("trPage");		if (a)  trPage = s2r(a);
		a = ePgd->Attribute("trDist");		if (a)  trDist = s2r(a);
		a = ePgd->Attribute("trDistImp");	if (a)  trDistImp = s2r(a);
		a = ePgd->Attribute("trRdDist");	if (a)  trRdDist = s2i(a);

		int pgl = 0;
		TiXmlElement* ePgL = ePgd->FirstChildElement("layer");
		while (ePgL)
		{
			PagedLayer l;
			a = ePgL->Attribute("on");			if (a)  l.on = s2i(a);  else  l.on = 1;
			a = ePgL->Attribute("name");		if (a)  l.name = String(a);
			a = ePgL->Attribute("dens");		if (a)  l.dens = s2r(a);
			a = ePgL->Attribute("minScale");	if (a)  l.minScale = s2r(a);
			a = ePgL->Attribute("maxScale");	if (a)  l.maxScale = s2r(a);
			a = ePgL->Attribute("ofsY");		if (a)  l.ofsY = s2r(a);
			a = ePgL->Attribute("addTrRdDist");	if (a)  l.addTrRdDist = s2i(a);
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
		TiXmlElement* eObj = eObjs->FirstChildElement("o");
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
	
	return true;
}


//  Save
//--------------------------------------------------------------------------------------------------------------------------------------

bool Scene::SaveXml(String file)
{
	TiXmlDocument xml;	TiXmlElement root("scene");

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
	root.InsertEndChild(sky);

	TiXmlElement fog("fog");
		switch (fogMode) {
			case FOG_LINEAR: fog.SetAttribute("mode", "lin");  break;
			case FOG_EXP:    fog.SetAttribute("mode", "exp");  break;
			case FOG_EXP2:   fog.SetAttribute("mode", "exp2");  break;
			case FOG_NONE:   fog.SetAttribute("mode", "none");  break;  }
		fog.SetAttribute("color",		toStrC( fogClr ));
		fog.SetAttribute("exp",			toStrC( fogExp ));
		fog.SetAttribute("linStart",	toStrC( fogStart ));
		fog.SetAttribute("linEnd",		toStrC( fogEnd ));
	root.InsertEndChild(fog);

	TiXmlElement li("light");
		li.SetAttribute("sceneryId",	toStrC( sceneryId ));
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
			tex.SetAttribute("nOnly",	l->bNoiseOnly ? 1 : 0);
			ter.InsertEndChild(tex);
		}
		l = &td.layerRoad;
		TiXmlElement tex("texture");
		tex.SetAttribute("road",	1);  setDmst();
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
		pgd.SetAttribute("grMtr",		grassMtr.c_str());
		pgd.SetAttribute("grClr",		grassColorMap.c_str());
		//  grass par
		pgd.SetAttribute("grPage",		toStrC( grPage ));
		pgd.SetAttribute("grDist",		toStrC( grDist ));

		pgd.SetAttribute("grMinSx",		toStrC( grMinSx ));
		pgd.SetAttribute("grMinSy",		toStrC( grMinSy ));
		pgd.SetAttribute("grMaxSx",		toStrC( grMaxSx ));
		pgd.SetAttribute("grMaxSy",		toStrC( grMaxSy ));

		pgd.SetAttribute("grSwayDistr",	toStrC( grSwayDistr ));
		pgd.SetAttribute("grSwayLen",	toStrC( grSwayLen   ));
		pgd.SetAttribute("grSwaySpeed",	toStrC( grSwaySpeed ));
		pgd.SetAttribute("grDensSmooth",toStrC( grDensSmooth ));

		pgd.SetAttribute("grTerMaxAngle",toStrC( grTerMaxAngle ));
		pgd.SetAttribute("grTerMaxHeight",toStrC( grTerMaxHeight ));
		//  trees
		pgd.SetAttribute("trPage",		toStrC( trPage ));
		pgd.SetAttribute("trDist",		toStrC( trDist ));
		pgd.SetAttribute("trDistImp",	toStrC( trDistImp ));
		pgd.SetAttribute("trRdDist",	toStrC( trRdDist  ));

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
			pgl.SetAttribute("addTrRdDist",	toStrC( l.addTrRdDist ));
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
	hfHeight = NULL;  hfAngle = NULL;
	Default();
}
void TerData::Default()
{
	iVertsX = 512*2 +1;
	fTriangleSize = 1.f;  // scale

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

TerLayer::TerLayer() : on(true), tiling(4.f),
	dust(0.f),dustS(0.2), mud(0.f), smoke(0), tclr(ColourValue::Black),
	angMin(0.f),angMax(90.f), angSm(20.f),
	hMin(-300.f),hMax(300.f), hSm(20.f),
	noise(1), bNoiseOnly(1)
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
	layers.clear();
	for (int i=0; i < ciNumLay; ++i)
	{
		if (layersAll[i].on)
			layers.push_back(i);
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
