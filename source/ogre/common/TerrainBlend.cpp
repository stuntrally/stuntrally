#include "pch.h"
#include "../common/RenderConst.h"
#include "../common/Def_Str.h"
#include "../common/data/SceneXml.h"
#include "../common/CScene.h"
#include "../../vdrift/pathmanager.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
#else
	#include "../CGame.h"
#endif
#include <OgreRoot.h>
#include <OgreTimer.h>
#include <OgreTerrain.h>
#include <OgreCamera.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRectangle2D.h>
#include <OgreViewport.h>
#include <OgreSceneNode.h>
#include <OgreTextureManager.h>
#include <OgreRenderTexture.h>
#include "../../shiny/Main/Factory.hpp"
#include "../../paged-geom/GrassLoader.h"
using namespace Ogre;


//  common rtt setup
void CScene::RenderToTex::Setup(Root* rt, String sName, TexturePtr pTex, String sMtr)
{
	if (!scm)  scm = rt->createSceneManager(ST_GENERIC);  // once-
	//  destroy old
	if (cam)  scm->destroyCamera(cam);
	if (nd)  scm->destroySceneNode(nd);
	delete rect;
	
	cam = scm->createCamera(sName+"C");
	cam->setPosition(Vector3(0,10,0));  cam->setOrientation(Quaternion(0.5,-0.5,0.5,0.5));
	cam->setNearClipDistance(0.5f);     cam->setFarClipDistance(500.f);
	cam->setAspectRatio(1.f);   cam->setProjectionType(PT_ORTHOGRAPHIC);
	cam->setOrthoWindow(1.f,1.f);

	rnd = pTex->getBuffer()->getRenderTarget();
	rnd->setAutoUpdated(false);  //rnd->addListener(this);
	vp = rnd->addViewport(cam);
	vp->setClearEveryFrame(true);   vp->setBackgroundColour(ColourValue(0,0,0,0));
	vp->setOverlaysEnabled(false);  vp->setSkiesEnabled(false);
	vp->setShadowsEnabled(false);   //vp->setVisibilityMask();
	vp->setMaterialScheme("reflection");

	rect = new Rectangle2D(true);   rect->setCorners(-1,1,1,-1);
	AxisAlignedBox aab;  aab.setInfinite();
	rect->setBoundingBox(aab);  rect->setCastShadows(false);
	rect->setMaterial( sMtr );

	nd = scm->getRootSceneNode()->createChildSceneNode(sName+"N");
	nd->attachObject(rect);
}

///  blendmap setup
//  every time terrain hmap size changes
//----------------------------------------------------------------------------------------------------
const String CScene::sHmap = "HmapTex",
	CScene::sAng = "AnglesRTT", CScene::sBlend = "blendmapRTT",
	CScene::sAngMat = "anglesMat", CScene::sBlendMat = "blendMat",
	CScene::sGrassDens = "GrassDensRTT", CScene::sGrassDensMat = "grassDensMat";

void CScene::CreateBlendTex()
{
	uint size = sc->td.iTerSize-1;
	TextureManager& texMgr = TextureManager::getSingleton();
	texMgr.remove(sHmap);  texMgr.remove(sAng);
	texMgr.remove(sBlend);  texMgr.remove(sGrassDens);

	//  Hmap tex
	heightTex = texMgr.createManual( sHmap, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_FLOAT32_R, TU_DYNAMIC_WRITE_ONLY); //TU_STATIC_WRITE_ONLY?
	if (heightTex.isNull())
		LogO("Error: Can't create Float32 (HMap) texture!");
	
	//  Angles rtt
	angleRTex = texMgr.createManual( sAng, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_FLOAT32_R, TU_RENDERTARGET);
	if (angleRTex.isNull())
		LogO("Error: Can't create Float32 (Angles) RenderTarget!");
	
	//  Blendmap rtt
	blendRTex = texMgr.createManual( sBlend, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_R8G8B8A8, TU_RENDERTARGET);
	if (blendRTex.isNull())
		LogO("Error: Can't create RGBA (Blendmap) RenderTarget!");

	//  rtt copy  (not needed)
	//blMap = texMgr.createManual("blendmapT", rgDef, TEX_TYPE_2D,
	//	size, size, 0, PF_R8G8B8A8, TU_DEFAULT);
	
	//  Grass Density rtt
	grassDensRTex = texMgr.createManual( sGrassDens, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_R8G8B8A8, TU_RENDERTARGET);
	if (grassDensRTex.isNull())
		LogO("Error: Can't create RGBA (GrassDens) RenderTarget!");
	
	blendRTT.Setup(app->mRoot, "bl", blendRTex, sBlendMat);
	angleRTT.Setup(app->mRoot, "ang", angleRTex, sAngMat);
	grassDensRTT.Setup(app->mRoot, "grd", grassDensRTex, sGrassDensMat);
	
	//UpdBlendmap();  //
}


///  update, fill hmap texture from cpu floats
//  every terrain hmap edit
//--------------------------------------------------------------------------
void CScene::UpdBlendmap()
{
	//if (!terrain)  return;
	Ogre::Timer ti;

	size_t size = sc->td.iTerSize-1;  //!^ same as in create
	float* fHmap = terrain ? terrain->getHeightData() : sc->td.hfHeight;
	if (!fHmap)  return;

	//  fill hmap  (copy to tex, full is fast)
	HardwarePixelBufferSharedPtr pt = heightTex->getBuffer();
	pt->lock(HardwareBuffer::HBL_DISCARD);

	const PixelBox& pb = pt->getCurrentLock();
	float* pD = static_cast<float*>(pb.data);
	size_t aD = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
	 
	register size_t j,i,a=0;
	for (j = 0; j < size; ++j)
	{
		for (i = 0; i < size; ++i)
		{	
			*pD++ = fHmap[a++];
		}
		pD += aD;  a++;  //Hmap is size+1
	}
	pt->unlock();

	//  rtt
	if (angleRTT.rnd && blendRTT.rnd)
	{
		UpdLayerPars();
		
		angleRTT.rnd->update();
		blendRTT.rnd->update();

		//  copy from rtt to normal texture
		//HardwarePixelBufferSharedPtr b = blendRTT.rnd->getBuffer();
		//b->blit(pt);

		//  test  save
		//Image im;
		//heightTex->convertToImage(im);
		//im.save(PATHMANAGER::DataUser()+ "/../hmap.png");

		//angleRTT.rnd->writeContentsToFile(PATHMANAGER::DataUser()+ "/../angle.png");
		//blendRTT.rnd->writeContentsToFile(PATHMANAGER::DataUser()+ "/../blend.jpg");
	}
	else
		LogO("Error: Didn't update blendmap !");

	//LogO(String("::: Time Upd blendmap: ") + fToStr(ti.getMilliseconds(),0,1) + " ms");  // 1ms on 512, 4ms on 1k
}


///  for game, wheel contacts
//--------------------------------------------------------------------------
#ifndef SR_EDITOR
void App::GetTerMtrIds()
{
	//Ogre::Timer ti;

	size_t size = scn->sc->td.iTerSize-1;  //!^ same as in create
	size_t size2 = size*size;
	//  new
	delete[] blendMtr;
	blendMtr = new char[size2];
	memset(blendMtr,0,size2);  // zero

	if (!scn->terrain)  return;
	blendMapSize = size;
	uint8* pd = new uint8[size2*4];  // temp

	HardwarePixelBufferSharedPtr pt = scn->blendRTex->getBuffer();
	PixelBox pb(pt->getWidth(), pt->getHeight(), pt->getDepth(), pt->getFormat(), pd);
	assert(pt->getWidth() == size && pt->getHeight() == size);

	pt->blitToMemory(pb);
	//RenderTexture* pTex = pt->getRenderTarget();
	//pTex->copyContentsToMemory(pb, RenderTarget::FB_AUTO);

	register size_t aa = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
	register uint8* p = pd, v, h;
	register size_t j,i,a=0;
	register char mtr;
	for (j = 0; j < size; ++j)
	{
		for (i = 0; i < size; ++i)
		{
			mtr = 0;  h = 0;  // layers B,G,R,A
			v = *p++;  if (v > h) {  h = v;  mtr = 2;  }
			v = *p++;  if (v > h) {  h = v;  mtr = 1;  }
			v = *p++;  if (v > h) {  h = v;  mtr = 0;  }
			v = *p++;  if (v > h) {  h = v;  mtr = 3;  }
			blendMtr[a++] = mtr;
		}
		pd += aa;
	}
	delete[] pd;

	//LogO(String("::: Time Ter Ids: ") + fToStr(ti.getMilliseconds(),3,5) + " ms");  // 10ms on 1k
}
#endif


///  update blendmap layer params in shader
//--------------------------------------------------------------------------
void CScene::UpdLayerPars()
{
	//  angles
	sh::MaterialInstance* mat = sh::Factory::getInstance().getMaterialInstance(sAngMat);
	mat->setProperty("InvTerSize", sh::makeProperty<sh::FloatValue>(new sh::FloatValue( 1.f / float(sc->td.iTerSize) )));
	mat->setProperty("TriSize",    sh::makeProperty<sh::FloatValue>(new sh::FloatValue( 2.f * sc->td.fTriangleSize )));

	//  blendmap
	mat = sh::Factory::getInstance().getMaterialInstance(sBlendMat);
	int i;
	float Hmin[4],Hmax[4],Hsmt[4], Amin[4],Amax[4],Asmt[4];
	float Nnext[4],Nprev[3],Nnext2[2], Nonly[4];
	float Nfreq[3],Noct[3],Npers[3],Npow[3];
	float Nfreq2[2],Noct2[2],Npers2[2],Npow2[2];
	//  zero
	for (i=0; i < 4; ++i)
	{	Hmin[i]=0.f; Hmax[i]=0.f; Hsmt[i]=0.f;  Amin[i]=0.f; Amax[i]=0.f; Asmt[i]=0.f;
		Nnext[i]=0.f;  Nonly[i]=0.f;  }
	for (i=0; i < 3; ++i)
	{	Nprev[i]=0.f;  Nfreq[i]=0.f; Noct[i]=0.f; Npers[i]=0.f; Npow[i]=0.f;  }
	for (i=0; i < 2; ++i)
	{	Nnext2[i]=0.f;  Nfreq2[i]=0.f; Noct2[i]=0.f; Npers2[i]=0.f; Npow2[i]=0.f;  }
	
	int nl = std::min(4, (int)sc->td.layers.size());
	for (i=0; i < nl; ++i)
	{	//  range
		const TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		Hmin[i] = l.hMin;	Hmax[i] = l.hMax;	Hsmt[i] = l.hSm;
		Amin[i] = l.angMin;	Amax[i] = l.angMax;	Asmt[i] = l.angSm;
		//  noise
		Nonly[i] = !l.nOnly ? 1.f : 0.f;
		Nnext[i] = i < nl-1 ? l.noise : 0.f;  // dont +1 last
		if (i > 0)  Nprev[i-1] = l.nprev;  // dont -1 first
		if (i < 2)  Nnext2[i] = nl > 2 ? l.nnext2 : 0.f;
		//  n par +1,-1, +2
		if (i < nl-1){  Nfreq[i] = l.nFreq[0];  Noct[i] = l.nOct[0];  Npers[i] = l.nPers[0];  Npow[i] = l.nPow[0];  }
		if (i < nl-2){  Nfreq2[i]= l.nFreq[1];  Noct2[i]= l.nOct[1];  Npers2[i]= l.nPers[1];  Npow2[i]= l.nPow[1];  }
	}
	#define Set4(s,v)  mat->setProperty(s, sh::makeProperty<sh::Vector4>(new sh::Vector4(v[0], v[1], v[2], v[3])))
	#define Set3(s,v)  mat->setProperty(s, sh::makeProperty<sh::Vector3>(new sh::Vector3(v[0], v[1], v[2])))
	#define Set2(s,v)  mat->setProperty(s, sh::makeProperty<sh::Vector2>(new sh::Vector2(v[0], v[1])))
	#define Set1(s,v)  mat->setProperty(s, sh::makeProperty<sh::FloatValue>(new sh::FloatValue(v)))
	Set4("Hmin", Hmin);  Set4("Hmax", Hmax);  Set4("Hsmt", Hsmt);
	Set4("Amin", Amin);  Set4("Amax", Amax);  Set4("Asmt", Asmt);  Set4("Nonly", Nonly);
	Set3("Nnext", Nnext);  Set3("Nprev", Nprev);  Set2("Nnext2", Nnext2);
	Set3("Nfreq", Nfreq);  Set3("Noct", Noct);  Set3("Npers", Npers);  Set3("Npow", Npow);
	Set2("Nfreq2", Nfreq2);  Set2("Noct2", Noct2);  Set2("Npers2", Npers2);  Set2("Npow2", Npow2);
}


///  update grass density channel params in shader
//--------------------------------------------------------------------------
void CScene::UpdGrassDens()
{
	if (!grassDensRTT.rnd)  return;

	Ogre::Timer ti;

	UpdGrassPars();
	
	grassDensRTT.rnd->update();

	//grassDensRTT.rnd->writeContentsToFile(PATHMANAGER::DataUser()+"/grassRD.png");

	LogO(String("::: Time Grass Dens: ") + fToStr(ti.getMilliseconds(),0,1) + " ms");
}

void CScene::UpdGrassPars()
{
	sh::MaterialInstance* mat = sh::Factory::getInstance().getMaterialInstance(sGrassDensMat);

	float Hmin[4],Hmax[4],Hsmt[4], Amin[4],Amax[4],Asmt[4];
	float Nmul[4], Nfreq[4],Noct[4],Npers[4],Npow[4], Rpow[4];
	
	for (int i=0; i < 4; ++i)
	{	//  range
		const SGrassChannel& gr = sc->grChan[i];
		Hmin[i] = gr.hMin;  Hmax[i] = gr.hMax;  Hsmt[i] = gr.hSm;
		Amin[i] = gr.angMin;  Amax[i] = gr.angMax;  Asmt[i] = gr.angSm;
		Rpow[i] = powf(2.f, gr.rdPow);
		//  noise
		Nmul[i] = gr.noise;  Nfreq[i] = gr.nFreq;
		Noct[i] = gr.nOct;  Npers[i] = gr.nPers;  Npow[i] = gr.nPow;
	}
	Set4("Hmin", Hmin);  Set4("Hmax", Hmax);  Set4("Hsmt", Hsmt);
	Set4("Amin", Amin);  Set4("Amax", Amax);  Set4("Asmt", Asmt);
	Set4("Nmul", Nmul);  Set4("Nfreq", Nfreq);  Set4("Rpow", Rpow);
	Set4("Noct", Noct);  Set4("Npers", Npers);  Set4("Npow", Npow);
}
