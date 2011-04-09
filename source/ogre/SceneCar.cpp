#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"

#include "../btOgre/BtOgrePG.h"
#include "../btOgre/BtOgreGP.h"
//#include "BtOgreDebug.h"
#include "../bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"


//  Bullet Terrain
//---------------------------------------------------------------------------------------------------------------

void App::CreateBltTerrain()
{
	btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
		sc.td.iVertsX, sc.td.iVertsY, sc.td.hfData, sc.td.fTriangleSize,
		/*>?*/-100.f,100.f, 2, PHY_FLOAT,false);
	
	hfShape->setUseDiamondSubdivision(true);

	btVector3 scl(sc.td.fTriangleSize, sc.td.fTriangleSize, 1);
	hfShape->setLocalScaling(scl);

	/*btRigidBody::btRigidBodyConstructionInfo infoHm(0.f, 0, hfShape);
	infoHm.m_restitution = 0.5;  //
	infoHm.m_friction = 0.9;  ///.. 0.9~
	pGame->collision.AddRigidBody(infoHm);/**/

	btCollisionObject* col = new btCollisionObject();
	col->setCollisionShape(hfShape);
	//col->setWorldTransform(tr);
	col->setFriction(0.9);
	col->setRestitution(0.5);
	pGame->collision.world.addCollisionObject(col);
	pGame->collision.shapes.push_back(hfShape);/**/

	
	///  border planes []
	const float px[4] = {-1, 1, 0, 0};
	const float py[4] = { 0, 0,-1, 1};
	if (1)
	for (int i=0; i < 4; i++)
	{
		btVector3 vpl(px[i], py[i], 0);
		btCollisionShape* shp = new btStaticPlaneShape(vpl,0);
		
		btTransform tr;  tr.setIdentity();
		tr.setOrigin(vpl * -0.5 * sc.td.fTerWorldSize);

		btDefaultMotionState* ms = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f,ms,shp);
		pGame->collision.AddRigidBody(rbInfo);
	}
}


///  car body  color change  * * * *
///---------------------------------------------------------------------------------------------------------------
void App::CarChangeClr()
{
	if (pGame->cars.size() == 0)  return;
	
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
	reloadMtrTex(sMtr[Mtr_CarBody]);
}


///---------------------------------------------------------------------------------------------------------------
///  Car model
///---------------------------------------------------------------------------------------------------------------

void App::CreateCar()
{
	// recreate, destroy node, ent ...
	mRoot->removeResourceLocation(PATHMANAGER::GetCacheDir());
	if (resCar != "")  mRoot->removeResourceLocation(resCar/*, "DynRes"/**/);
		resCar = PATHMANAGER::GetCarPath() + "/" + pSet->car + "/textures";
	if (resTrk != "")  mRoot->removeResourceLocation(resTrk);
		resTrk = TrkDir() + "objects";
	if (resDrv != "")  mRoot->removeResourceLocation(resDrv);
		resDrv = "drivers/driver2/textures";
		
	if (pGame->cars.size() == 0)  return;

	mRoot->addResourceLocation(PATHMANAGER::GetCacheDir(), "FileSystem");
	mRoot->addResourceLocation(resCar, "FileSystem"/*, "DynRes"/**/);
	mRoot->addResourceLocation(resTrk, "FileSystem");
	mRoot->addResourceLocation(resDrv, "FileSystem");

	ndCar = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	CAR* pCar = &(*pGame->cars.begin());


	//  --------  Follow Camera  --------
	mFCam->mGoalNode = ndCar;

	String s = pSet->shaders == 0 ? "_old" : "";
	sMtr[Mtr_CarBody]		= "car_body"+s;
	sMtr[Mtr_CarInterior]	= "car_interior"+s;
	sMtr[Mtr_CarGlass]		= "car_glass"+s;
	sMtr[Mtr_CarTireFront]	= "cartire_front"+s;
	sMtr[Mtr_CarTireRear]	= "cartire_rear"+s;
	sMtr[Mtr_Road]			= "road"+s;


	//  car Models:  body, interior, glass  -------
	//vis flags:  2 not rendered in reflections  16 off by in-car camera
	SceneNode* ncart = ndCar->createChildSceneNode();
		//ncart->setScale(0.5,0.5,0.5);  //test
		//ncart->setOrientation(Quaternion(Degree(180), Vector3(1,0,0)));
		//ncart->setPosition(0,-0.5,0);

	//  body  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(resCar + "/body.mesh"))
	{
		Entity* eCar = mSceneMgr->createEntity("Car", "body.mesh");
		//SceneNode* nc = ndCar->createChildSceneNode();
		ncart->attachObject(eCar);  eCar->setVisibilityFlags(2);
	}else{
		ManualObject* mCar = CreateModel(sMtr[Mtr_CarBody], &pCar->bodymodel.mesh);
		if (mCar){	ncart->attachObject(mCar);  mCar->setVisibilityFlags(2);  }
	}

	//  interior  ----------------------
	vPofs = Vector3(pCar->vInteriorOffset[0],pCar->vInteriorOffset[1],pCar->vInteriorOffset[2]);  //x+ back y+ down z+ right

	if (FileExists(resCar + "/interior.mesh"))
	{
		Entity* eInter = mSceneMgr->createEntity("Car.interior", "interior.mesh");
		ncart->attachObject(eInter);  eInter->setVisibilityFlags(2);
	}else{
		ManualObject* mInter = CreateModel(sMtr[Mtr_CarInterior],&pCar->interiormodel.mesh);
		if (mInter){  ncart->attachObject(mInter);  mInter->setVisibilityFlags(2);  }
	}
	
	//  glass  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(resCar + "/glass.mesh"))
	{
		Entity* eGlass = mSceneMgr->createEntity("Car.glass", "glass.mesh");
		ncart->attachObject(eGlass);  eGlass->setRenderQueueGroup(RENDER_QUEUE_8);  eGlass->setVisibilityFlags(16);
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
		if (w < 2 && FileExists(resCar + "/wheel_front.mesh"))
		{
			Entity* eWh = mSceneMgr->createEntity("Wheel"+toStr(w), "wheel_front.mesh");
			ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);
		}else
		if (FileExists(resCar + "/wheel_rear.mesh"))
		{
			Entity* eWh = mSceneMgr->createEntity("Wheel"+toStr(w), "wheel_rear.mesh");
			ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);
		}else{
			ManualObject* mWh;
			if (w < 2)	mWh = CreateModel(sMtr[Mtr_CarTireFront], &pCar->wheelmodelfront.mesh, true);
			else		mWh = CreateModel(sMtr[Mtr_CarTireRear],  &pCar->wheelmodelrear.mesh, true);
			if (mWh)  {
				mWh->setVisibilityFlags(2);
				ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(mWh);  }
		}
			
		///  Ray info  *----*
		/*Entity* entS,*entD;
		entS = mSceneMgr->createEntity("RayS"+toStr(w), "sphere.mesh");
		entS->setMaterialName("sphere_norm");  entS->setCastShadows(false);
		entD = mSceneMgr->createEntity("RayD"+toStr(w), "sphere.mesh");
		entD->setMaterialName("sphere_rot");  entD->setCastShadows(false);
		
		ndRs[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		ndRd[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		ndRs[w]->attachObject(entS);  ndRs[w]->setScale(0.2f * Vector3::UNIT_SCALE);
		ndRd[w]->attachObject(entD);  ndRd[w]->setScale(0.2f * Vector3::UNIT_SCALE);/**/
	}


	///  wheel emitters  ------------------------
	for (int w=0; w < 4; w++)
	{
		if (!ps[w])  {
			ps[w] = mSceneMgr->createParticleSystem("Smoke"+toStr(w), sc.sParSmoke);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ps[w]);
			ps[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pm[w])  {
			pm[w] = mSceneMgr->createParticleSystem("Mud"+toStr(w), sc.sParMud);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pm[w]);
			pm[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pd[w])  {
			pd[w] = mSceneMgr->createParticleSystem("Dust"+toStr(w), sc.sParDust);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pd[w]);
			pd[w]->getEmitter(0)->setEmissionRate(0);  }

		//  trails
		if (!ndWhE[w])
			ndWhE[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		if (!whTrl[w])  {
			NameValuePairList params;
			params["numberOfChains"] = "1";
			params["maxElements"] = toStr(320 * pSet->trails_len);  //80

			whTrl[w] = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			mSceneMgr->getRootSceneNode()->attachObject(whTrl[w]);
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
	if (!pr && sc.rainEmit > 0)  {
		pr = mSceneMgr->createParticleSystem("Rain", sc.rainName);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr->getEmitter(0)->setEmissionRate(0);  }
	//  rain2  =====
	if (!pr2 && sc.rain2Emit > 0)  {
		pr2 = mSceneMgr->createParticleSystem("Rain2", sc.rain2Name);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr2->getEmitter(0)->setEmissionRate(0);  }

	UpdParsTrails();
	
	CarChangeClr();

	//  reload car materials, omit car and road
	for (int i=1; i < Mtr_Road; ++i)
		reloadMtrTex(sMtr[i]);
}


void App::UpdParsTrails()
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
