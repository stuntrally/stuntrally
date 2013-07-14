#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include <OgreTerrain.h>
#include <OgreHardwarePixelBuffer.h>
//#include "../vdrift/settings.h"
using namespace Ogre;


//  Brush Presets data
//---------------------------------------------------------------------------------------------------------------
const App::BrushSet App::brSets[App::brSetsNum] = {
//ED_MODE,curBr,  Size, Intens,  Pow, Freq,  Ofs, Oct, EBrShape, Filter, HSet,  Name
//------  easy sinus
	{ED_Deform,0,  16.f, 10.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Small"},
	{ED_Deform,0,  32.f, 20.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Medium"},
	{ED_Deform,0,  40.f, 20.f,   4.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Spike"},
													
	{ED_Height,2,  32.f, 20.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Height small"},
	{ED_Height,2,  64.f, 20.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f, 0.f,   0, "Height=0 big"},
																				 
	{ED_Smooth,1,  22.f, 30.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Smooth medium"},
	{ED_Smooth,1,  16.f, 60.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Smooth heavy"},
																				 
	{ED_Filter,3,  16.f, 40.f,   4.f,  1.f,   0.f, 5,   BRS_Sinus,   2.f,-0.01f, 0, "Filter small"},
	{ED_Filter,3,  32.f, 20.f,   3.f,  1.f,   0.f, 5,   BRS_Sinus,   4.f,-0.01f, 0, "Filter big"},
	{ED_Filter,3, 128.f, 20.f,   2.f,  1.f,   0.f, 5,   BRS_Sinus,   2.f,-0.01f, 0, "Filter huge"},

	{ED_Smooth,0,  48.f, 50.f,   4.f,  1.f,   5.8f,5,BRS_Triangle,  -1.f,-0.01f, 0, "Smooth light peak"},
	{ED_Smooth,0,  18.f, 20.f,   0.4f, 1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Smooth hard circle"},
	{ED_Deform,0,  32.f, 10.f,   0.5f, 1.f,   0.f, 5,   BRS_Sinus,  -1.f,-0.01f, 0, "Bold"},
	{ED_Deform,0,  48.f, 30.f,   0.05f,1.f,   0.f, 5,BRS_Triangle,  -1.f,-0.01f, 0, "Drop"},

//------  noise	small  Oo
	{ED_Deform,0,  30.f, 20.f,   1.f,  0.72f, 1.5f, 3,  BRS_Noise,  -1.f,-0.01f, 0, "Noise low"},
	{ED_Deform,0,  48.f, 20.f,   2.f,  1.f,   2.5f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "Noise normal"},
	{ED_Deform,0,  24.f, 20.f,   4.f,  0.93f, 2.7f, 5,  BRS_Noise,  -1.f,-0.01f,-1, "Noise random"},
//------  cracks down  xXx
	{ED_Deform,0,  64.f, 20.f,   1.2f, 0.5f,  1.9f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "Cracks sym down"},
	{ED_Deform,0,  60.f, 40.f,   0.7f, 0.4f,  0.f,  5,  BRS_Noise,  -1.f,-0.01f, 0, "Cracks sym"},
	{ED_Deform,0,  96.f, 20.f,   0.7f, 0.25f, 1.9f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "Cracks sym big, detail"},
	{ED_Deform,0,  80.f, 30.f,   0.72f,0.32f,-9.1f, 7,  BRS_Noise,  -1.f,-0.01f, 0, "Cracks asym big"},
	{ED_Deform,0,  60.f, 20.f,   0.68f,0.32f,-6.8f, 6,  BRS_Noise,  -1.f,-0.01f, 0, "Cracks asym big"},

//------  noise	peaks  ^^^
	{ED_Deform,0,  96.f, 20.f,   7.f,  0.37f, 0.9f, 7,  BRS_Noise,  -1.f,-0.01f, 1, "Noise peaks"},
	{ED_Deform,0,  60.f, 40.f,   3.3f, 0.4f,  1.1f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "Rocky scratch"},
	{ED_Deform,0,  60.f, 20.f,   2.3f, 0.13f, 0.f,  5,  BRS_Noise,  -1.f,-0.01f,-1, "High Noise, bumps"},
//------  tool,h,f
	{ED_Smooth,0,  49.f, 57.f,   4.0f, 0.8f,  5.8f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "Smooth noise row"},
	{ED_Height,0,  83.f,120.f,   2.f,  1.f,   0.f,  5,  BRS_Sinus,  -1.f, 20.0f, 0, "Noise Overdriven Height"},
	{ED_Filter,0,  42.f, 20.f,   2.f,  0.93f, 2.7f, 5,  BRS_Noise,   4.f,-0.01f, 0, "Filter Strong"},
	{ED_Filter,0,  70.f, 30.f,   1.9f, 0.2f,  0.2f, 5,  BRS_Noise,   6.f,-0.01f, 0, "Filter Noise"},

//------  rocky asym potataoes
	{ED_Deform,0,  25.f, 20.f,   2.f,  0.4f,  0.f,  5,  BRS_Noise,  -1.f,-0.01f, 1, "row twin"},
	{ED_Deform,0,  46.f, 20.f,   1.3f, 0.34f, 0.5f, 5,  BRS_Noise,  -1.f,-0.01f, 0, "rocky 1"},
											 									  
	{ED_Deform,0,  60.f, 40.f,   0.9f, 0.34f, 1.5f, 4,  BRS_Noise,  -1.f,-0.01f, 0, "rocky 2"},
	{ED_Deform,0,  60.f, 20.f,   1.4f, 0.30f, 0.f,  4,  BRS_Noise,  -1.f,-0.01f, 0, "rocky 3"},
																				  
	{ED_Deform,0,  30.f, 33.f,   1.4f, 0.34f, 6.3f, 4,  BRS_Noise,  -1.f,-0.01f, 0, "noise asymetric"},
	{ED_Deform,0,  30.f, 33.f,   1.4f, 0.34f, 4.1f, 4,  BRS_Noise,  -1.f,-0.01f, 0, "noise cracked 3"},
	{ED_Deform,0,  80.f, 30.f,   0.58f,0.48f,12.1f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "noise big, cracked"},
	{ED_Deform,0,  66.f, 20.f,   8.26f,0.22f,-7.8f, 5,  BRS_Noise2, -1.f,-0.01f, 0, "bold holes 3"},

//------  noise cloud, twin
	{ED_Deform,0,  32.f, 20.f,   2.f,  0.93f, 2.7f, 5,  BRS_Noise2, -1.f,-0.01f, 1, "Noise Cloud"},
											 
	{ED_Deform,0,  45.f, 20.f,   1.5f, 0.4f,  0.5f, 2,  BRS_Noise2, -1.f,-0.01f, 0, "Noise Cloud twin sharp"},
	{ED_Deform,0,  60.f, 40.f,   0.46f,0.95f,44.3f, 4,  BRS_Noise2, -1.f,-0.01f, 0, "Noise Cloud blob"},
											 									 
	{ED_Deform,0,  84.f,120.f,   3.35f,0.6f,  3.8f, 5,  BRS_Noise2, -1.f,-0.01f, 0, "Noise Cloud c"},
	{ED_Deform,0,  68.f, 20.f,   3.09f,0.52f, 7.5f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Noise big twin blob"},
	{ED_Deform,0,  62.f, 20.f,   0.40f,0.48f,22.3f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Noise big 3 Hq"},

//------  laser low oct x*X
	{ED_Deform,0,  32.f, 40.f,   0.6f, 0.2f,  5.5f, 1,  BRS_Noise2, -1.f,-0.01f, 1, "Fancy twin 1"},
	{ED_Deform,0,  33.f, 30.f,   0.6f, 0.2f,  3.6f, 1,  BRS_Noise2, -1.f,-0.01f, 0, "Fancy twin 2"},
	{ED_Deform,0,  30.f, 30.f,   0.54f,0.2f, 15.9f, 1,  BRS_Noise2, -1.f,-0.01f,-1, "Fancy twin 3"},
//------  laser blob glow fancy
	{ED_Deform,0,  60.f, 30.f,   0.54f,0.36f,15.9f, 3,  BRS_Noise2, -1.f,-0.01f, 0, "Fancy blob 1"},
	{ED_Deform,0,  60.f, 30.f,   0.54f,0.65f,15.9f, 3,  BRS_Noise2, -1.f,-0.01f, 0, "Fancy blob 2"},
																				 
	{ED_Deform,0,  80.f, 30.f,   0.84f,0.65f,-13.7f,4,  BRS_Noise2, -1.f,-0.01f, 0, "Fancy blob 3"},
	{ED_Deform,0,  80.f, 30.f,   0.22f,0.64f,-31.4f,4,  BRS_Noise2, -1.f,-0.01f, 0, "Fancy blob 4 big"},
//------  chaos big crack up
	{ED_Deform,0,  96.f, 20.f,   0.54f,0.2f, 14.3f, 7,  BRS_Noise2, -1.f,-0.01f, 1, "Chaos crack big center"},
	{ED_Deform,0,  96.f, 30.f,   0.54f,0.2f, 14.7f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Chaos crack big2"},
	{ED_Deform,0,  88.f, 30.f,   0.54f,0.2f, 15.9f, 3,  BRS_Noise2, -1.f,-0.01f,-1, "Chaos crack"},

	{ED_Deform,0,  72.f, 20.f,   0.41f,0.32f, 0.2f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "Chaos crack twin1"},
	{ED_Deform,0,  82.f, 33.f,   0.55f,0.26f,-5.2f, 4,  BRS_Noise2, -1.f,-0.01f, 0, "Chaos crack twin2"},
	{ED_Deform,0,  66.f, 20.f,   0.70f,0.25f, 1.9f, 5,  BRS_Noise2, -1.f,-0.01f, 0, "Chaos crack h"},
	{ED_Deform,0,  66.f, 20.f,   0.83f,0.22f,-8.1f, 4,  BRS_Noise2, -1.f,-0.01f, 0, "Chaos crack star"},

//------  row | or -
	{ED_Deform,0,  32.f, 40.f,   7.9f, 1.f,  15.4f, 7,  BRS_Noise,  -1.f,-0.01f, 1, "Row| noised"},
	{ED_Deform,0,  51.f, 20.f,  1.19f, 0.4f, -1.0f, 5,  BRS_Noise2, -1.f,-0.01f, 0, "Row| mirror"},
	{ED_Deform,0,  81.f, 30.f,   0.58f,0.48f, 6.6f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "Row| single crack"},

	{ED_Deform,0,  20.f, 20.f,   0.69f,1.0f,  30.f, 4,  BRS_Noise2, -1.f,-0.01f, 0, "Row- sharp"},
	{ED_Deform,0,  46.f, 30.f,   0.97f,0.48f, 4.6f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Row- noised"},
																				 
	{ED_Deform,0,  63.f, 20.f,   3.09f,0.52f, 5.0f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Row- big s"},
	{ED_Deform,0,  67.f, 20.f,   0.68f,0.69f, 2.1f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Row- big sharp v"},

//------  noise	plasma bulb hq
	{ED_Deform,0,  88.f, 20.f,   1.4f, 0.30f, 0.f,  4,  BRS_Noise2, -1.f,-0.01f, 1, "Big detail 1"},
	{ED_Deform,0,  72.f, 20.f,   0.68f,0.69f,13.0f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big detail 2"},
	{ED_Deform,0,  72.f, 20.f,   0.68f,0.46f, 8.3f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big detail 3"},

	{ED_Deform,0,  80.f, 33.f,   0.6f, 0.34f,-5.6f, 4,  BRS_Noise2, -1.f,-0.01f, 0, "Big detail 4"},
	{ED_Deform,0,  85.f, 30.f,   0.53f,0.48f,21.9f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big detail 5"},
//------  noise	lightning hq
	{ED_Deform,0,  90.f, 30.f,   0.36f,0.46f,10.7f, 5,  BRS_Noise2, -1.f,-0.01f, 1, "Big Lightnting 1"},
	{ED_Deform,0, 120.f, 30.f,   0.4f, 0.48f, 8.2f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 2"},
	{ED_Deform,0, 120.f, 30.f,   0.27f,0.32f,-7.5f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 3"},
																				 
	{ED_Deform,0,  90.f, 20.f,   0.41f,0.32f, 2.1f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 4"},
	{ED_Deform,0,  80.f, 30.f,   0.41f,0.32f, 3.5f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 5"},
																		 
	{ED_Deform,0,  80.f, 30.f,   0.54f,0.48f,15.0f, 7,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 6"},
	{ED_Deform,0,  64.f, 20.f,   0.40f,0.48f,22.5f, 6,  BRS_Noise2, -1.f,-0.01f, 0, "Big Lightnting 7"},

//                Size, Intens,  Pow,  Freq,  Ofs, Oct
//------  N-gon															 
	{ED_Deform,0,  30.f, 20.f,  1.19f, 0.4f, -0.7f, 3,  BRS_Ngon,   -1.f,-0.01f, 1, "smooth triangle"},
	{ED_Deform,0,  36.f, 20.f,  1.54f, 0.4f, -1.2f, 5,  BRS_Ngon,   -1.f,-0.01f, 0, "smooth star 5"},
	{ED_Deform,0,  42.f, 20.f,  2.97f, 0.4f, -0.9f, 3,  BRS_Ngon,   -1.f,-0.01f,-1, "sharp star 3"},

	{ED_Deform,0,  32.f, 20.f,  2.97f, 0.4f,  1.0f, 3,  BRS_Ngon,   -1.f,-0.01f, 0, "blob star 3"},
	{ED_Deform,0,  83.f, 20.f,  3.46f, 0.4f,  1.3f, 5,  BRS_Ngon,   -1.f,-0.01f, 0, "flower star 5"},
																				 																				 
	{ED_Deform,0,  22.f, 20.f,  124.f, 0.4f,  0.f,  4,  BRS_Ngon,   -1.f,-0.01f, 1, "ideal square sharp"},
	{ED_Deform,0,  25.f, 20.f,  2.22f, 0.4f, -0.4f, 6,  BRS_Ngon,   -1.f,-0.01f,-1, "smooth hexagon"},
																				 
	{ED_Deform,0,  55.f, 20.f,  124.f, 0.4f, -0.6f, 7,  BRS_Ngon,   -1.f,-0.01f, 0, "sharp star 7"},
	{ED_Deform,0,  55.f, 20.f,  124.f, 0.4f,  0.8f, 8,  BRS_Ngon,   -1.f,-0.01f, 0, "sharp flower 8"},
	{ED_Deform,0, 125.f, 20.f,  204.f, 0.4f, -0.7f, 5,  BRS_Ngon,   -1.f,-0.01f, 0, "inside sharp star 5"},
																				 
	{ED_Smooth,0,  30.f, 20.f,  2.97f, 0.4f, -0.9f, 3,  BRS_Ngon,   -1.f,-0.01f, 0, "Smooth sharp star 3"},
};

//  color factors for edit mode D,S,E,F
const float App::brClr[4][3] = {
	{0.3, 0.8, 0.1}, {0.2, 0.8, 0.6}, {0.6, 0.9, 0.6}, {0.4, 0.7, 1.0} };


void App::btnBrushPreset(WP img)
{
	int id = 0;
	sscanf(img->getName().c_str(), "brI%d", &id);
	SetBrushPreset(id);
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
	if (!pSet->brush_prv || brushPrvTex.isNull())  return;

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

			float c = d * (1.0-pow( fabs(Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, c);
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Noise:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*0.5f) * 0.9f;
			
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
    			fQ * powf(fabs(d / (-1.f+nof + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			
			uint8 bR = c * fR, bG = c * fG, bB = c * fB;
			*p++ = bR;  *p++ = bG;  *p++ = bB;  *p++ = bG > 32 ? 255 : 0;
		}	break;

	case BRS_Triangle:
		for (size_t y = 0; y < BrPrvSize; ++y)
		for (size_t x = 0; x < BrPrvSize; ++x)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = powf( abs(d), fP);
			
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

			float c = d * (1.0-pow( fabs(Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*d)) * (1.5f-fP*0.1);
			c = std::max(0.f, c);
			
			mBrushData[a] = std::min(1.f, c );
		}	}	break;


	case BRS_Noise:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1

			float c = d * pow( fabs(Noise(x*s1+nof,y*s1+nof, fQ, oct, 0.5f)), fP*0.5f);

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
				fQ * powf(fabs(d / (-1.f+nof + cosf(PiN) / cosf( fmodf(k, 2*PiN) - PiN ) )),fP) ));
			mBrushData[a] = c;
		}	}	break;

	case BRS_Triangle:
		for (int y = 0; y < size; ++y) {  a = y * BrushMaxSize;
		for (int x = 0; x < size; ++x,++a)
		{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1
			float d = std::max(0.f, 1.f - float(sqrt(fx*fx + fy*fy)));  // 0..1
			
			float c = powf( abs(d), fP);
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


///  Terrain  generate
///--------------------------------------------------------------------------------------------------------------------------
void App::btnTerGenerate(WP wp)
{
	const std::string& n = wp->getName();
	bool add = false, sub = false;
	if (n == "TerrainGenAdd")  add = true;  else
	if (n == "TerrainGenSub")  sub = true;/*else
	if (n == "TerrainGenMul")  mul = true;*/

	float* hfData = sc->td.hfHeight, *hfAng = sc->td.hfAngle;
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
		catch(...)
			{	imgRoad.load(String("grassDensity.png"),"General");  }
		r = imgRoad.getWidth();
	}

	QTimer ti;  ti.update();  /// time

	//  generate noise terrain hmap
	register int a,x,y;  register float c;

	for (y=0; y < sx; ++y)  {  a = y * sx;
	for (x=0; x < sx; ++x,++a)
	{	float fx = ((float)x - s)*s1, fy = ((float)y - s)*s1;  // -1..1

		c = Noise(y*s1-oy, x*s1+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;
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
		
		c *= linRange(hfAng[a],  pSet->gen_terMinA,pSet->gen_terMaxA, pSet->gen_terSmA);
		c *= linRange(hfData[a], pSet->gen_terMinH,pSet->gen_terMaxH, pSet->gen_terSmH);

		hfData[a] = add ? (hfData[a] + c * pSet->gen_scale + pSet->gen_ofsh) : (
					sub ? (hfData[a] - c * pSet->gen_scale - pSet->gen_ofsh) :
						  (hfData[a] * c * pSet->gen_mul) );
	}	}

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Ter Gen: ") + fToStr(dt,0,3) + " ms");

	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	int siz = sx * sx * sizeof(float);
	of.write((const char*)&hfData[0], siz);
	of.close();

	ti.update();  /// time
	dt = ti.dt * 1000.f;
	LogO(String("::: Time Ter Gen save: ") + fToStr(dt,0,3) + " ms");

	bNewHmap = true;	UpdateTrack();
}

///  update terrain generator preview texture
//--------------------------------------------------------------------------------------------------------------------------
void App::updateTerPrv(bool first)
{
	if (!first && !ovTerPrv)  return;
	if (terPrvTex.isNull())  return;

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

		float c = Noise(x*s1-oy, y*s1+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist) * 0.8f;  // par fit
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
	float tws = sc->td.fTerWorldSize;
	int t = sc->td.iTerSize;

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
	if (!getEditRect(pos, rcBrush, rcMap, sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * sc->td.iTerSize + rcMap.left;
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
	if (pSet->autoBlendmap)
	{
		GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
		initBlendMaps(terrain, rcMap.left,rcMap.top, rcMap.right,rcMap.bottom, false);
	}
	bTerUpd = true;
}


///  -_ set Height
//-----------------------------------------------------------------------------------------------
void App::height(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
		
	float its = mBrIntens[curBr] * dtime * brMul;
	int mapPos, brPos, jj = cy;
	
	for (int j = rcMap.top; j < rcMap.bottom; ++j,++jj)
	{
		mapPos = j * sc->td.iTerSize + rcMap.left;
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
	if (pSet->autoBlendmap)
	{
		GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
		initBlendMaps(terrain, rcMap.left,rcMap.top, rcMap.right,rcMap.bottom, false);
	}
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
	if (!getEditRect(pos, rcBrush, rcMap, sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	int mapPos;

	avg = 0.0f;  sample_count = 0;
	
	for (int j = rcMap.top;j < rcMap.bottom; ++j)
	{
		mapPos = j * sc->td.iTerSize + rcMap.left;
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
	if (!getEditRect(pos, rcBrush, rcMap, sc->td.iTerSize, cx,cy))
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
		mapPos = j * sc->td.iTerSize + rcMap.left;

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
	if (pSet->autoBlendmap)
	{
		GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
		initBlendMaps(terrain, rcMap.left,rcMap.top, rcMap.right,rcMap.bottom, false);
	}
	bTerUpd = true;
}


///  ^v \ Filter - low pass, removes noise
//-----------------------------------------------------------------------------------------------
void App::filter(Vector3 &pos, float dtime, float brMul)
{
	Rect rcBrush, rcMap;  int cx,cy;
	if (!getEditRect(pos, rcBrush, rcMap, sc->td.iTerSize, cx,cy))
		return;
	
	float *fHmap = terrain->getHeightData();
	
	float its = mBrIntens[curBr] * dtime * std::min(1.f,brMul);  //mul >1 errors
	int mapPos, brPos, jj = cy,
		ter = sc->td.iTerSize, ter2 = ter*ter, ter1 = ter+1;

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
	if (pSet->autoBlendmap)
	{
		GetTerAngles(rcMap.left,rcMap.top, rcMap.right,rcMap.bottom);
		initBlendMaps(terrain, rcMap.left,rcMap.top, rcMap.right,rcMap.bottom, false);
	}
	bTerUpd = true;
}

///  todo: one shot brushes..



//  preview texture for brush and noise ter gen
//--------------------------------------------------------------------------------------------------------------------------
void App::createBrushPrv()
{
	brushPrvTex = TextureManager::getSingleton().createManual(
		"BrushPrvTex", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D, BrPrvSize,BrPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	 	
	// Create a material using the texture
	MaterialPtr material = MaterialManager::getSingleton().create(
		"BrushPrvMtr", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	 
	Pass* pass = material->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("BrushPrvTex");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

	if (ovBrushMtr)
		ovBrushMtr->setMaterialName("BrushPrvMtr");

	updateBrushPrv(true);
	
	
	terPrvTex = TextureManager::getSingleton().createManual(
		"TerPrvTex", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D, TerPrvSize,TerPrvSize,0, PF_BYTE_RGBA, TU_DYNAMIC);
	 	
	material = MaterialManager::getSingleton().create(
		"TerPrvMtr", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	 
	pass = material->getTechnique(0)->getPass(0);
	pass->createTextureUnitState("TerPrvTex");
	pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

	if (ovTerMtr)
		ovTerMtr->setMaterialName("TerPrvMtr");

	updateTerPrv(true);


///==============================================================================================
	///  _Tool_ update all Brushes png
	#if 0  // 0 in release !!
	Image im;
	for (int i=0; i < brSetsNum; ++i)
	{
		SetBrushPreset(i);
		brushPrvTex->convertToImage(im);
		im.save("data/editor/brush"+toStr(i)+".png");
		// todo: 1 big 2k texture with all brush.png on it (less batches)..
		// todo: brush presets in xml..
	}
	#endif
}
