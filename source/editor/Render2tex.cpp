#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/RenderConst.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

#include "../ogre/common/MaterialGen/MaterialFactory.h"
using namespace Ogre;


//  Setup render 2 texture (road maps)
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::Rnd2TexSetup()
{
	/// rt:  0 road minimap,  1 road for grass,  2 terrain minimap,  3 track preview full
	// visibility:  1 road  2 hud,ed  4 terrain  8 trees  16-car glass  32 sky
	const Ogre::uint32 visMask[RTs] =
		{ RV_Road, RV_Road, RV_Terrain, RV_MaskAll-RV_Hud };
	const int dim[RTs] =  //1025: sc.td.iVertsX
		{ 1024, 1025, 512, 1024 };
		
	AxisAlignedBox big(-100000.0*Vector3::UNIT_SCALE, 100000.0*Vector3::UNIT_SCALE);
	Real sz = pSet->size_minimap;
	xm1 = 1-sz/asp, ym1 = -1+sz, xm2 = 1.0, ym2 = -1.0;
	
	for (int i=0; i < RTs+RTsAdd; ++i)
	{
		SRndTrg& r = rt[i];  bool full = i==3;
		String si = toStr(i), sMtr = /*i==3 ? "road_mini_add" :*/ "road_mini_"+si;
		if (i < RTs)
		{
			String sTex = "RttTex"+si, sCam = "RttCam"+si;

			Ogre::TextureManager::getSingleton().remove(sTex);
			mSceneMgr->destroyCamera(sCam);  // dont destroy old - const tex sizes opt..
			
			///  rnd to tex - same dim as Hmap	// after track load
			Real fDim = sc.td.fTerWorldSize;  // world dim
			Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(sTex,
				  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
				  dim[i], dim[i], 0, PF_R8G8B8A8, TU_RENDERTARGET);
				  
			r.rndCam = mSceneMgr->createCamera(sCam);  // up
			r.rndCam->setPosition(Vector3(0,1000,0));  r.rndCam->setOrientation(Quaternion(0.5,-0.5,0.5,0.5));
			r.rndCam->setNearClipDistance(0.5);		r.rndCam->setFarClipDistance(50000);
			r.rndCam->setAspectRatio(1.0);			if (!full)  r.rndCam->setProjectionType(PT_ORTHOGRAPHIC);
			r.rndCam->setOrthoWindow(fDim,fDim);	//rt[i].rndCam->setPolygonMode(PM_WIREFRAME);

			r.rndTex = texture->getBuffer()->getRenderTarget();
			r.rndTex->setAutoUpdated(false);	r.rndTex->addListener(this);
			Viewport* rvp = r.rndTex->addViewport(r.rndCam);
			rvp->setClearEveryFrame(true);   rvp->setBackgroundColour(ColourValue(0,0,0,0));
			rvp->setOverlaysEnabled(false);  rvp->setSkiesEnabled(full);
			rvp->setVisibilityMask(visMask[i]);
			rvp->setShadowsEnabled(false);
		}
		///  minimap  . . . . . . . . . . . . . . . . . . . . . . . . . . . 
		if (r.ndMini)  mSceneMgr->destroySceneNode(r.ndMini);
		ResourcePtr mt = Ogre::MaterialManager::getSingleton().getByName(sMtr);
		if (!mt.isNull())  mt->reload();

		r.rcMini = new Ogre::Rectangle2D(true);  // screen rect preview
		if (i == RTs)  r.rcMini->setCorners(-1/asp, 1, 1/asp, -1);  // fullscr,square
		else  r.rcMini->setCorners(xm1, ym1, xm2, ym2);  //+i*sz*all

		r.rcMini->setBoundingBox(big);
		r.ndMini = mSceneMgr->getRootSceneNode()->createChildSceneNode("Minimap"+si);
		r.ndMini->attachObject(r.rcMini);	r.rcMini->setCastShadows(false);
		r.rcMini->setMaterial(i == RTs+1 ? "BrushPrvMtr" : sMtr);
		r.rcMini->setRenderQueueGroup(RQG_Hud2);
		r.rcMini->setVisibilityFlags(i == RTs ? RV_MaskPrvCam : RV_Hud);
	}

	//  pos dot on minimap  . . . . . . . .
	if (!ndPos)  {
		mpos = Create2D("hud/CarPos", 0.2f, true);  // dot size
		mpos->setVisibilityFlags(RV_Hud);
		mpos->setRenderQueueGroup(RQG_Hud3 /*RENDER_QUEUE_OVERLAY+1*/);
		ndPos = mSceneMgr->getRootSceneNode()->createChildSceneNode(
			Vector3(xm1+(xm2-xm1)/2, ym1+(ym2-ym1)/2, 0));
		float fHudSize = 0.04f;
		ndPos->scale(fHudSize, fHudSize, 1);
		ndPos->attachObject(mpos);  }
	if (ndPos)   ndPos->setVisible(pSet->trackmap);
	UpdMiniVis();
}

void App::UpdMiniVis()
{
	for (int i=0; i < RTs+RTsAdd; ++i)
		if (rt[i].ndMini)
			rt[i].ndMini->setVisible(pSet->trackmap && (i == pSet->num_mini));
}


///  Image from road
///  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::SaveGrassDens()
{
	for (int i=0; i < RTs-1; ++i)  //-1 preview camera manual
	{
		if (!rt[i].rndTex)  return;
		rt[i].rndTex->update();  // all have to exist
	}

	int w = rt[1].rndTex->getWidth(), h = rt[1].rndTex->getHeight();
	using Ogre::uint;
	uint *rd = new uint[w*h];   // road render
	uint *gd = new uint[w*h];   // grass dens
	PixelBox pb_rd(w,h,1, PF_BYTE_RGBA, rd);
	rt[1].rndTex->copyContentsToMemory(pb_rd, RenderTarget::FB_FRONT);

	const int f = std::max(0, sc.grDensSmooth);
	float ff = 0.f;  //2.f / ((f*2+1)*(f*2+1)) / 255.f;
	register int v,i,j,m,x, a,b,d,y;

	//  gauss kernel for smoothing
	int *mask = new int[(f*2+1)*(f*2+1)];  m = 0;
	for (j = -f; j <= f; ++j)
	for (i = -f; i <= f; ++i, ++m)
	{
		v = std::max(0.f, (1.f - sqrtf((float)(i*i+j*j)) / float(f)) * 256.f);
		mask[m] = v;  ff += v;
	}
	ff = 2.f / ff;  // normally would be 1.f - but road needs to stay black and be smooth outside
	//  change smooth to distance from road with fade ?..
		
	///  road - rotate, smooth  -----------
	for (y = f; y < h-f; ++y) {  a = y*w +f;
	for (x = f; x < w-f; ++x, ++a)
	{	b = x*w + (h-y);  // rot 90 ccw  b=a
		
		//  sum kernel
		v = 0;  m = 0;
		for (j = -f; j <= f; ++j) {  d = b -f + j*w;
		for (i = -f; i <= f; ++i, ++d, ++m)
			v += ((rd[d] >> 16) & 0xFF) * mask[m] / 256;  }

		v = std::max(0, (int)(255.f * (1.f - v * ff) ));  // avg, inv, clamp
		
		gd[a] = 0xFF000000 + /*0x010101 */ v;  // write
	}	}

	v = 0xFFFFFFFF;  //  frame f []  todo: get from rd[b] not clear..
	for (y = 0;  y <= f; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - up
	for (y=h-f-1; y < h; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - down
	for (x = 0;  x <= f; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | left
	for (x=w-f-1; x < w; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | right

	Image im;  // for trees, before grass angle and height
	im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
	im.save(TrkDir()+"objects/roadDensity.png");

	///  terrain - max angle, height for grass  -----------
	for (y = 0; y < h; ++y) {  b = y*w;
	for (x = 0; x < w; ++x, ++b)
	{
		a = (h-1-y) * sc.td.iVertsY / h;  a *= sc.td.iVertsX;
		a += x * sc.td.iVertsX / w;
		// would be better to interpolate 4 neighbours, or smooth this map

		float ad = std::max(0.f, sc.td.hfAngle[a] - sc.grTerMaxAngle);  // ang diff
		float hd = std::max(0.f, sc.td.hfHeight[a] - sc.grTerMaxHeight);  // height diff
		if (ad > 0.f || hd > 0.f)
		{
			d = 20.f * ad + 20.f * hd;  //par mul,  smooth transition ..
			v = std::max(0, std::min(255, 255-d));
			v = std::min((int)(gd[b] & 0xFF), v);  // preserve road
			gd[b] = 0xFF000000 + /*0x010101 */ v;  // no grass
		}
	}	}

	//Image im;
	im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
	im.save(TrkDir()+"objects/grassDensity.png");

	delete[] rd;  delete[] gd;  delete[] mask;

	//  road  ----------------
	int u = pSet->allow_save ? pSet->gui.track_user : 1;
	rt[0].rndTex->writeContentsToFile(pathTrkPrv[u] + pSet->gui.track + "_mini.png");
	//  terrain
	rt[2].rndTex->writeContentsToFile(pathTrkPrv[u] + pSet->gui.track + "_ter.jpg");
}


///  pre and post  rnd to tex
//-----------------------------------------------------------------------------------------------------------
void App::preRenderTargetUpdate(const RenderTargetEvent &evt)
{
	if (evt.source->getViewport(0)->getCamera()->getName() !=  "RttCam3")
	{
		if (!terrain)  return;
		MaterialPtr terrainMaterial = terrain->_getMaterial();
		if (!terrainMaterial.isNull())
		{
			for (int i=0; i < terrainMaterial->getNumTechniques(); ++i)
			{
				if (terrainMaterial->getTechnique(i)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("enableShadows"))
					terrainMaterial->getTechnique(i)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("enableShadows", 0.f);
			}
		}
		if (materialFactory)
			materialFactory->setShadowsEnabled(false);
	}
	
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());
	
	//terrain->setVisibilityFlags(0);
	
	if (num == 3)  // full
	{
		rt[3].rndCam->setPosition(mCameraT->getPosition());
		rt[3].rndCam->setDirection(mCameraT->getDirection());
		//ndSky->setVisible(true);
		//ndSky->setPosition(mCameraT->getPosition());
	}
	else
	if (road)
		road->SetForRnd(num == 0 ? "render_clr" : "render_grass");
}

void App::postRenderTargetUpdate(const RenderTargetEvent &evt)
{
	if (evt.source->getViewport(0)->getCamera()->getName() !=  "RttCam3")
	{
		if (!terrain)  return;
		MaterialPtr terrainMaterial = terrain->_getMaterial();
		if (!terrainMaterial.isNull())
		{
			for (int i=0; i < terrainMaterial->getNumTechniques(); ++i)
			{
				if (terrainMaterial->getTechnique(i)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("enableShadows"))
					terrainMaterial->getTechnique(i)->getPass(0)->getFragmentProgramParameters()->setNamedConstant("enableShadows", 1.f);
			}
		}
		if (materialFactory)
			materialFactory->setShadowsEnabled(true);
	}
	
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());

	if (num == 3)  // full
	{}
	else
	if (road)  {
		road->UnsetForRnd();
		road->UpdLodVis(pSet->road_dist);  }

	//  restore shadows splits todo...
	//mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	//mCamera->setNearClipDistance(0.1f);
	//UpdPSSMMaterials();
}


///  save water depth map
//-----------------------------------------------------------------------------------------------------------
void App::SaveWaterDepth()
{
	if (sc.fluids.empty())
	{
		Delete(TrkDir()+"objects/waterDepth.png");  // no tex if no fluids
		// save white texture, copy white.png
		//Copy(PATHMANAGER::GetDataPath()+"/materials/white.png",TrkDir()+"objects/waterDepth.png");
		return;
	}
	QTimer ti;  ti.update();  ///T  /// time

	//  2048 for bigger terrains ?
	int w = 1024, h = w;  float fh = h-1, fw = w-1;
	using Ogre::uint;
	uint *wd = new uint[w*h];   // water depth
	register int x,y,a,i,ia,id;
	register float fa,fd;
	
	btVector3 from(0,0,0), to = from;
	btCollisionWorld::ClosestRayResultCallback rayRes(from, to);
		
	///  write to img  -----------
	//  get ter height to fluid height difference for below
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		float fx = float(y)/fh, fz = float(x)/fw;
		//  pos on ter  -terSize..terSize
		float wx = (fx-0.5f) * sc.td.fTerWorldSize, wz = -(fz-0.5f) * sc.td.fTerWorldSize;

		fa = 0.f;  // fluid y pos
		for (i=0; i < sc.fluids.size(); ++i)
		{
			const FluidBox& fb = sc.fluids[i];
			const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
			//  check rect 2d - no rot !  todo: make 2nd type circle..
			if (wx > fb.pos.x - sizex && wx < fb.pos.x + sizex &&
				wz > fb.pos.z - sizez && wz < fb.pos.z + sizez)
			{
				float f = fb.pos.y - terrain->getHeightAtTerrainPosition(fx,fz);
				if (f > fa)  fa = f;
			}
		}
		fd = fa * 0.4f * 255.f;  // depth far
		fa = fa * 8.f * 255.f;  // alpha near

		ia = std::max(0, std::min(255, (int)fa ));  // clamp
		id = std::max(0, std::min(255, (int)fd ));
		
		wd[a] = 0xFF000000 + /*0x01 */ ia + 0x0100 * id;  // write
	}	}

	Image im;  // save img
	im.loadDynamicImage((uchar*)wd, w,h,1, PF_BYTE_RGBA);
	im.save(TrkDir()+"objects/waterDepth.png");
	delete[] wd;

	try {
	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
	if (!tex.isNull())
		tex->reload();
	else  // 1st fluid after start, refresh matdef ?..
		TextureManager::getSingleton().load("waterDepth.png",ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	} catch(...) {  }


	ti.update();	///T  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time WaterDepth: ") + toStr(dt) + " ms");
}


///  align terrain to road selected segments
//-----------------------------------------------------------------------------------------------------------
void App::AlignTerToRoad()
{
	QTimer ti;  ti.update();  ///T  /// time

	//  setup bullet world
	btDefaultCollisionConfiguration* config;
	btCollisionDispatcher* dispatcher;
	bt32BitAxisSweep3* broadphase;
	btSequentialImpulseConstraintSolver* solver;
	//btDiscreteDynamicsWorld* world;

	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);

	broadphase = new bt32BitAxisSweep3(btVector3(-5000, -5000, -5000), btVector3(5000, 5000, 5000));
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);

	world->setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	world->getSolverInfo().m_restitution = 0.0f;
	world->getDispatchInfo().m_enableSPU = true;
	world->setForceUpdateAllAabbs(false);  //+
	
	//  scene
	//CreateBltFluids();
	//-CreateBltTerrain();  // from terrain->, faster?
	//TODO:
	///  create bullet road for selected segments ...
	//**/road->RebuildRoadInt();  bool blt=true;
	//  get min max x,z from sel segs aabb-s (dont raycast whole terrain)


	int w = 1024, h = w;  float fh = h-1, fw = w-1;
	using Ogre::uint;
	uint *wd = new uint[w*h];   // water depth
	register int x,y,a,ia,id;
	register float fa,fd, fx,fz, wx,wz;
	
	btVector3 from(0,0,0), to = from;
	btCollisionWorld::ClosestRayResultCallback rayRes(from, to);
		
	///  write to img  -----------
	//  get ter height, compare with ray cast to bullet fluids only
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		fx = float(y)/fh;  fz = float(x)/fw;
		//  pos on ter  -terSize..terSize
		wx = (fx-0.5f) * sc.td.fTerWorldSize;  wz = -(fz-0.5f) * sc.td.fTerWorldSize;
		//if (x==0 && y==0 || x==w-1 && y==h-1)  // check
		//	LogO(toStr(fx)+","+toStr(fz)+" "+toStr(wx)+","+toStr(wz));

		//  ray pos,to
		btVector3 from(wx,wz,300), to = from;
		to.setZ(to.getZ() - 600);  // max range
		//  cast  -------
		rayRes.m_rayFromWorld = from;
		rayRes.m_rayToWorld = to;
		world->rayTest(from, to, rayRes);
		if (rayRes.hasHit())
			fa = rayRes.m_hitPointWorld.getZ() *10.01f;  //..
		else  fa = 0.f;  // no fluids

		fd = fa * 0.4f * -255.f;  // depth far
		fa = fa * 8.f * -255.f;  // alpha near

		ia = std::max(0, std::min(255, (int)fa ));  // clamp
		id = std::max(0, std::min(255, (int)fd ));
		
		wd[a] = 0xFF000000 + /*0x01 */ ia + 0x0100 * id;  // write
	}	}

	Image im;  // save img
	im.loadDynamicImage((uchar*)wd, w,h,1, PF_BYTE_RGBA);
	im.save(TrkDir()+"objects/waterDepth2.png");
	delete[] wd;


	//  clear
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
			delete body->getMotionState();

		world->removeCollisionObject(obj);

		ShapeData* sd = static_cast<ShapeData*>(obj->getUserPointer());
		delete sd;
		delete obj;
	}
	delete world;  world = 0;
	delete solver;	delete broadphase;	delete dispatcher;	delete config;


	ti.update();	///T  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time BulletRays: ") + toStr(dt) + " ms");
}
