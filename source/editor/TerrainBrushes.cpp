#include "pch.h"
#include "CApp.h"


//  Brush Presets data
//---------------------------------------------------------------------------------------------------------------
const App::BrushSet App::brSets[App::brSetsNum] = {
	//ED_MODE,curBr,Size,Intens, Pow,  Freq,  Ofs, Oct, EBrShape,  Filter,HSet, newLine, Name
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
