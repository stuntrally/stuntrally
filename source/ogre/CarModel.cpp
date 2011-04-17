#include "stdafx.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"

#include "boost/filesystem.hpp"
#define FileExists(s) boost::filesystem::exists(s)

CarModel::CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s) : 
	hue(0), sat(0), val(0), fCam(0), pMainNode(0), pCar(0)
{
	iIndex = index;
	sDirname = name;
	pSceneMgr = sceneMgr;
	pSet = set;
	pGame = game;
	sc = s;
	
	///TODO create pCar
	
	pReflect = new CarReflection(pSet, pSceneMgr, iIndex);
}
CarModel::~CarModel(void)
{
	delete pReflect;
	
	delete fCam;
	pSceneMgr->destroyCamera("CarCamera" + iIndex);
	
	///TODO allow multiple cars
	Ogre::Root::getSingletonPtr()->removeResourceLocation(PATHMANAGER::GetCacheDir());
	if (sDirname != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(sDirname/*, "DynRes"/**/);
		sDirname = PATHMANAGER::GetCarPath() + "/" + pSet->car + "/textures";
	/*if (resTrk != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(resTrk);
		resTrk = TrkDir() + "objects";*/
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
void CarModel::Update(void)
{
	pReflect->camPosition = pMainNode->getPosition();
	pReflect->Update();
}
void CarModel::Create(void)
{	
	///TODO seperate (CreateMeshes, CreateCamera, ...) [network cars don't need camera]
	///TODO multiple cars
	Ogre::Root::getSingletonPtr()->addResourceLocation(PATHMANAGER::GetCacheDir(), "FileSystem");
	Ogre::Root::getSingletonPtr()->addResourceLocation(sDirname, "FileSystem"/*, "DynRes"/**/);
	//mRoot->addResourceLocation(resTrk, "FileSystem");
	//mRoot->addResourceLocation(resDrv, "FileSystem");

	pMainNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
	pCar = &(*pGame->cars.begin());


	//  --------  Follow Camera  --------
	Ogre::Camera* cam = pSceneMgr->createCamera("CarCamera" + iIndex);
	fCam = new FollowCamera(cam);
	fCam->mGoalNode = pMainNode;
	fCam->loadCameras();

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
	vPofs = Ogre::Vector3(0,0,0);

	if (FileExists(sDirname + "/body.mesh"))
	{
		Entity* eCar = pSceneMgr->createEntity("Car", "body.mesh");
		if (FileExists(sDirname + "/body00_add.png") && FileExists(sDirname + "/body00_red.png"))
			eCar->setMaterialName(sMtr[Mtr_CarBody]);
		ncart->attachObject(eCar);  eCar->setVisibilityFlags(2);
	}else{
		ManualObject* mCar = CreateModel(sMtr[Mtr_CarBody], &pCar->bodymodel.mesh);
		if (mCar){	ncart->attachObject(mCar);  mCar->setVisibilityFlags(2);  }
	}

	//  interior  ----------------------
	vPofs = Vector3(pCar->vInteriorOffset[0],pCar->vInteriorOffset[1],pCar->vInteriorOffset[2]);  //x+ back y+ down z+ right

	if (FileExists(sDirname + "/interior.mesh"))
	{
		Entity* eInter = pSceneMgr->createEntity("Car.interior", "interior.mesh");
		eInter->setMaterialName(sMtr[Mtr_CarInterior]);
		ncart->attachObject(eInter);  eInter->setVisibilityFlags(2);
	}else{
		ManualObject* mInter = CreateModel(sMtr[Mtr_CarInterior],&pCar->interiormodel.mesh);
		if (mInter){  ncart->attachObject(mInter);  mInter->setVisibilityFlags(2);  }
	}
	
	//  glass  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(sDirname + "/glass.mesh"))
	{
		Entity* eGlass = pSceneMgr->createEntity("Car.glass", "glass.mesh");
		eGlass->setMaterialName(sMtr[Mtr_CarGlass]);
		eGlass->setRenderQueueGroup(RENDER_QUEUE_8);  eGlass->setVisibilityFlags(16);
		ncart->attachObject(eGlass);
	}else{
		ManualObject* mGlass = CreateModel(sMtr[Mtr_CarGlass], &pCar->glassmodel.mesh);
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
		if (w < 2 && FileExists(sDirname + "/wheel_front.mesh"))
		{
			Entity* eWh = pSceneMgr->createEntity("Wheel"+toStr(w), "wheel_front.mesh");
			eWh->setMaterialName(sMtr[Mtr_CarTireFront]);
			ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
		}else
		if (FileExists(sDirname + "/wheel_rear.mesh"))
		{
			Entity* eWh = pSceneMgr->createEntity("Wheel"+toStr(w), "wheel_rear.mesh");
			eWh->setMaterialName(sMtr[Mtr_CarTireRear]);
			ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
		}else{
			ManualObject* mWh;
			if (w < 2)	mWh = CreateModel(sMtr[Mtr_CarTireFront], &pCar->wheelmodelfront.mesh, true);
			else		mWh = CreateModel(sMtr[Mtr_CarTireRear],  &pCar->wheelmodelrear.mesh, true);
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

	//  rain  -----
	///TODO move this to App (rain doesn't belong to car)
	/*if (!pr && sc.rainEmit > 0)  {
		pr = pSceneMgr->createParticleSystem("Rain", sc.rainName);
		pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr->getEmitter(0)->setEmissionRate(0);  }
	//  rain2  =====
	if (!pr2 && sc.rain2Emit > 0)  {
		pr2 = pSceneMgr->createParticleSystem("Rain2", sc.rain2Name);
		pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr2->getEmitter(0)->setEmissionRate(0);  }*/

	UpdParsTrails();
	
	ChangeClr();

	//  reload car materials, omit car and road
	///TODO
	/*for (int i=1; i < Mtr_Road; ++i)
		reloadMtrTex(sMtr[i]);*/

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
	//reloadMtrTex(sMtr[Mtr_CarBody]);
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
ManualObject* CarModel::CreateModel(const String& mat, class VERTEXARRAY* a, bool flip, bool track)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m = pSceneMgr->createManualObject();
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

