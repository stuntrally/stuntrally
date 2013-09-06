#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/RenderConst.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

#include <OgreTerrain.h>

using namespace Ogre;


//  Setup render 2 texture (road maps)
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::Rnd2TexSetup()
{
	/// rt:  0 road minimap,  1 road for grass,  2 terrain minimap,  3 track preview full
	// visibility:  1 road  2 hud,ed  4 terrain  8 trees  16-car glass  32 sky
	const Ogre::uint32 visMask[RTs] =
		{ RV_Road, RV_Road+RV_Objects, RV_Terrain+RV_Objects, RV_MaskAll-RV_Hud };
	const int dim[RTs] =  //1025: sc->td.iVertsX
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
			Real fDim = sc->td.fTerWorldSize;  // world dim  ..vdr
			Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(sTex,
				  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
				  dim[i], dim[i], 0, PF_R8G8B8A8, TU_RENDERTARGET);
				  
			r.rndCam = mSceneMgr->createCamera(sCam);  // up
			r.rndCam->setPosition(Vector3(0,1000,0/*-300..*/));  r.rndCam->setOrientation(Quaternion(0.5,-0.5,0.5,0.5));
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

			if (i != 3)
				rvp->setMaterialScheme ("reflection");
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

	const int f = std::max(0, sc->grDensSmooth);
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

	if (!IsVdrTrack())  // vdr trk no grass, only previews
	{
		Image im;  // for trees, before grass angle and height
		im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
		im.save(TrkDir()+"objects/roadDensity.png");

		///  terrain - max angle, height for grass  -----------
		for (y = 0; y < h; ++y) {  b = y*w;
		for (x = 0; x < w; ++x, ++b)
		{
			a = (h-1-y) * sc->td.iVertsY / h;  a *= sc->td.iVertsX;
			a += x * sc->td.iVertsX / w;
			// would be better to interpolate 4 neighbours, or smooth this map

			//for (int i=0; i < sc->ciNumPgLay; ++i)
			//	if (sc->pgLayersAll[i].on)

			uint uu = 0xFF000000;
			SGrassLayer* gr = &sc->grLayersAll[0];  ///todo: grass channels, ter range in r,g,b,a..
			v = std::max(0, std::min(255, int(255.f *
				linRange(sc->td.hfAngle[a], 0.f,gr->terMaxAng, gr->terAngSm) *
				linRange(sc->td.hfHeight[a], gr->terMinH,gr->terMaxH, gr->terHSm) )));
			v = std::min((int)(gd[b] & 0xFF), v);  // preserve road
			uu += v;  // v << (i*8)
			gd[b] = uu;  // no grass
		}	}

		//Image im;
		im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
		im.save(TrkDir()+"objects/grassDensity.png");
	}
	delete[] rd;  delete[] gd;  delete[] mask;

	//  road, terrain  ----------------
	int u = pSet->allow_save ? pSet->gui.track_user : 1;
	rt[0].rndTex->writeContentsToFile(pathTrk[u] + pSet->gui.track + "/preview/road.png");
	rt[2].rndTex->writeContentsToFile(pathTrk[u] + pSet->gui.track + "/preview/terrain.jpg");
}


///  pre and post  rnd to tex
//-----------------------------------------------------------------------------------------------------------
void App::preRenderTargetUpdate(const RenderTargetEvent &evt)
{
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());
	
	if (num == 3)  // full
	{
		rt[3].rndCam->setPosition(mCamera->getPosition());
		rt[3].rndCam->setDirection(mCamera->getDirection());
	}
	else if (road)
		road->SetForRnd(num == 0 ? "render_clr" : "render_grass");
}

void App::postRenderTargetUpdate(const RenderTargetEvent &evt)
{	
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());

	if (num == 3)  // full
	{
	}
	else if (road)
	{	road->UnsetForRnd();
		road->UpdLodVis(pSet->road_dist);
	}

	//  restore shadows splits todo...
	//mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	//mCamera->setNearClipDistance(0.1f);
	//UpdPSSMMaterials();
}


///  save water depth map
//-----------------------------------------------------------------------------------------------------------
void App::SaveWaterDepth()
{
	if (sc->fluids.empty())
	{
		Delete(TrkDir()+"objects/waterDepth.png");  // no tex if no fluids
		return;
	}
	QTimer ti;  ti.update();  ///T  /// time

	//  2048 for bigger terrains ?
	int w = 1024, h = w;  float fh = h-1, fw = w-1;
	using Ogre::uint;
	uint *wd = new uint[w*h];   // water depth
	register int x,y,a,i,ia,id;
	register float fa,fd;
	
	///  write to img  -----------
	//  get ter height to fluid height difference for below
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		float fx = float(y)/fh, fz = float(x)/fw;
		//  pos on ter  -terSize..terSize
		float wx = (fx-0.5f) * sc->td.fTerWorldSize, wz = -(fz-0.5f) * sc->td.fTerWorldSize;

		fa = 0.f;  // fluid y pos
		for (i=0; i < sc->fluids.size(); ++i)
		{
			const FluidBox& fb = sc->fluids[i];
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
	LogO(String("::: Time WaterDepth: ") + fToStr(dt,0,3) + " ms");
}



///  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
///  align terrain to road selected segments
//-----------------------------------------------------------------------------------------------------------
struct RayResult : public btCollisionWorld::RayResultCallback
{
	RayResult(const btVector3& rayFromWorld, const btVector3& rayToWorld)
		: m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld)
	{	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
		
	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		btCollisionObject* obj = rayResult.m_collisionObject;
		if (obj->getUserPointer() != (void*)111)  // allow only road
			return 1.0;

		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = obj;
		
		if (normalInWorldSpace)
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		else  ///need to transform normal into worldspace
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;

		m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
		return rayResult.m_hitFraction;
	}
};

void App::AlignTerToRoad()
{
	if (road->vSel.empty())  return;
	QTimer ti;  ti.update();  ///T  /// time

	///  create bullet road for selected segments
	road->edWmul = pSet->al_w_mul;
	road->edWadd = pSet->al_w_add;
	road->RebuildRoadInt(true);

	//  terrain
	float *fHmap = terrain->getHeightData();
	int w = sc->td.iVertsX, h = w;  float fh = h-1, fw = w-1;

	float *rd = new float[w*h];  // road depth
	bool  *rh = new bool[w*h];  // road hit

	const float Len = 400;  // max ray length
	register int x,y,a;
	register float v,k, fx,fz, wx,wz;
	
	///  ray casts  -----------
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		fx = float(x)/fh;  fz = float(y)/fw;
		//  pos on ter  -terSize..terSize
		wx = (fx-0.5f) * sc->td.fTerWorldSize;  wz = (fz-0.5f) * sc->td.fTerWorldSize;

		btVector3 from(wx,wz,Len), to(wx,wz,-Len);  // x -z y
		RayResult rayRes(from, to);
		world->rayTest(from, to, rayRes);

		//  terrain height if not hit
		rh[a] = rayRes.hasHit();
		rd[a] = rayRes.hasHit() ? rayRes.m_hitPointWorld.getZ() : fHmap[a];
	}	}

	//  smooth edges, road-terrain border
	const float fv = pSet->al_smooth;
	if (fv > 0.5f)
	{
		const int f = std::ceil(fv);
		const unsigned int fs = (f*2+1)*(f*2+1);
		float ff = 0.f;
		register int i,j,m,d,b;

		//  gauss kernel for smoothing
		float *mask = new float[fs];  m = 0;
		for (j = -f; j <= f; ++j)
		for (i = -f; i <= f; ++i, ++m)
		{
			v = std::max(0.f, 1.f - sqrtf((float)(i*i+j*j)) / fv );
			mask[m] = v;  ff += v;
		}
		ff = 1.f / ff;  // smooth, outside (>1.f)
			
		//  sum kernel
		for (y = f; y < h-f; ++y) {  a = y*w +f;
		for (x = f; x < w-f; ++x, ++a)
		{		
			v = 0;  m = 0;  b = 0;
			for (j = -f; j <= f; ++j) {  d = a -f + j*w;
			for (i = -f; i <= f; ++i, ++d, ++m)
			{	k = mask[m];  //maskB ?
				v += rd[d] * k;
				if (rh[d] && k > 0.1f)  ++b;
			}	}
			if (b > 0 && b < fs*0.8f)  //par?
				rd[a] = v * ff;
		}	}
		delete[] mask;
	}

	//  set new hmap
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		fHmap[a] = rd[a];
	}	}

	delete[] rd;  delete[] rh;



	//  clear bullet world
	for (int i=0; i < road->vbtTriMesh.size(); ++i)
		delete road->vbtTriMesh[i];
	road->vbtTriMesh.clear();
	
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		if (obj->getUserPointer() == (void*)111)  // only road
		{
			delete obj->getCollisionShape();  //?
			
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
				delete body->getMotionState();

			world->removeCollisionObject(obj);
			delete obj;
		}
	}


	//  update terrain  todo: rect only
	terrain->dirty();  //dirtyRect(Rect());
	//GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
	//initBlendMaps(terrain);
	bTerUpd = true;


	//  put sel segs on terrain
	for (std::set<int>::const_iterator it = road->vSel.begin(); it != road->vSel.end(); ++it)
		road->mP[*it].onTer = true;

	//  restore orig road width
	road->RebuildRoad(true);
	
	// todo: ?restore road sel after load F5..


	ti.update();	///T  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Ter Align: ") + fToStr(dt,0,3) + " ms");
}
