#include "stdafx.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"

#include "boost/filesystem.hpp"
#define FileExists(s) boost::filesystem::exists(s)

CarModel::CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s, Camera* cam) : 
	hue(0), sat(0), val(0), fCam(0), pMainNode(0), pCar(0), terrain(0), resCar(""), mCamera(0)
{
	iIndex = index;
	sDirname = name;
	pSceneMgr = sceneMgr;
	pSet = set;
	pGame = game;
	sc = s;
	mCamera = cam;
	
	for (int w = 0; w < 4; ++w)
	{	ps[w] = 0;  pm[w] = 0;  pd[w] = 0;
		ndWh[w] = 0;  ndWhE[w] = 0; whTrl[w] = 0;
		ndRs[w] = 0;  ndRd[w] = 0;
		wht[w] = 0.f;  whTerMtr[w] = 0; }
	
	///TODO create pCar
	pCar = &(*pGame->cars.begin());
	
	pReflect = new CarReflection(pSet, pSceneMgr, iIndex);
}
CarModel::~CarModel(void)
{
	delete pReflect; pReflect = 0;
	
	delete fCam; fCam = 0;
	pSceneMgr->destroyCamera("CarCamera" + iIndex);
	
	// Destroy par sys
	for (int w=0; w < 4; w++)  {
		if (ps[w]) {  pSceneMgr->destroyParticleSystem(ps[w]);   ps[w]=0;  }
		if (pm[w]) {  pSceneMgr->destroyParticleSystem(pm[w]);   pm[w]=0;  }
		if (pd[w]) {  pSceneMgr->destroyParticleSystem(pd[w]);   pd[w]=0;  }  }
	
	///TODO allow multiple cars
	Ogre::Root::getSingletonPtr()->removeResourceLocation(PATHMANAGER::GetCacheDir());
	Ogre::Root::getSingletonPtr()->removeResourceLocation(resCar);

		/*if (resDrv != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(resDrv);
		resDrv = "drivers/driver2/textures";*/
					
	if (pMainNode) pSceneMgr->destroySceneNode(pMainNode);
	if (pSceneMgr->hasEntity("Car")) pSceneMgr->destroyEntity("Car");
	if (pSceneMgr->hasEntity("Car.interior")) pSceneMgr->destroyEntity("Car.interior");
	if (pSceneMgr->hasEntity("Car.glass")) pSceneMgr->destroyEntity("Car.glass");
	
	MeshPtr mp = MeshManager::getSingleton().getByName("body.mesh");
	if (!mp.isNull())
		MeshManager::getSingleton().remove("body.mesh");
}

void CarModel::Update(PosInfo newPosInfo, float time)
{	
	///????
	/*if (!bNew)  return;  // new only
	bNew = false;*/
	if (!pMainNode) return;
	//  car pos and rot
	pMainNode->setPosition(newPosInfo.newPos);
	pMainNode->setOrientation(newPosInfo.newRot);	
	
	//  wheels
	for (int w=0; w < 4; w++)
	{
		float wR = newPosInfo.newWhR[w];
		ndWh[w]->setPosition(newPosInfo.newWhPos[w]);
		ndWh[w]->setOrientation(newPosInfo.newWhRot[w]);
		int whMtr = newPosInfo.newWhMtr[w];  //whTerMtr[w];
		
		
		//  update particle emitters
		//-----------------------------------------------------------------------------
		float whVel = newPosInfo.newWhVel[w] * 3.6f;  //kmh
		float slide = newPosInfo.newWhSlide[w], squeal = newPosInfo.newWhSqueal[w];
		float onGr = slide < 0.f ? 0.f : 1.f;

		//  wheel temp
		wht[w] += squeal * time * 7;
		wht[w] -= time*6;  if (wht[w] < 0.f)  wht[w] = 0.f;

		///  emit rates +
		Real emitS = 0.f, emitM = 0.f, emitD = 0.f;  //paused
		if (!pGame->pause)
		{
			 Real sq = squeal* min(1.f, wht[w]), l = pSet->particles_len * onGr;
			 emitS = sq * (whVel * 30) * l *0.3f;  //..
			 emitM = slide < 1.4f ? 0.f :  (8.f * sq * min(5.f, slide) * l);
			 emitD = (min(140.f, whVel) / 3.5f + slide * 1.f ) * l;  
			 
			 // resume
			 pd[w]->setSpeedFactor(1.f);
			 ps[w]->setSpeedFactor(1.f);
			 pm[w]->setSpeedFactor(1.f);
		}
		else
		{
			 // stop par sys
			 pd[w]->setSpeedFactor(0.f);
			 ps[w]->setSpeedFactor(0.f);
			 pm[w]->setSpeedFactor(0.f);
		}
		Real sizeD = (0.3f + 1.1f * min(140.f, whVel) / 140.f) * (w < 2 ? 0.5f : 1.f);
		//  ter mtr factors
		int mtr = min((int)(whMtr-1), (int)(sc->td.layers.size()-1));
		TerLayer& lay = whMtr==0 ? sc->td.layerRoad : sc->td.layersAll[sc->td.layers[mtr]];
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		//  par emit
		Vector3 vpos = newPosInfo.newWhPos[w];
		if (pSet->particles)
		{
			if (ps[w] && sc->td.layerRoad.smoke > 0.f/*&& !sc->ter*/)  // only at vdr road
			{
				ParticleEmitter* pe = ps[w]->getEmitter(0);
				pe->setPosition(vpos + newPosInfo.newCarY * wR*0.7f); // 0.218
				/**/ps[w]->getAffector(0)->setParameter("alpha", toStr(-0.4f - 0.07f/2.4f * whVel));
				/**/pe->setTimeToLive( max(0.1, 2 - whVel/2.4f * 0.04) );  // fade,live
				pe->setDirection(-newPosInfo.newCarY);	pe->setEmissionRate(emitS);
			}
			if (pm[w])	//  mud
			{	ParticleEmitter* pe = pm[w]->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + newPosInfo.newCarY * wR*0.7f); // 0.218
				pe->setDirection(-newPosInfo.newCarY);	pe->setEmissionRate(emitM);
			}
			if (pd[w])	//  dust
			{	pd[w]->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = pd[w]->getEmitter(0);
				pe->setPosition(vpos + newPosInfo.newCarY * wR*0.51f ); // 0.16
				pe->setDirection(-newPosInfo.newCarY);	pe->setEmissionRate(emitD);
			}
		}

		//  update trails h+
		if (pSet->trails)  {
			if (ndWhE[w])
			{	Vector3 vp = vpos + newPosInfo.newCarY * wR*0.72f;  // 0.22
				if (terrain && whMtr > 0)
					vp.y = terrain->getHeightAtWorldPosition(vp) + 0.05f;
					//if (/*whOnRoad[w]*/whMtr > 0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
			}
			float al = 0.5f * /*squeal*/ min(1.f, 0.7f * wht[w]) * onGr;  // par+
			if (whTrl[w])	whTrl[w]->setInitialColour(0,
				lay.tclr.r,lay.tclr.g,lay.tclr.b, lay.tclr.a * al/**/);
		}
	}

	// Reflection
	pReflect->camPosition = pMainNode->getPosition();
	pReflect->Update();
	
	//blendmaps
	UpdWhTerMtr();
}
void CarModel::Create(void)
{		
	Ogre::Root::getSingletonPtr()->addResourceLocation(PATHMANAGER::GetCacheDir(), "FileSystem");
	resCar = PATHMANAGER::GetCarPath() + "/" + sDirname + "/textures";
	Ogre::Root::getSingletonPtr()->addResourceLocation(resCar, "FileSystem");
	
	pMainNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
	pCar = &(*pGame->cars.begin());


	//  --------  Follow Camera  --------
	if (mCamera)
	{
		fCam = new FollowCamera(mCamera);
		fCam->mGoalNode = pMainNode;
		fCam->loadCameras();
	}

	String s = pSet->shaders == 0 ? "_old" : "";
	sMtr[Mtr_CarBody]		= "car_body"+s;
	sMtr[Mtr_CarInterior]	= "car_interior"+s;
	sMtr[Mtr_CarGlass]		= "car_glass"+s;
	sMtr[Mtr_CarTireFront]	= "cartire_front"+s;
	sMtr[Mtr_CarTireRear]	= "cartire_rear"+s;

	//  car Models:  body, interior, glass  -------
	//vis flags:  2 not rendered in reflections  16 off by in-car camera
	SceneNode* ncart = pMainNode->createChildSceneNode();
	
	//  body  ----------------------
	Ogre::Vector3 vPofs(0,0,0);

	if (FileExists(resCar + "/body.mesh"))
	{
		Entity* eCar = pSceneMgr->createEntity("Car", "body.mesh");
		if (FileExists(resCar + "/body00_add.png") && FileExists(resCar + "/body00_red.png"))
			eCar->setMaterialName(sMtr[Mtr_CarBody]);
		ncart->attachObject(eCar);  eCar->setVisibilityFlags(2);
	}else{
		ManualObject* mCar = CreateModel(pSceneMgr, sMtr[Mtr_CarBody], &pCar->bodymodel.mesh, vPofs);
		if (mCar){	ncart->attachObject(mCar);  mCar->setVisibilityFlags(2);  }
	}

	//  interior  ----------------------
	vPofs = Vector3(pCar->vInteriorOffset[0],pCar->vInteriorOffset[1],pCar->vInteriorOffset[2]);  //x+ back y+ down z+ right

	if (FileExists(resCar + "/interior.mesh"))
	{
		Entity* eInter = pSceneMgr->createEntity("Car.interior", "interior.mesh");
		eInter->setMaterialName(sMtr[Mtr_CarInterior]);
		ncart->attachObject(eInter);  eInter->setVisibilityFlags(2);
	}else{
		ManualObject* mInter = CreateModel(pSceneMgr, sMtr[Mtr_CarInterior],&pCar->interiormodel.mesh, vPofs);
		if (mInter){  ncart->attachObject(mInter);  mInter->setVisibilityFlags(2);  }
	}
	
	//  glass  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(resCar + "/glass.mesh"))
	{
		Entity* eGlass = pSceneMgr->createEntity("Car.glass", "glass.mesh");
		eGlass->setMaterialName(sMtr[Mtr_CarGlass]);
		eGlass->setRenderQueueGroup(RENDER_QUEUE_8);  eGlass->setVisibilityFlags(16);
		ncart->attachObject(eGlass);
	}else{
		ManualObject* mGlass = CreateModel(pSceneMgr, sMtr[Mtr_CarGlass], &pCar->glassmodel.mesh, vPofs);
		if (mGlass){  ncart->attachObject(mGlass);	mGlass->setRenderQueueGroup(RENDER_QUEUE_8);  mGlass->setVisibilityFlags(16);  }
	}

	/*ManualObject* mDriv  = CreateModel("car/driver",   &pCar->drivermodel.mesh);
	if (mDriv)
	{	//MATHVECTOR<float,3> pos = pCar->drivernode->GetTransform().GetTranslation();
		//Vector3 tr(pos[1], pos[2], pos[0]);
		SceneNode* ndriv = ncart->createChildSceneNode(tr);
		ndriv->attachObject(mDriv);
	}/**/
	
	///  save .mesh
	/**  MeshPtr mpCar = mCar->convertToMesh("MeshCar");
	Ogre::MeshSerializer* msr = new Ogre::MeshSerializer();
	msr->exportMesh(mpCar.getPointer(), "car.mesh");/**/


	//  wheels  ----------------------
	for (int w=0; w < 4; w++)
	{
		if (w < 2 && FileExists(resCar + "/wheel_front.mesh"))
		{
			Entity* eWh = pSceneMgr->createEntity("Wheel"+toStr(w), "wheel_front.mesh");
			eWh->setMaterialName(sMtr[Mtr_CarTireFront]);
			ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
		}else
		if (FileExists(resCar + "/wheel_rear.mesh"))
		{
			Entity* eWh = pSceneMgr->createEntity("Wheel"+toStr(w), "wheel_rear.mesh");
			eWh->setMaterialName(sMtr[Mtr_CarTireRear]);
			ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
		}else{
			ManualObject* mWh;
			if (w < 2)	mWh = CreateModel(pSceneMgr, sMtr[Mtr_CarTireFront], &pCar->wheelmodelfront.mesh, vPofs, true);
			else		mWh = CreateModel(pSceneMgr, sMtr[Mtr_CarTireRear],  &pCar->wheelmodelrear.mesh, vPofs, true);
			if (mWh)  {
				ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(mWh);  mWh->setVisibilityFlags(2);  }
		}
	}


	///  wheel emitters  ------------------------
	for (int w=0; w < 4; w++)
	{		
		if (!ps[w])  {
			ps[w] = pSceneMgr->createParticleSystem("Smoke"+toStr(w), sc->sParSmoke);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ps[w]);
			ps[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pm[w])  {
			pm[w] = pSceneMgr->createParticleSystem("Mud"+toStr(w), sc->sParMud);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pm[w]);
			pm[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pd[w])  {
			pd[w] = pSceneMgr->createParticleSystem("Dust"+toStr(w), sc->sParDust);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pd[w]);
			pd[w]->getEmitter(0)->setEmissionRate(0);  }

		//  trails
		if (!ndWhE[w])
			ndWhE[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();

		if (!whTrl[w])  {
			NameValuePairList params;
			params["numberOfChains"] = "1";
			params["maxElements"] = toStr(320 * pSet->trails_len);  //80

			whTrl[w] = (RibbonTrail*)pSceneMgr->createMovableObject("RibbonTrail", &params);
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			pSceneMgr->getRootSceneNode()->attachObject(whTrl[w]);
			whTrl[w]->setMaterialName("TireTrail");
			whTrl[w]->setCastShadows(false);
			whTrl[w]->addNode(ndWhE[w]);
		}
			whTrl[w]->setTrailLength(90 * pSet->trails_len);  //30
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			whTrl[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08 * 1.f / pSet->trails_len);
			whTrl[w]->setInitialWidth(0, 0.16);  //0.18 0.2
	}

	UpdParsTrails();
	
	ChangeClr();

	//  reload car materials, omit car and road
	for (int i=1; i < NumMaterials; ++i)
		ReloadTex(sMtr[i]);

}
void CarModel::ReloadTex(String mtrName)
{
	MaterialPtr mtr = (MaterialPtr)MaterialManager::getSingleton().getByName(mtrName);
	if (!mtr.isNull())
	{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
		while (techIt.hasMoreElements())
		{	Technique* tech = techIt.getNext();
			Technique::PassIterator passIt = tech->getPassIterator();
			while (passIt.hasMoreElements())
			{	Pass* pass = passIt.getNext();
				Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
				while (tusIt.hasMoreElements())
				{	TextureUnitState* tus = tusIt.getNext();  String name = tus->getTextureName();
					if (name != "ReflectionCube")
					{
						Ogre::LogManager::getSingletonPtr()->logMessage( "Tex Reload: " + name );
						TexturePtr tex = (TexturePtr)Ogre::TextureManager::getSingleton().getByName( name );
						if (!tex.isNull())
						{							
							tex->reload();
						}
					}
				}
	}	}	}	
}
void CarModel::ChangeClr(void)
{
	///TODO allow multiple cars here, i.e. give mat/tex an index
	bool add = 1;
	Image ima;	try{
		ima.load("body00_add.png","General");  // add, not colored
	}catch(...){  add = 0;  }
	uchar* da = 0;  size_t incRow,incRowA=0, inc1=0,inc1A=0;
	if (add)
	{	PixelBox pba = ima.getPixelBox();
		da = (uchar*)pba.data;  incRowA = pba.rowPitch;
		inc1A = PixelUtil::getNumElemBytes(pba.format);
	}
	String svName = PATHMANAGER::GetCacheDir() + "/body_dyn.png";  // dynamic
	Image im;  try{
		im.load("body00_red.png","General");  // original red diffuse
	}catch(...){  return;  }
	if (im.getWidth())
	{
		PixelBox pb = im.getPixelBox();
		size_t xw = pb.getWidth(), yw = pb.getHeight();

		uchar* d = (uchar*)pb.data;  incRow = pb.rowPitch;
		inc1 = PixelUtil::getNumElemBytes(pb.format);

		Ogre::LogManager::getSingleton().logMessage(
			"img clr +++  w "+toStr(xw)+"  h "+toStr(yw)+"  pf "+toStr(pb.format)+"  iA "+toStr(inc1A));

		size_t x,y,a,aa;
		for (y = 0; y < yw; ++y)
		{	a = y*incRow*inc1, aa = y*incRowA*inc1A;
		for (x = 0; x < xw; ++x)
		{
			uchar r,g,b;
			if (da && da[aa+3] > 60)  // adding area (not transparent)
			{	r = da[aa];  g = da[aa+1];  b = da[aa+2];	}
			else
			{	r = d[a], g = d[a+1], b = d[a+2];  // get
				ColourValue c(r/255.f,g/255.f,b/255.f);  //

				Real h,s,v;  // hue shift
				c.getHSB(&h,&s,&v);
				h += pSet->car_hue;  if (h>1.f) h-=1.f;  // 0..1
				s += pSet->car_sat;  // -1..1
				v += pSet->car_val;
				c.setHSB(h,s,v);

				r = c.r*255;  g = c.g*255;  b = c.b*255;  // set
			}
			d[a] = r;  d[a+1] = g;  d[a+2] = b;	 // write back
			a += inc1;  aa += inc1A;  // next pixel
		}	}
	}
	im.save(svName);
	ReloadTex(sMtr[Mtr_CarBody]);
}
void CarModel::UpdParsTrails()
{
	for (int w=0; w < 4; w++)
	{
		Ogre::uint8 grp = RENDER_QUEUE_9;  //9=road  after glass
		if (whTrl[w]){  whTrl[w]->setVisible(pSet->trails);  whTrl[w]->setRenderQueueGroup(grp);  }  grp += 2;
		if (ps[w])	{	ps[w]->setVisible(pSet->particles);  ps[w]->setRenderQueueGroup(grp);  }  // vdr only && !sc.ter
		if (pm[w])	{	pm[w]->setVisible(pSet->particles);  pm[w]->setRenderQueueGroup(grp);  }
		if (pd[w])	{	pd[w]->setVisible(pSet->particles);  pd[w]->setRenderQueueGroup(grp);  }
	}
}
///  terrain mtr from blend maps
//-------------------------------------------------------------------------------------------------------
void CarModel::UpdWhTerMtr()
{
	if (!pCar || !ndWh[0])  return;
	if (!terrain || !blendMtr)	// vdr trk
	{
		for (int i=0; i<4; ++i)  // for particles/trails only
			whTerMtr[i] = pCar->dynamics.bWhOnRoad[i] ? 0 : 1;
		return;
	}

	int t = blendMapSize;
	Real tws = sc->td.fTerWorldSize;

	//  wheels
	for (int i=0; i<4; ++i)
	{
		Vector3 w = ndWh[i]->getPosition();
		int mx = (w.x + 0.5*tws)/tws*t, my = (w.z + 0.5*tws)/tws*t;
		mx = max(0,min(t-1, mx)), my = max(0,min(t-1, my));

		int mtr = blendMtr[my*t + mx];
		if (pCar->dynamics.bWhOnRoad[i])
			mtr = 0;
		whTerMtr[i] = mtr;

		///  vdr set surface for wheel
		TRACKSURFACE* tsu = &pGame->track.tracksurfaces[mtr];
		pCar->dynamics.terSurf[i] = tsu;
		pCar->dynamics.bTerrain = true;
	}
}

ManualObject* CarModel::CreateModel(SceneManager* sceneMgr, const String& mat, class VERTEXARRAY* a, Vector3 vPofs, bool flip, bool track)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m = sceneMgr->createManualObject();
	m->begin(mat, RenderOperation::OT_TRIANGLE_LIST);

	int t = 0;
	if (track)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v+0], a->vertices[v+2], -a->vertices[v+1]);
			if (norms)
			m->normal(	a->normals [v+0], a->normals [v+2], -a->normals [v+1]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; ++f)
			m->index(a->faces[f]);
	}else
	if (flip)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v], a->vertices[v+1], a->vertices[v+2]);
			if (norms)
			m->normal(  a->normals [v], a->normals [v+1], a->normals [v+2]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}else
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(-a->vertices[v+1]+vPofs.x, -a->vertices[v+2]+vPofs.y, a->vertices[v]+vPofs.z);
			if (norms)
			m->normal(	-a->normals [v+1], -a->normals [v+2], a->normals [v]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}
	m->end();
	return m;
}

