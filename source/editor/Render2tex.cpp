#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include <OgrePrerequisites.h>
#include <OgreTimer.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include <OgreManualObject.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRectangle2D.h>
#include <OgreCamera.h>
#include <OgreTextureManager.h>
#include <OgreRenderTexture.h>
#include <OgreViewport.h>
#include <OgreMaterialManager.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
using namespace Ogre;


//  Setup render 2 texture (road maps)
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::Rnd2TexSetup()
{
	///  RT:  0 road minimap,  1 road for grass,  2 terrain minimap,  3 track preview full
	const uint32 visMask[RT_Last] =
		{ RV_Road, RV_Road+RV_Objects, RV_Terrain+RV_Objects, RV_MaskAll-RV_Hud };
	const int dim[RT_Last] =  //1025: sc->td.iVertsX
		{ 1024, 1025, 1024, 1024 };
		
	asp = float(mWindow->getWidth())/float(mWindow->getHeight());
	Real sz = pSet->size_minimap;
	xm1 = 1-sz/asp, ym1 = -1+sz, xm2 = 1.0, ym2 = -1.0;
	AxisAlignedBox aab;  aab.setInfinite();
	
	TexturePtr texture[RT_Last];
	for (int i=0; i < RT_ALL; ++i)
	{
		SRndTrg& r = rt[i];  bool full = i==RT_View;
		String si = toStr(i), sMtr = "road_mini_"+si;
		if (i < RT_Last)
		{
			String sTex = "RttTex"+si, sCam = "RttCam"+si;

			if (TextureManager::getSingleton().resourceExists(sTex))
				TextureManager::getSingleton().remove(sTex);
			mSceneMgr->destroyCamera(sCam);  // dont destroy old - const tex sizes opt..
			
			///  rnd to tex - same dim as Hmap	// after track load
			Real fDim = scn->sc->td.fTerWorldSize;  // world dim
			texture[i] = TextureManager::getSingleton().createManual(
				sTex, rgDef, TEX_TYPE_2D, dim[i], dim[i], 0,
				i == RT_View || i == RT_Terrain ? PF_R8G8B8 : PF_R8G8B8A8, TU_RENDERTARGET);
				  
			r.cam = mSceneMgr->createCamera(sCam);  // up
			r.cam->setPosition(Vector3(0,1500,0));  //par- max height
			r.cam->setOrientation(Quaternion(0.5,-0.5,0.5,0.5));
			r.cam->setNearClipDistance(0.5);	r.cam->setFarClipDistance(50000);
			r.cam->setAspectRatio(1.0);			if (!full)  r.cam->setProjectionType(PT_ORTHOGRAPHIC);
			r.cam->setOrthoWindow(fDim,fDim);	//rt[i].rndCam->setPolygonMode(PM_WIREFRAME);

			r.tex = texture[i]->getBuffer()->getRenderTarget();
			r.tex->setAutoUpdated(false);	r.tex->addListener(this);
			Viewport* rvp = r.tex->addViewport(r.cam);
			rvp->setClearEveryFrame(true);   rvp->setBackgroundColour(ColourValue(0,0,0,0));
			rvp->setOverlaysEnabled(false);  rvp->setSkiesEnabled(full);
			rvp->setVisibilityMask(visMask[i]);
			rvp->setShadowsEnabled(false);

			if (i != RT_View)  rvp->setMaterialScheme("reflection");
		}
		///  minimap  . . . . . . . . . . . . . . . . . . . . . . . . . . . 
		if (r.ndMini)  mSceneMgr->destroySceneNode(r.ndMini);
		MaterialPtr mt = MaterialManager::getSingleton().getByName(sMtr);
	#if defined(OGRE_VERSION) && OGRE_VERSION >= 0x10A00
		if (i == RT_View)
		{	mt->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(texture[0]);
			mt->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTexture(texture[2]);
		}else if (i < RT_Last)
			mt->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(texture[i]);
		else if (i == RT_Last)
			mt->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(texture[3]);
	#endif
		if (mt)  mt->reload();

		r.mini = new Rectangle2D(true);  // screen rect preview
		if (i == RT_Last)  r.mini->setCorners(-1/asp, 1, 1/asp, -1);  // fullscr,square
		else  r.mini->setCorners(xm1, ym1, xm2, ym2);  //+i*sz*all

		r.mini->setBoundingBox(aab);
		r.ndMini = mSceneMgr->getRootSceneNode()->createChildSceneNode("Minimap"+si);
		r.ndMini->attachObject(r.mini);	r.mini->setCastShadows(false);
#if defined(OGRE_VERSION) && OGRE_VERSION < 0x10A00
		r.mini->setMaterial(i == RT_Brush ? "BrushPrvMtr" : sMtr);
#else
		MaterialPtr brush_mt = MaterialManager::getSingleton().getByName("BrushPrvMtr");
		r.mini->setMaterial(i == RT_Brush ? brush_mt : mt);
#endif
		r.mini->setRenderQueueGroup(RQG_Hud2);
		r.mini->setVisibilityFlags(i == RT_Last ? RV_MaskPrvCam : RV_Hud);
	}

	//  pos dot on minimap  . . . . . . . .
	if (!ndPos)
	{	mpos = Create2D("hud/CarPos", 0.2f, true);  // dot size
		mpos->setVisibilityFlags(RV_Hud);
		mpos->setRenderQueueGroup(RQG_Hud3 /*RENDER_QUEUE_OVERLAY+1*/);
		ndPos = mSceneMgr->getRootSceneNode()->createChildSceneNode(
			Vector3(xm1+(xm2-xm1)/2, ym1+(ym2-ym1)/2, 0));
		float fHudSize = 0.04f;
		ndPos->scale(fHudSize, fHudSize, 1);
		ndPos->attachObject(mpos);
	}
	if (ndPos)   ndPos->setVisible(pSet->trackmap);
	UpdMiniVis();
}

void App::UpdMiniVis()
{
	for (int i=0; i < RT_ALL; ++i)
		if (rt[i].ndMini)
			rt[i].ndMini->setVisible(pSet->trackmap && (i == pSet->num_mini));
}


///  Image from road
///  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::SaveGrassDens()
{
	Ogre::Timer ti;

	for (int i=0; i < RT_View; ++i)  //-1 preview camera manual
	{
		if (!rt[i].tex)  return;
		rt[i].tex->update();  // all have to exist
	}

	int w = rt[RT_Grass].tex->getWidth(), h = rt[RT_Grass].tex->getHeight();
	using Ogre::uint;
	uint *rd = new uint[w*h];   // road render
	uint *gd = new uint[w*h];   // grass dens
	PixelBox pb_rd(w,h,1, PF_BYTE_RGBA, rd);
	rt[RT_Grass].tex->copyContentsToMemory(pb_rd, RenderTarget::FB_FRONT);

	const int f = std::max(0, scn->sc->grDensSmooth);
	float sum = 0.f;
	int v,i,j,x,y, a,b,d,m;

	//  gauss kernel for smoothing
	int *mask = new int[(f*2+1)*(f*2+1)];  m = 0;
	if (f==0)
	{	mask[0] = 256.f;  sum = 256.f;  }
	else
	for (j = -f; j <= f; ++j)
	for (i = -f; i <= f; ++i, ++m)
	{
		v = std::max(0.f, (1.f - sqrtf((float)(i*i+j*j)) / float(f)) * 256.f);
		mask[m] = v;  sum += v;
	}
	sum = 2.f / sum;  //par normally would be 1.f - but road needs to stay black and be smooth outside
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

		v = std::max(0, (int)(255.f * (1.f - v * sum) ));  // avg, inv, clamp
		
		gd[a] = 0xFF000000 + v;  // write
	}	}

	v = 0xFF0000FF;  //  frame f []  todo: get from rd[b] not clear..
	for (y = 0;  y <= f; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - up
	for (y=h-f-1; y < h; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - down
	for (x = 0;  x <= f; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | left
	for (x=w-f-1; x < w; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | right

	Image im;  // for trees, before grass angle and height
	im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
	im.save(gcom->TrkDir()+"objects/roadDensity.png");

	LogO(String("::: Time road dens: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	delete[] rd;  delete[] gd;  delete[] mask;

	//  road, terrain  ----------------
	int user = pSet->allow_save ? pSet->gui.track_user : 1;
	auto path = gcom->pathTrk[user] + pSet->gui.track;
	rt[RT_Road   ].tex->writeContentsToFile(path + "/preview/road.png");
	rt[RT_Terrain].tex->writeContentsToFile(path + "/preview/terrain.jpg");

	LogO(String("::: Time save prv : ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


///  pre and post  rnd to tex
//-----------------------------------------------------------------------------------------------------------
void App::preRenderTargetUpdate(const RenderTargetEvent &evt)
{
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());
	
	if (num == RT_View)  // full
	{
		rt[RT_View].cam->setPosition(mCamera->getPosition());
		rt[RT_View].cam->setDirection(mCamera->getDirection());
	}
	else for (auto r : scn->roads)
		if (!r->IsRiver())
			r->SetForRnd(num == RT_Road ? "render_clr" : "render_grass");
}

void App::postRenderTargetUpdate(const RenderTargetEvent &evt)
{	
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());

	if (num == RT_View)  // full
	{	}
	else for (auto r : scn->roads)
	{	if (!r->IsRiver())  r->UnsetForRnd();
		r->UpdLodVis(pSet->road_dist);
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
	if (scn->sc->fluids.empty())
	{
		gui->Delete(gcom->TrkDir()+"objects/waterDepth.png");  // no tex if no fluids
		return;
	}
	Ogre::Timer ti;

	//  2048 for bigger terrains ?
	int w = 1024, h = w;  float fh = h-1, fw = w-1;
	using Ogre::uint;
	uint *wd = new uint[w*h];   // water depth
	int x,y,a,i,ia,id;
	float fa,fd;
	
	///  write to img  -----------
	//  get ter height to fluid height difference for below
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		float fx = float(y)/fh, fz = float(x)/fw;
		//  pos on ter  -terSize..terSize
		float w = scn->sc->td.fTerWorldSize;
		float wx = (fx-0.5f) * w, wz = -(fz-0.5f) * w;

		fa = 0.f;  // fluid y pos
		for (i=0; i < scn->sc->fluids.size(); ++i)
		{
			const FluidBox& fb = scn->sc->fluids[i];
			const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
			//  check rect 2d - no rot !  todo: make 2nd type circle..
			if (wx > fb.pos.x - sizex && wx < fb.pos.x + sizex &&
				wz > fb.pos.z - sizez && wz < fb.pos.z + sizez)
			{
				float f = fb.pos.y - scn->terrain->getHeightAtTerrainPosition(fx,fz);
				if (f > fa)  fa = f;
			}
		}		//par
		fd = fa * 0.4f * 255.f;  // depth far  full at 2.5 m
		fa = fa * 8.f * 255.f;  // alpha near  full at 1/8 m

		ia = std::max(0, std::min(255, (int)fa ));  // clamp
		id = std::max(0, std::min(255, (int)fd ));
		
		wd[a] = 0xFF000000 + /*0x01 */ ia + 0x0100 * id;  // write
	}	}

	Image im;  // save img
	im.loadDynamicImage((uchar*)wd, w,h,1, PF_BYTE_RGBA);
	im.save(gcom->TrkDir()+"objects/waterDepth.png");
	delete[] wd;

	try {
	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
	if (tex)
		tex->reload();
	else  // 1st fluid after start, refresh matdef ?..
		TextureManager::getSingleton().load("waterDepth.png", rgDef);
	} catch(...) {  }

	LogO(String("::: Time WaterDepth: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
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
		const btCollisionObject* obj = rayResult.m_collisionObject;
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
	SplineRoad* road = scn->road;
	if (road->vSel.empty())  return;
	Ogre::Timer ti;

	///  create bullet road for selected segments
	road->ed_Wmul = pSet->al_w_mul;
	road->ed_Wadd = pSet->al_w_add;
	road->RebuildRoadInt(true);

	//  terrain
	float *fHmap = scn->terrain->getHeightData();
	const int w = scn->sc->td.iVertsX, h = w;
	const float fh = h-1, fw = w-1;

	float *rd = new float[w*h];  // road depth
	bool  *rh = new bool[w*h];  // road hit

	const float ws = scn->sc->td.fTerWorldSize;
	const float Len = 400;  // max ray length
	int x,y,a;
	float v,k, fx,fz, wx,wz;
	
	///  ray casts  -----------
	for (y = 0; y < h; ++y) {  a = y*w;
	for (x = 0; x < w; ++x, ++a)
	{
		//  pos 0..1
		fx = float(x)/fh;  fz = float(y)/fw;
		//  pos on ter  -terSize..terSize
		wx = (fx-0.5f) * ws;  wz = (fz-0.5f) * ws;

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
		int i,j,m,d,b;

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
	}	}


	//  update terrain
	scn->terrain->dirty();  //rect..
	scn->UpdBlendmap();
	bTerUpd = true;


	//  put sel segs on terrain
	for (auto i : scn->road->vSel)
		scn->road->mP[i].onTer = true;

	//  restore orig road width
	scn->road->Rebuild(true);
	
	// todo: ?restore road sel after load F5..

	LogO(String("::: Time Ter Align: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
