#include "pch.h"
#include "../common/RenderConst.h"
#include "../common/QTimer.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
#else
	#include "../CGame.h"
#endif
#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreHardwarePixelBuffer.h>
using namespace Ogre;


//  common rtt setup
void App::Blmap::Setup(String sName, TexturePtr pTex, String sMtr)
{
	if (!scm)  return;  // once-
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
	//vp->setMaterialScheme("reflection");

	rect = new Rectangle2D(true);   rect->setCorners(-1,1,1,-1);
	AxisAlignedBox aab;  aab.setInfinite();
	rect->setBoundingBox(aab);  rect->setCastShadows(false);
	rect->setMaterial( sMtr );

	nd = scm->getRootSceneNode()->createChildSceneNode(sName+"N");
	nd->attachObject(rect);
}

//  blendmap setup
//  every time terrain hmap size changes
//--------------------------------------------------------------------------------------------------------------------------
const static String sHmap = "HmapTex", sAng = "AnglesRTT", sBlend = "blendmapRTT";
void App::CreateBlendTex()
{
	uint size = sc->td.iTerSize-1;
	TextureManager& texMgr = TextureManager::getSingleton();
	texMgr.remove(sHmap);
	texMgr.remove(sAng);
	texMgr.remove(sBlend);

	//  Hmap tex
	hMap = texMgr.createManual(
		sHmap, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_FLOAT32_R, TU_DYNAMIC_WRITE_ONLY); //TU_STATIC_WRITE_ONLY?
	
	//  Angles rtt
	angRT = texMgr.createManual(
		sAng, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_FLOAT32_R, TU_RENDERTARGET);
	
	//  blendmap rtt
	blRT = texMgr.createManual(
		sBlend, rgDef, TEX_TYPE_2D,
		size, size, 0, PF_R8G8B8A8, TU_RENDERTARGET);

	//  rtt copy  (not needed?)
	//blMap = texMgr.createManual(
	//	"blendmapT", rgDef, TEX_TYPE_2D,
	//	size, size, 0, PF_R8G8B8A8, TU_DEFAULT);
	
	if (!bl.scm)
		bl.scm = mRoot->createSceneManager(ST_GENERIC);
		//ang.scm = mRoot->createSceneManager(ST_GENERIC);
	bl.Setup("bl", blRT, "blendMat");
	//ang.Setup("ang", angMapRT, "anglesMat");
}

//  update
void App::FillHmapTex()
{
	QTimer ti;  ti.update();  /// time

	uint size = sc->td.iTerSize-1; //!^
	float* fHmap = terrain ? terrain->getHeightData() : sc->td.hfHeight;

	//  fill hmap
	HardwarePixelBufferSharedPtr pt = hMap->getBuffer();
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

	if (bl.rnd)
	{	bl.rnd->update();
		//  copy from rtt to normal texture
		//HardwarePixelBufferSharedPtr b = blMap->getBuffer();
		//b->blit(pt);
		//bl.rnd->writeContentsToFile(/*PATHMANAGER::DataUser()+*/ "blend.png");
	}

	ti.update();  /// time (1ms on 512, 4ms on 1k)
	float dt = ti.dt * 1000.f;
	//LogO(String("::: Time fill Hmap: ") + fToStr(dt,3,5) + " ms");
}


//  fill Blend maps  (old)
//--------------------------------------------------------------------------------------------------------------------------
void App::initBlendMaps(Terrain* terrain, int xb,int yb, int xe,int ye, bool full)
{
	QTimer ti;  ti.update();  /// time
	//for (float f=-1.f; f<=2.f; f+=0.02f)  // test
	//	LogO(toStr(f) + " = " + toStr( linRange(f,-0.5f,1.5f,0.2f) ));

	int b = sc->td.layers.size()-1, i;
	float* pB[6];	TerrainLayerBlendMap* bMap[6];
	int t = terrain->getLayerBlendMapSize(), x,y;
	//LogO(String("Ter blendmap size: ")+toStr(t));
	const float f = 0.8f / t * 2 * sc->td.fTerWorldSize / t * 3.14f;  //par repeat-

	//  mtr map
	#ifndef SR_EDITOR
	delete[] blendMtr;  blendMtr = 0;
	blendMtr = new char[t*t];
	#endif

	for (i=0; i < b; ++i)
	{	bMap[i] = terrain->getLayerBlendMap(i+1);  pB[i] = bMap[i]->getBlendPointer();  }
	float *fHmap = terrain->getHeightData();

	Math math;
	#define sin_(a)  math.Sin(a,true)
	#define cos_(a)  math.Cos(a,true)
	#define m01(v)  std::max(0.f, std::min(1.f, v ))
	
	//  default layer params
	Real val[5], aMin[5],aMax[5],aSm[5], hMin[5],hMax[5],hSm[5], noise[5];  bool bNOnly[5];
	for (i=0; i < 5; ++i)
	{	val[i]=0.f;  aMin[i]=0.f; aMax[i]=90.f;  aSm[i]=5.f;  hSm[i]=20.f;  bNOnly[i]=1;
		hMin[i]=-300.f; hMax[i]=300.f;  noise[i]=1.f;  }
	
	//  params from layers
	for (i=0; i < std::min(5, (int)sc->td.layers.size()); ++i)
	{
		const TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		aMin[i] = l.angMin;	aMax[i] = l.angMax;
		hMin[i] = l.hMin;	hMax[i] = l.hMax;  noise[i] = l.noise;
		aSm[i] = l.angSm;	hSm[i] = l.hSm;    bNOnly[i] = l.bNoiseOnly;
	}
	
	//  fill blendmap  ---------------
	int w = sc->td.iTerSize;
	float ft = t, fw = w;
	int xB, yB, xE, yE;
	if (full)
	{	xB = 0;  yB = 0;
		xE = t;  yE = t;
	}else{
		xB = xb / fw * ft;  yB = yb / fw * ft;
		xE = xe / fw * ft;  yE = ye / fw * ft;
	}
	//texNoise->getData()
	for (y = yB; y < yE; ++y)  {  int aa = y*t + xB, bb = (t-1-y)*t + xB;
	for (x = xB; x < xE; ++x,++aa,++bb)
	{
		float fx = f*x, fy = f*y;	//  val,val1:  0 0 - [0]   1 0  - [1]   0 1 - [2]
		//const Real p = (b >= 4) ? 3.f : ( (b >= 3) ? 2.f : 1.f ), q = 1.f;
		int x1 = ( x*1     )%1024, y1 = ( y*1     )%1024;
		int x2 = (-x*1+1524)%1024, y2 = ( y*1+600 )%1024;
		int x3 = ( x*1+150 )%1024, y3 = ( y*1+1124)%1024;
		int x4 = ( x*1+300 )%1024, y4 = (-y*1+1224)%1024;
		if (b >= 1)  val[0] = 0.f*(texNoise[3].getColourAt(x1,y1,0).r )*noise[0];
		if (b >= 2)  val[1] = 0.f*(texNoise[0].getColourAt(x2,y2,0).r )*noise[1];
		if (b >= 3)  val[2] = 0.f*(texNoise[1].getColourAt(x3,y3,0).r )*noise[2];
		if (b >= 4)  val[3] = 0.f*(texNoise[2].getColourAt(x4,y4,0).r )*noise[3];

		//  ter angle and height ranges
		#if 1
		int tx = (float)(x)/ft * w, ty = (float)(y)/ft * w, tt = ty * w + tx;
		float a = sc->td.hfAngle[tt], h = fHmap[tt];  // sc->td.hfHeight[tt];
		for (i=0; i < b; ++i)  if (!bNOnly[i]) {  const int i1 = i;//+1;
			val[i] += /*m01*/( /*val[i1]*noise[i] + */linRange(a,aMin[i1],aMax[i1],aSm[i1]) * linRange(h,hMin[i1],hMax[i1],hSm[i1]) );  }
		#endif
		float norm = 0.f;
		for (i=0; i < b; ++i)
			norm += val[i];
		//norm /= b;

		char mtr = 1;
		for (i=0; i < b; ++i)
		{	*(pB[i]+bb) = val[i] / norm;  if (val[i]*norm > 0.5f)  mtr = i+2;  }

		#ifndef SR_EDITOR
		blendMtr[aa] = mtr;
		#endif
	}	}
	
	for (i=0; i < b; ++i)
	{
		if (full)	bMap[i]->dirty();
		else		bMap[i]->dirtyRect(Rect(xB,t-yE,xE,t-yB));  //t-1? max(0,
		//bMap[i]->blit
		bMap[i]->update();
	}
	
	iBlendMaps = b+1;  blendMapSize = t;

	//LogO("blTex: "+terrain->getBlendTextureName(0));  //TerrBlend1
	//bMap[0]->loadImage("blendmap.png", rgDef);
	//bMap[0]->dirty();  bMap[0]->update();
	/*Image bl0;  // ?-
	terrain->getLayerBlendTexture(0)->convertToImage(bl0);
	bl0.save("blendmap.png");/**/
	//terrain->getCompositeMapMaterial
	
	/*// set up a colour map
	if (!terrain->getGlobalColourMapEnabled())
	{
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("testcolourmap.jpg", rgDef);
		terrain->getGlobalColourMap()->loadImage(colourMap);
	}*/
	
	#ifndef SR_EDITOR  // game
	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Blendmap: ") + fToStr(dt,0,3) + " ms");
	#endif
}


///  Hmap angles  .....
void App::GetTerAngles(int xb,int yb, int xe,int ye, bool full)
{
	FillHmapTex();
	return;

//  old  ----
	int wx = sc->td.iVertsX, wy = sc->td.iVertsY;
	int xB,xE, yB,yE;
	if (full)
	{	xB = 1;  xE = wx-1;
		yB = 1;  yE = wy-1;
	}else
	{	xB = std::max(1,xb);  xE = std::min(wx-1,xe);
		yB = std::max(1,yb);  yE = std::min(wy-1,ye);
	}
	float* hf = terrain ? terrain->getHeightData() : sc->td.hfHeight;

	Real t = sc->td.fTriangleSize * 2.f;
	for (int j = yB; j < yE; ++j)  // 1 from borders
	{
		int a = j * wx + xB;
		for (int i = xB; i < xE; ++i,++a)
		{
			Vector3 vx(t, hf[a+1] - hf[a-1], 0);  // x+1 - x-1
			Vector3 vz(0, hf[a+wx] - hf[a-wx], t);	// z+1 - z-1
			Vector3 norm = -vx.crossProduct(vz);  norm.normalise();
			Real ang = Math::ACos(norm.y).valueDegrees();

			sc->td.hfAngle[a] = ang;
		}
	}
	if (!full)  return;
	
	//  only corner[] vals
	//sc->td.hfNorm[0] = 0.f;
	
	//  only border[] vals  todo: like above
	for (int j=0; j < wy; ++j)  // |
	{	int a = j * wx;
		sc->td.hfAngle[a + wx-1] = 0.f;
		sc->td.hfAngle[a] = 0.f;
	}
	int a = (wy-1) * wx;
	for (int i=0; i < wx; ++i,++a)  // --
	{	sc->td.hfAngle[i] = 0.f;
		sc->td.hfAngle[a] = 0.f;
	}
}
