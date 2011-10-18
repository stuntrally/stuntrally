#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
//#include <OgreHardwarePixelBuffer.h>
#include "../road/Road.h"
#include "../ogre/common/RenderConst.h"
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
				  dim[i], dim[i], 0, /*PF_R8G8B8*/PF_R8G8B8A8, TU_RENDERTARGET);
				  
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
	uint *gd  = new uint[w*h];  // grass dens
	PixelBox pb_rd(w,h,1, PF_BYTE_RGBA, rd);
	rt[1].rndTex->copyContentsToMemory(pb_rd, RenderTarget::FB_FRONT);

	//  rotate, filter  smooth size
	const int f = sc.grDensSmooth;
	const float ff = 2.f / ((f*2+1)*(f*2+1)) / 255.f;
	register int v,y,x,i,j,a,b,d;

	for (y = f; y < h-f; ++y) {  a = y*w +f;
	for (x = f; x < w-f; ++x, ++a)
	{	b = x*w + (h-y);  // rot 90 ccw  b=a
		
		v = 0;  // filter f  sum
		for (j = -f; j <= f; ++j) {  d = b -f + j*w;
		for (i = -f; i <= f; ++i, ++d)
			v += (rd[d] >> 16) & 0xFF;  }

		v = std::max(0, (int)(255.f * (1.f - v * ff) ));  // avg, inv, clamp
		
		gd[a] = 0xFF000000 + 0x010101 * v;  // write
	}	}

	v = 0xFFFFFFFF;  //  frame f []
	for (y = 0;  y <= f; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - up
	for (y=h-f-1; y < h; ++y)	for (x=0; x < w; ++x)	gd[y*w+x] = v;  // - down
	for (x = 0;  x <= f; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | left
	for (x=w-f-1; x < w; ++x)	for (y=0; y < h; ++y)	gd[y*w+x] = v;  // | right

	Image im;
	im.loadDynamicImage((uchar*)gd, w,h,1, PF_BYTE_RGBA);
	im.save(TrkDir()+"objects/grassDensity.png");

	delete[] rd;  delete[] gd;

	//  road  ----------------
	rt[0].rndTex->writeContentsToFile(pathTrkPrv[1] + pSet->track + "_mini.png");
	//  terrain
	rt[2].rndTex->writeContentsToFile(pathTrkPrv[1] + pSet->track + "_ter.jpg");
}


///  pre and post  rnd to tex
//-----------------------------------------------------------------------------------------------------------
void App::preRenderTargetUpdate(const RenderTargetEvent &evt)
{
	const String& s = evt.source->getName();
	int num = atoi(s.substr(s.length()-1, s.length()-1).c_str());
	
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
