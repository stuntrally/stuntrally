#include "pch.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include <OgreTerrain.h>
#include <OgreHardwarePixelBuffer.h>
//#include "../settings.h"
#include <MyGUI.h>
#include <OgreTimer.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
using namespace Ogre;


//  color factors for edit mode D,S,E,F
const float App::brClr[4][3] = {
	{0.3, 0.8, 0.1}, {0.2, 0.8, 0.6}, {0.6, 0.9, 0.6}, {0.4, 0.7, 1.0} };


void CGui::btnBrushPreset(WP img)
{
	int id = 0;
	sscanf(img->getName().c_str(), "brI%d", &id);
	app->SetBrushPreset(id);
}
void App::SetBrushPreset(int id)
{
	const BrushSet& st = brSets[id];  // copy params
	if (!shift)  SetEdMode(st.edMode);  curBr = st.curBr;
	mBrSize[curBr] = st.Size;  mBrIntens[curBr] = st.Intens;  mBrShape[curBr] = st.shape;
	mBrPow[curBr] = st.Pow;  mBrFq[curBr] = st.Fq;  mBrNOf[curBr] = st.NOf;  mBrOct[curBr] = st.Oct;
	if (st.Filter > 0.f)  mBrFilt = st.Filter;
	if (st.HSet != -0.01f)  terSetH = st.HSet;

	updBrush();  UpdEditWnds();
}


static float GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (2*PI_d - atan2f(y, x));
}


///  update brush preview texture
//--------------------------------------------------------------------------------------------------------------------------
void App::updateBrushPrv(bool first)
{
	if (!first && (!ovBrushPrv || edMode >= ED_Road /*|| bMoveCam/*|| !bEdit()*/))  return;
	if (!pSet->brush_prv || !brushPrvTex)  return;

	//  Lock texture and fill pixel data
	HardwarePixelBufferSharedPtr pbuf = brushPrvTex->getBuffer();
	pbuf->lock(HardwareBuffer::HBL_DISCARD);
	const PixelBox& pb = pbuf->getCurrentLock();
	uint8* p = static_cast<uint8*>(pb.data);

	const float fB = brClr[edMode][0]*255.f, fG = brClr[edMode][1]*255.f, fR = brClr[edMode][2]*255.f;

	const float s = BrPrvSize * 0.5f, s1 = 1.f/s,
		fP = mBrPow[curBr], fQ = mBrFq[curBr]*5.f, nof = mBrNOf[curBr];
	int oct = mBrOct[curBr];	const float PiN = PI_d/oct;

	switch (mBrShape[curBr])
	{
	case BRS_Noise2:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * (1.0-pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, c);
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Noise:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*0.5f) * 0.9f;
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Sinus:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( sinf(d * PI_d*0.5f), fP);
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Ngon:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			float k = GetAngle(fx,fy);  // 0..2Pi

    		float c = std::max(0.f, std::min(1.f,
    			fQ * powf( fabs(d / (-1.f+nof + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Triangle:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( fabs(d), fP);
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;
	}
	pbuf->unlock();
}

///  fill brush data (shape), after size change
//--------------------------------------------------------------------------------------------------------------------------
void App::updBrush()
{
	if (mBrSize[curBr] < 2)  mBrSize[curBr] = 2;
	if (mBrSize[curBr] > BrushMaxSize)  mBrSize[curBr] = BrushMaxSize;
	if (mBrFq[curBr] < 0.1)  mBrFq[curBr] = 0.1;
	if (mBrPow[curBr] < 0.02)  mBrPow[curBr] = 0.02;

	int size = (int)mBrSize[curBr], a = 0;
	float s = size * 0.5f, s1 = 1.f/s,
		fP = mBrPow[curBr], fQ = mBrFq[curBr]*5.f, nof = mBrNOf[curBr];
	int oct = mBrOct[curBr];	const float PiN = PI_d/oct;

	switch (mBrShape[curBr])
	{
	case BRS_Noise2:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * (1.0-pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, c);
			
			mBrushData[a] = std::min(1.f, c );
		}	}	break;


	case BRS_Noise:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(CScene::Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*0.5f);

			mBrushData[a] = std::max(-1.f, std::min(1.f, c ));
		}	}	break;

	case BRS_Sinus:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			
			float c = powf( sinf(d * PI_d*0.5f), fP);
			mBrushData[a] = c;
		}	}	break;

	case BRS_Ngon:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			float k = GetAngle(fx,fy);  // 0..2Pi

			float c = std::max(0.f, std::min(1.f,
				fQ * powf( fabs(d / (-1.f+nof + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			mBrushData[a] = c;
		}	}	break;

	case BRS_Triangle:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			
			float c = powf( fabs(d), fP);
			mBrushData[a] = c;
		}	}	break;
	}
	
	//  filter brush kernel  ------
	if (mBrFilt != mBrFiltOld)
	{	mBrFilt = std::max(0.f, std::min(8.f, mBrFilt));
		mBrFiltOld = mBrFilt;
	
		delete[] pBrFmask;  pBrFmask = 0;
		const float fl = mBrFilt;
		const int f = ceil(fl);  float fd = 1.f + fl - floor(fl);
		int m,i,j;

		//  gauss kernel for smoothing
		const int mm = (f*2+1)*(f*2+1);
		pBrFmask = new float[mm];  m = 0;

		float fm = 0.f;  //sum
		for (j = -f; j <= f; ++j)
		{
			float fj = float(j)/f;
			for (i = -f; i <= f; ++i, ++m)
			{
				float fi = float(i)/f;
				float u = std::max(0.f, fd - sqrtf(fi*fi+fj*fj) );
				pBrFmask[m] = u;  fm += u;
			}
		}
		fm = 1.f / fm;  //avg
		for (m = 0; m < mm; ++m)
			pBrFmask[m] *= fm;
	}
	
	updateBrushPrv();  // upd skip..
}


///  Terrain  generate
///--------------------------------------------------------------------------------------------------------------------------
void CGui::btnTerGenerate(WP wp)
{
	const std::string& n = wp->getName();
	bool add = false, sub = false;
	if (n == "TerrainGenAdd")  add = true;  else
	if (n == "TerrainGenSub")  sub = true;/*else
	if (n == "TerrainGenMul")  mul = true;*/

	float* hfData = sc->td.hfHeight; //, *hfAng = sc->td.hfAngle;
	const int sx = sc->td.iVertsX;  // sx=sy
	const float s = sx * 0.5f, s1 = 1.f/s;
	const float ox = pSet->gen_ofsx, oy = pSet->gen_ofsy;

	//)  road test
	bool bRoad = pSet->gen_roadsm > 0.1f;
	float rdPow = pSet->gen_roadsm;  //-
	int r = 0;
	Image imgRoad;
	if (bRoad)
	{
		try {	imgRoad.load(String("roadDensity.png"),"General");  }
		catch(...) {	}
		r = imgRoad.getWidth();
	}

	Ogre::Timer ti;

	//  generate noise terrain hmap
	int a,x,y;  float c;

	for (y=0; y < sx; ++y)  {  a = y * sx;
	for (x=0; x < sx; ++x,++a)
	{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1

		c = CScene::Noise(y*s1-oy, x*s1+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;
		c = c >= 0.f ? powf(c, pSet->gen_pow) : -powf(-c, pSet->gen_pow);

		//)  check if on road - uses roadDensity.png
		//  todo: smooth depends on -smooth grass dens par, own val?
		if (bRoad)
		{
			int mx = ( fx+1.f)*0.5f*r, my = (-fy+1.f)*0.5f*r;
					
			float cr = imgRoad.getColourAt(
				std::max(0,std::min(r-1, mx)), std::max(0,std::min(r-1, my)), 0).r;

			//c = c + std::max(0.f, std::min(1.f, 2*c-cr)) * pow(cr, rdPow);
			c *= pow(cr, rdPow);
		}
		//FIXME: ter gen ang pars
		//c *= app->linRange(hfAng[a],  pSet->gen_terMinA,pSet->gen_terMaxA, pSet->gen_terSmA);
		c *= CScene::linRange(hfData[a], pSet->gen_terMinH,pSet->gen_terMaxH, pSet->gen_terSmH);

		hfData[a] = add ? (hfData[a] + c * pSet->gen_scale + pSet->gen_ofsh) : (
					sub ? (hfData[a] - c * pSet->gen_scale - pSet->gen_ofsh) :
						  (hfData[a] * c * pSet->gen_mul) );
	}	}

	LogO(String("::: Time Ter Gen: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	std::ofstream of;
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	int siz = sx * sx * sizeof(float);
	of.write((const char*)&hfData[0], siz);
	of.close();

	LogO(String("::: Time Ter Gen save: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	app->bNewHmap = true;	app->UpdateTrack();
}

///  update terrain generator preview texture
//--------------------------------------------------------------------------------------------------------------------------
void App::updateTerPrv(bool first)
{
	if (!first && !ovTerPrv)  return;
	if (!terPrvTex)  return;

	HardwarePixelBufferSharedPtr pbuf = terPrvTex->getBuffer();
	pbuf->lock(HardwareBuffer::HBL_DISCARD);
	const PixelBox& pb = pbuf->getCurrentLock();  using Ogre::uint8;
	uint8* p = static_cast<uint8*>(pb.data);

	const static float fB[2] = { 90.f, 90.f}, fG[2] = {255.f,160.f}, fR[2] = { 90.f,255.f};

	const float s = TerPrvSize * 0.5f, s1 = 1.f/s;
	const float ox = pSet->gen_ofsx, oy = pSet->gen_ofsy;

	for (int y = 0; y < TerPrvSize; ++y)
	for (int x = 0; x < TerPrvSize; ++x)
	{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1

		float c = CScene::Noise(x*s1-oy, y*s1+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;  // par fit
		bool b = c >= 0.f;
		c = b ? powf(c, pSet->gen_pow) : -powf(-c, pSet->gen_pow);

		int i = b ? 0 : 1;  c = b ? c : -c;
		//c *= pSet->gen_scale;  //no
		
		uint8 bR = c * fR[i], bG = c * fG[i], bB = c * fB[i];
		*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = 255;//bG > 32 ? 255 : 0;
	}
	pbuf->unlock();
}



///  get edit rect
//-----------------------------------------------------------------------------------------------
bool App::getEditRect(Vector3& pos, Rect& rcBrush, Rect& rcMap, int size,  int& cx, int& cy)
{
	float tws = scn->sc->td.fTerWorldSize;
	int t = scn->sc->td.iTerSize;

	//  world float to map int
	int mapX = (pos.x + 0.5*tws)/tws*t, mapY = (-pos.z + 0.5*tws)/tws*t;
	mapX = std::max(0,std::min(t-1, mapX)), mapY = std::max(0,std::min(t-1, mapY));

	int brS = (int)mBrSize[curBr];
	float hBr = brS * 0.5f;
	rcMap = Rect(mapX-hBr, mapY-hBr, mapX+hBr, mapY+hBr);
	rcBrush = Rect(0,0, brS,brS);
	cx = 0;  cy = 0;

	if (rcMap.left < 0)  // trim
	{	rcBrush.left += 0 - rcMap.left;  cx += -rcMap.left;
		rcMap.left = 0;
	}
	if (rcMap.top < 0)
	{	rcBrush.top  += 0 - rcMap.top;   cy += -rcMap.top;
		rcMap.top = 0;
	}
	if (rcMap.right > size)
	{	rcBrush.right  -= rcMap.right - size;
		rcMap.right = size;
	}
	if (rcMap.bottom > size)
	{	rcBrush.bottom -= rcMap.bottom - size;
		rcMap.bottom = size;
	}

	if (rcMap.right - rcMap.left < 1 ||
		rcMap.bottom - rcMap.top < 1)
		return false;  // no area
	
	/*sprintf(sBrushTest,
		" ---br rect--- \n"
		"size: %3d %6.3f \n"
		"pos %3d %3d  c: %3d %3d \n"
		"rect:  %3d %3d  %3d %3d \n"
		"map: %3d %3d  %3d %3d \n"
		"br: %3d %3d  %3d %3d \n"
		,brS, mBrSize[curBr]
		,mapX, mapY, cx, cy
		 ,rcMap.right-rcMap.left, rcMap.bottom-rcMap.top
		 ,rcBrush.right-rcBrush.left, rcBrush.bottom-rcBrush.top
		,rcMap.left, rcMap.top, rcMap.right, rcMap.bottom
		,rcBrush.left, rcBrush.top, rcBrush.right, rcBrush.bottom);
	LogO(String(sBrushTest));/**/
	return true;
}


///  ^v Deform
//-----------------------------------------------------------------------------------------------
void App::deform(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, scn->sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = scn->terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * scn->sc->td.iTerSize + rcMap.left;
		brPos = jj * BrushMaxSize + cx;
		//brPos = std::max(0, std::min(BrushMaxSize*BrushMaxSize-1, brPos ));

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			///  pos float -> brush data compute here (for small size brushes) ..
			fHmap[mapPos] += mBrushData[brPos] * its;  // deform
			++mapPos;  ++brPos;
		}
	}
	scn->terrain->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}


///  -_ set Height
//-----------------------------------------------------------------------------------------------
void App::height(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, scn->sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = scn->terrain->getHeightData();
		
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * scn->sc->td.iTerSize + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			float d = terSetH - fHmap[mapPos];
			d = d > 2.f ? 2.f : d < -2.f ? -2.f : d;  // par speed-
			fHmap[mapPos] += d * mBrushData[brPos] * its;
			++mapPos;  ++brPos;
		}
	}
	scn->terrain->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}


///  ~- Smooth (Flatten)
//-----------------------------------------------------------------------------------------------
void App::smooth(Vector3 &pos, float dtime)
{
	float avg = 0.0f;
	int sample_count = 0;
	calcSmoothFactor(pos, avg, sample_count);
	
	if (sample_count)
		smoothTer(pos, avg / (float)sample_count, dtime);
}

void App::calcSmoothFactor(Vector3 &pos, float& avg, int& sample_count)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, scn->sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = scn->terrain->getHeightData();
	int mapPos;

	avg = 0.0f;  sample_count = 0;
	
	for (int j = rcMap.top;j < rcMap.bottom; ++j)
	{
		mapPos = j * scn->sc->td.iTerSize + rcMap.left;
		for (int i = rcMap.left;i < rcMap.right; ++i)
		{
			avg += fHmap[mapPos];  ++mapPos;
		}
	}
	sample_count = (rcMap.right - rcMap.left) * (rcMap.bottom - rcMap.top);
}

//-----------------------------------------------------------------------------------------------
void App::smoothTer(Vector3 &pos, float avg, float dtime)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, scn->sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = scn->terrain->getHeightData();
	float mRatio = 1.f, brushPos;
	int mapPos;
	float mFactor = mBrIntens[curBr] * dtime * 0.1f;

	for(int j = rcMap.top;j < rcMap.bottom;j++)
	{
		brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BrushMaxSize;
		brushPos += rcBrush.left;
		//**/brushPos += cy * BrushMaxSize + cx;
		mapPos = j * scn->sc->td.iTerSize + rcMap.left;

		for(int i = rcMap.left;i < rcMap.right;i++)
		{
			float val = avg - fHmap[mapPos];
			val = val * std::min(mBrushData[(int)brushPos] * mFactor, 1.0f);
			fHmap[mapPos] += val;
			++mapPos;
			brushPos += mRatio;
		}
	}
	scn->terrain->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}


///  ^v \ Filter - low pass, removes noise
//-----------------------------------------------------------------------------------------------
void App::filter(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, scn->sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = scn->terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * std::min(1.f,brMul);  //mul >1 errors
	int mapPos, brPos, jj = cy,
		ter = scn->sc->td.iTerSize, ter2 = ter*ter, ter1 = ter+1;

	const float fl = mBrFilt;  const int f = ceil(fl);
	int x,y,m,yy,i,j;
	
	for (j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * ter + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (i = rcMap.left; i < rcMap.right; ++i)
		if (mapPos -f*ter1 >= 0 && mapPos +f*ter1 < ter2)  // ter borders
		{
			//  sum in kernel
			float s = 0.f;  m = 0;
			for (y = -f; y <= f; ++y) {  yy = y*ter-f;
			for (x = -f; x <= f; ++x, ++m, ++yy)
				s += fHmap[mapPos + yy] * pBrFmask[m];  }
				
			fHmap[mapPos] += (s-fHmap[mapPos]) * mBrushData[brPos] * its;  // filter
			++mapPos;  ++brPos;
		}
	}

	scn->terrain->dirtyRect(rcMap);
	scn->UpdBlendmap();
	bTerUpd = true;
}



//  preview texture for brush and noise ter gen
//--------------------------------------------------------------------------------------------------------------------------
void App::createBrushPrv()
{
	//  brush
	brushPrvTex = TextureManager::getSingleton().createManual(
		"BrushPrvTex", rgDef, TEX_TYPE_2D,
		BrPrvSize,BrPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	 	
	MaterialPtr mat = MaterialManager::getSingleton().create(
		"BrushPrvMtr", rgDef);
	 
	Pass* pass = mat->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("BrushPrvTex");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

	if (ovBrushMtr)
		ovBrushMtr->setMaterialName("BrushPrvMtr");

	updateBrushPrv(true);
	
	//  ter gen
	terPrvTex = TextureManager::getSingleton().createManual(
		"TerPrvTex", rgDef, TEX_TYPE_2D,
		TerPrvSize,TerPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	 	
	mat = MaterialManager::getSingleton().create(
		"TerPrvMtr", rgDef);
	 
	pass = mat->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("TerPrvTex");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

	if (ovTerMtr)
		ovTerMtr->setMaterialName("TerPrvMtr");

	updateTerPrv(true);
}
