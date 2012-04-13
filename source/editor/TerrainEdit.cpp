#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include <OgreHardwarePixelBuffer.h>
//#include "../vdrift/settings.h"
using namespace Ogre;


///  get edit rect
//--------------------------------------------------------------------------------------------------------------------------
bool App::getEditRect(Vector3& pos, Rect& rcBrush, Rect& rcMap, int size,  int& cx, int& cy)
{
	float tws = sc.td.fTerWorldSize;
	int t = sc.td.iTerSize;

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


static float GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (2*PI_d - atan2f(y, x));
}/**/


///  update brush preview texture  ---------------------------------
void App::updateBrushPrv(bool first)
{
	if (!first && (!ovBrushPrv || edMode >= ED_Road || !bEdit()))  return;
	if (!pSet->brush_prv || brushPrvTex.isNull())  return;

	//  Lock texture and fill pixel data
	HardwarePixelBufferSharedPtr pbuf = brushPrvTex->getBuffer();
	pbuf->lock(HardwareBuffer::HBL_DISCARD);
	const PixelBox& pb = pbuf->getCurrentLock();
	uint8* p = static_cast<uint8*>(pb.data);

	float s = BrPrvSize * 0.5f, s1 = 1.f/s,
		fP = mBrPow[curBr], fQ = mBrFq[curBr]*5.f;  int oct = mBrOct[curBr];

	const static float cf[4][3] = {  // color factors
		{0.3, 0.8, 0.1}, {0.2, 0.8, 0.6}, {0.6, 0.9, 0.6}, {0.4, 0.7, 1.0} };
	float fB = cf[edMode][0]*255.f, fG = cf[edMode][1]*255.f, fR = cf[edMode][2]*255.f;

	switch (mBrShape[curBr])
	{
	case BRS_Triangle:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( abs(d), fP);
			
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

	case BRS_Noise:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( abs(Noise(x*s1,y*s1, fQ, oct, 0.5f)), fP*0.5f) * 0.9f;
			
			//float aa = GetAngle(fx, fy), am = 2*PI_d;
			//float n = aa/am		 * Noise(     aa*0.1f, 0.1f * fP, 3, 0.7f)
			//		+ (am-aa)/am * Noise((am-aa)*0.1f, 0.1f * fP, 3, 0.7f);
			//float c = d * pow( n * 2.f, 4.f);  //star-
			
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
	if (mBrSize[curBr] < 1)  mBrSize[curBr] = 1;
	if (mBrSize[curBr] > BrushMaxSize)  mBrSize[curBr] = BrushMaxSize;
	if (mBrFq[curBr] < 0.1)  mBrFq[curBr] = 0.1;
	if (mBrPow[curBr] < 0.02)  mBrPow[curBr] = 0.02;

	int size = (int)mBrSize[curBr], a = 0;
	float s = size * 0.5f, s1 = 1.f/s,
		fP = mBrPow[curBr], fQ = mBrFq[curBr]*5.f;  int oct = mBrOct[curBr];

	switch (mBrShape[curBr])
	{
	case BRS_Triangle:
		for (int y = 0; y < size; ++y)
		{	a = y * BrushMaxSize;
			for (int x = 0; x < size; ++x,++a)
			{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
				float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
				
				float c = powf( abs(d), fP);
				mBrushData[a] = c;
		}	}	break;

	case BRS_Sinus:
		for (int y = 0; y < size; ++y)
		{	a = y * BrushMaxSize;
			for (int x = 0; x < size; ++x,++a)
			{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
				float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
				
				float c = powf( sinf(d * PI_d*0.5f), fP);
				mBrushData[a] = c;
		}	}	break;

	case BRS_Noise:
		for (int y = 0; y < size; ++y)
		{	a = y * BrushMaxSize;
			for (int x = 0; x < size; ++x,++a)
			{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
				float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
				
				float c = d * pow( abs(Noise(x*s1,y*s1, fQ, oct, 0.5f)), fP*0.5f);

				//float aa = GetAngle(fx, fy);
				//float c = d * pow( Noise(aa*0.01f,aa*0.1f, 0.3f * fP, 1, 0.7f) * 1.1f, 2.f);  //star-
				mBrushData[a] = std::max(-1.f, std::min(1.f, c ));
		}	}	break;
	}
	
	//  filter brush kernel  ------
	if (mBrFilt != mBrFiltOld)
	{	mBrFilt = std::max(0.f, std::min(8.f, mBrFilt));
		mBrFiltOld = mBrFilt;
	
		delete[] pBrFmask;  pBrFmask = 0;
		const float fl = mBrFilt;
		const int f = ceil(fl);  float fd = 1.f + fl - floor(fl);
		register int m,i,j;

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


///  ^v Deform
//--------------------------------------------------------------------------------------------------------------------------
void App::deform(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc.td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * sc.td.iTerSize + rcMap.left;
		brPos = jj * BrushMaxSize + cx;
		//brPos = std::max(0, std::min(BrushMaxSize*BrushMaxSize-1, brPos ));

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			///  pos float -> brush data compute here (for small size brushes) ..
			fHmap[mapPos] += mBrushData[brPos] * its;  // deform
			++mapPos;  ++brPos;
		}
	}
	terrain->dirtyRect(rcMap);
	GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
	//initBlendMaps(terrain);
	bTerUpd = true;
}


///  -_ set Height
//--------------------------------------------------------------------------------------------------------------------------
void App::height(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc.td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
		
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * sc.td.iTerSize + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (int i = rcMap.left; i < rcMap.right; ++i)
		{
			float d = terSetH - fHmap[mapPos];
			d = d > 2.f ? 2.f : d < -2.f ? -2.f : d;  // par speed-
			fHmap[mapPos] += d * mBrushData[brPos] * its;
			++mapPos;  ++brPos;
		}
	}
	terrain->dirtyRect(rcMap);
	GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
	bTerUpd = true;
}


///  ~- Smooth (Flatten)
//--------------------------------------------------------------------------------------------------------------------------
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
	if (!getEditRect(pos, rcBrush, rcMap, sc.td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	int mapPos;

	avg = 0.0f;  sample_count = 0;
	
	for (int j = rcMap.top;j < rcMap.bottom; ++j)
	{
		mapPos = j * sc.td.iTerSize + rcMap.left;
		for (int i = rcMap.left;i < rcMap.right; ++i)
		{
			avg += fHmap[mapPos];  ++mapPos;
		}
	}
	sample_count = (rcMap.right - rcMap.left) * (rcMap.bottom - rcMap.top);
}

//--------------------------------------------------------------------------------------------------------------------------
void App::smoothTer(Vector3 &pos, float avg, float dtime)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc.td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	float mRatio = 1.f, brushPos;
	int mapPos;
	float mFactor = mBrIntens[curBr] * dtime * 0.1f;

	for(int j = rcMap.top;j < rcMap.bottom;j++)
	{
		brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BrushMaxSize;
		brushPos += rcBrush.left;
		//**/brushPos += cy * BrushMaxSize + cx;
		mapPos = j * sc.td.iTerSize + rcMap.left;

		for(int i = rcMap.left;i < rcMap.right;i++)
		{
			float val = avg - fHmap[mapPos];
			val = val * std::min(mBrushData[(int)brushPos] * mFactor, 1.0f);
			fHmap[mapPos] += val;
			++mapPos;
			brushPos += mRatio;
		}
	}
	terrain->dirtyRect(rcMap);
	GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
	bTerUpd = true;
}


///  ^v \ Filter - low pass, removes noise
//--------------------------------------------------------------------------------------------------------------------------
void App::filter(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc.td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy,
		ter = sc.td.iTerSize, ter2 = ter*ter, ter1 = ter+1;

	const float fl = mBrFilt;  const int f = ceil(fl);
	register int x,y,m,yy,i,j;
	
	for (j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * ter + rcMap.left;
		brPos = jj * BrushMaxSize + cx;

		for (i = rcMap.left; i < rcMap.right; ++i)
		if (mapPos -f*ter1 >= 0 && mapPos +f*ter1 < ter2)  // ter borders
		{
			//  sum in kernel
			register float s = 0.f;  m = 0;
			for (y = -f; y <= f; ++y) {  yy = y*ter-f;
			for (x = -f; x <= f; ++x, ++m, ++yy)
				s += fHmap[mapPos + yy] * pBrFmask[m];  }
				
			fHmap[mapPos] += (s-fHmap[mapPos]) * mBrushData[brPos] * its;  // filter
			++mapPos;  ++brPos;
		}
	}

	terrain->dirtyRect(rcMap);
	GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
	//initBlendMaps(terrain);
	bTerUpd = true;
}


///  \\ Ramp, par: angle, height? roll?
///  .. one shot brushes, geom shapes: circle,arc,rectangle,etc.
///  change ter roll,pitch brush
//--------------------------------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------------------------------
/*void OgreApp::_splat(Vector3 &pos, float dtime)
{
	Rect rcBrush, rcMap;

	//Terrain *terrain = static_cast<Terrain*>(handle->getHandle());

	int mBlendMapSize = terrain->getLayerBlendMapSize();
	pos.x *= (float)mBlendMapSize;
	pos.y = (1.0f - pos.y) * (float)mBlendMapSize;
	if(!_getEditRect(pos, rcBrush, rcMap, mBlendMapSize))
		return;

	int mLayer = 0;
	
	mLayer = handle->_getLayerID(mTextureDiffuse, mTextureNormal, mEditDirection);  //--
	
	if(mLayer < 1)
		return;

	int mLayerMax = terrain->getLayerCount();
	TerrainLayerBlendMap *mBlendMaps[128];
	float *mBlendDatas[128];

	TerrainLayerBlendMap *mCurrentBlendMap = terrain->getLayerBlendMap(mLayer);
	float *mCurrentBlendData = mCurrentBlendMap->getBlendPointer();

	for(int l = mLayer;l < mLayerMax;l++)
	{
		mBlendMaps[l] = terrain->getLayerBlendMap(l);
		mBlendDatas[l] = mBlendMaps[l]->getBlendPointer();
	}

	float mRatio = (float)BRUSH_DATA_SIZE / (float)mBrushSize;
	float brushPos;
	int mapPos;

	int right = rcBrush.right;
	rcBrush.right = mBlendMapSize - rcBrush.left;
	rcBrush.left = mBlendMapSize - right;

	float factor = mBrushIntensity * dtime * 0.2f;
	if(!mEditDirection)
	{
		int u;
		float sum;

		for(int j = rcMap.top;j < rcMap.bottom;j++)
		{
			brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BRUSH_DATA_SIZE;
			brushPos += rcBrush.right;

			mapPos = (j * mBlendMapSize) + rcMap.left;

			for(int i = rcMap.left;i < rcMap.right;i++)
			{
				brushPos -= mRatio;

				assert(mapPos < (mBlendMapSize * mBlendMapSize) && mapPos >= 0);
				assert((int)brushPos < (BRUSH_DATA_SIZE * BRUSH_DATA_SIZE) && (int)brushPos >= 0);

				sum = 0.0f;
				
				for(u = mLayer + 1;u < mLayerMax;u++)
					sum += mBlendDatas[u][mapPos];
				
				float val = mCurrentBlendData[mapPos] + (mBrushData[(int)brushPos] * factor);
				sum += val;

				if(sum > 1.0f)
				{
					float normfactor = 1.0f / (float)sum;
					mCurrentBlendData[mapPos] = val * normfactor;
					for(u = mLayer + 1;u < mLayerMax;u++)
						mBlendDatas[u][mapPos] *= normfactor;
				}
				else
					mCurrentBlendData[mapPos] = val;
				
				++mapPos;
			}
		}
		for(u = mLayer;u < mLayerMax;u++)
		{
			mBlendMaps[u]->dirtyRect(rcMap);
			mBlendMaps[u]->update();
		}
	}
	else
	{
		for(int j = rcMap.top;j < rcMap.bottom;j++)
		{
			brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BRUSH_DATA_SIZE;
			brushPos += rcBrush.right;

			mapPos = (j * mBlendMapSize) + rcMap.left;

			for(int i = rcMap.left;i < rcMap.right;i++)
			{
				brushPos -= mRatio;

				assert(mapPos < (mBlendMapSize * mBlendMapSize) && mapPos >= 0);
				assert((int)brushPos < (BRUSH_DATA_SIZE * BRUSH_DATA_SIZE) && (int)brushPos >= 0);

				float val = mCurrentBlendData[mapPos] - (mBrushData[(int)brushPos] * factor);

				if(val < 0.0f)
					val = 0.0f;

				mCurrentBlendData[mapPos] = val;
				++mapPos;
			}
		}
		mCurrentBlendMap->dirtyRect(rcMap);
		mCurrentBlendMap->update();
	}
}/**/


//-----------------------------------------------------------------------------------------
/*void OgreApp::_paint(Vector3 &pos, float dtime)
{
	Rect rcBrush, rcMap;
	int ColourMapSize = mColourMapTextureSize->get();
	pos.x *= (float)ColourMapSize;
	pos.y = (1.0f - pos.y) * (float)ColourMapSize;
	if(!_getEditRect(pos, rcBrush, rcMap, ColourMapSize))
		return;

	float mRatio = (float)BRUSH_DATA_SIZE / (float)mBrushSize;
	float brushPos;
	int mapPos;

	int buffersize = terrain->getGlobalColourMap()->getBuffer()->getSizeInBytes();
	int spacing = buffersize / (ColourMapSize * ColourMapSize);
	unsigned char *data = (unsigned char *)terrain->getGlobalColourMap()->getBuffer()->lock(0,  buffersize, HardwareBuffer::HBL_NORMAL);
	PixelFormat pf = terrain->getGlobalColourMap()->getBuffer()->getFormat();
	ColourValue colVal;

	int right = rcBrush.right;
	rcBrush.right = ColourMapSize - rcBrush.left;
	rcBrush.left = ColourMapSize - right;

	float factor = std::min(mBrushIntensity * dtime * 0.2f, 1.0f);
	float bfactor;
	if(!mEditDirection)
	{
		for(int j = rcMap.top;j < rcMap.bottom;j++)
		{
			brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BRUSH_DATA_SIZE;
			brushPos += rcBrush.right;

			mapPos = (j * ColourMapSize) + rcMap.left;

			for(int i = rcMap.left;i < rcMap.right;i++)
			{
				brushPos -= mRatio;

				assert(mapPos < (ColourMapSize * ColourMapSize) && mapPos >= 0);
				assert((int)brushPos < (BRUSH_DATA_SIZE * BRUSH_DATA_SIZE) && (int)brushPos >= 0);

				bfactor = mBrushData[(int)brushPos] * factor;
				PixelUtil::unpackColour(&colVal, pf, (void*)&data[mapPos * spacing]);
				colVal.r = colVal.r + ((mColour.r - colVal.r) * bfactor);
				colVal.g = colVal.g + ((mColour.g - colVal.g) * bfactor);
				colVal.b = colVal.b + ((mColour.b - colVal.b) * bfactor);
				PixelUtil::packColour(colVal, pf, (void*)&data[mapPos * spacing]);

				++mapPos;
			}
		}
	}
	else
	{
		for(int j = rcMap.top;j < rcMap.bottom;j++)
		{
			brushPos = (rcBrush.top + (int)((j - rcMap.top) * mRatio)) * BRUSH_DATA_SIZE;
			brushPos += rcBrush.right;

			mapPos = (j * ColourMapSize) + rcMap.left;

			for(int i = rcMap.left;i < rcMap.right;i++)
			{
				brushPos -= mRatio;

				assert(mapPos < (ColourMapSize * ColourMapSize) && mapPos >= 0);
				assert((int)brushPos < (BRUSH_DATA_SIZE * BRUSH_DATA_SIZE) && (int)brushPos >= 0);

				bfactor = mBrushData[(int)brushPos] * factor;
				PixelUtil::unpackColour(&colVal, pf, (void*)&data[mapPos * spacing]);
				colVal.r = colVal.r + ((1.0f - colVal.r) * bfactor);
				colVal.g = colVal.g + ((1.0f - colVal.g) * bfactor);
				colVal.b = colVal.b + ((1.0f - colVal.b) * bfactor);
				PixelUtil::packColour(colVal, pf, (void*)&data[mapPos * spacing]);

				++mapPos;
			}
		}
	}
	terrain->getGlobalColourMap()->getBuffer()->unlock();
}/**/


//  preview texture for brush and noise ter gen
void App::createBrushPrv()
{
	brushPrvTex = TextureManager::getSingleton().createManual(
		"BrushPrvTex", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D, BrPrvSize,BrPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	 	
	//reloadMtrTex("BrushPrvMtr");
	//ResourcePtr mt = Ogre::MaterialManager::getSingleton().getByName("BrushPrvMtr");
	//if (!mt.isNull())  mt->reload();

	// Create a material using the texture
	MaterialPtr material = MaterialManager::getSingleton().create(
		"BrushPrvMtr", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	 
	Pass* pass = material->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("BrushPrvTex");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

	if (ovBrushMtr)
		ovBrushMtr->setMaterialName("BrushPrvMtr");

	updateBrushPrv(true);
}
