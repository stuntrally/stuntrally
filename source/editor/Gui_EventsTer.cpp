#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include <fstream>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
#include "../vdrift/pathmanager.h"
using namespace MyGUI;
using namespace Ogre;


///  [Terrain]
//-----------------------------------------------------------------------------------------------------------

///  Change terrain texture layer, update values
//
void App::tabTerLayer(TabPtr wp, size_t id)
{
	idTerLay = id;  // help vars
	bTerLay = id < sc->td.ciNumLay;
	float scale = 0.f;
	TerLayer* lay = &sc->td.layerRoad;
	noBlendUpd = true;

	//if (tabsTerLayers->getItemSelected()->getCaption().asUTF8() == "Road")
	cmbTexDiff->setVisible(bTerLay);  cmbTexNorm->setVisible(bTerLay);
	chkTerLay->setVisible(bTerLay);   chkTexNormAuto->setVisible(bTerLay);  chkTerLayTripl->setVisible(bTerLay);
	imgTexDiff->setVisible(bTerLay);  edTerLScale->setVisible(bTerLay);
	
	if (bTerLay)
	{	if (id >= sc->td.ciNumLay)  return;
		lay = &sc->td.layersAll[id];

		chkTerLay->setStateSelected(lay->on);
		cmbTexDiff->setIndexSelected( cmbTexDiff->findItemIndexWith(lay->texFile) );
		cmbTexNorm->setIndexSelected( cmbTexNorm->findItemIndexWith(lay->texNorm) );

		//  auto norm check  if  norm = tex + _nh
		String sTex,sNorm, sExt;
		StringUtil::splitBaseFilename(lay->texFile,sTex,sExt);
		StringUtil::splitBaseFilename(lay->texNorm,sNorm,sExt);
		bool bAuto = sTex + "_nh" == sNorm;
		chkTexNormAuto->setStateSelected(bAuto);
		//  tex image
	    imgTexDiff->setImageTexture(sTex + "_prv.png");
	    scale = lay->tiling;

		//  Ter Blendmap
		Slider* sl;
		Slv(TerLAngMin, lay->angMin/90.f);  Slv(TerLHMin, (lay->hMin+300.f)/600.f);
		Slv(TerLAngMax, lay->angMax/90.f);	Slv(TerLHMax, (lay->hMax+300.f)/600.f);
		Slv(TerLAngSm, lay->angSm/90.f);	Slv(TerLHSm, lay->hSm/200.f);
		Slv(TerLNoise, (lay->noise+2.f)/4.f);
		chkTerLNoiseOnly->setStateSelected(lay->bNoiseOnly);
		chkTerLayTripl->setStateSelected(lay->triplanar);
	}

	//  scale layer
	sldTerLScale->setVisible(bTerLay);
	if (bTerLay)  {
		edTerLScale->setCaption(toStr(scale));
		editTerLScale(edTerLScale);  }
	SetUsedStr(valTerLAll, sc->td.layers.size(), 3);
	
	//  Terrain Particles
	edLDust->setCaption(toStr(lay->dust));	edLDustS->setCaption(toStr(lay->dustS));
	edLMud->setCaption(toStr(lay->mud));	edLSmoke->setCaption(toStr(lay->smoke));
	edLTrlClr->setCaption(toStr(lay->tclr));  if (clrTrail)  clrTrail->setColour(Colour(lay->tclr.r,lay->tclr.g,lay->tclr.b));
	
	//  Surfaces
	cmbSurface->setIndexSelected( cmbSurface->findItemIndexWith(
		!bTerLay ? sc->td.layerRoad.surfName : sc->td.layersAll[idTerLay].surfName ));
	UpdSurfInfo();

	noBlendUpd = false;
}

void App::editTerTriSize(EditPtr ed)
{
	Real r = std::max(0.1f, s2r(ed->getCaption()) );
	sc->td.fTriangleSize = r;  sc->td.UpdVals();

	Slider* sl = (Slider*)mWndOpts->findWidget("TerTriSize");  // set slider
	float v = std::min(1.f, powf((r -0.1f)/5.9f, 0.5f) );
	if (sl)  sl->setValue(v);
	// result val text
	int size = getHMapSizeTab();
	if (valTerTriSize){  valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }
}
// |
void App::slTerTriSize(SL)
{
	Real v = 0.1f + 5.9f * powf(val, 2.f);
	sc->td.fTriangleSize = v;  sc->td.UpdVals();
	if (edTerTriSize)  edTerTriSize->setCaption(toStr(v));  // set edit
	// result val text
	int size = getHMapSizeTab();
	if (valTerTriSize){  valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }
}

int App::getHMapSizeTab()
{
	const char* str = tabsHmap->getItemSelected()->getCaption().asUTF8().substr(7).c_str();
	return atoi(str);
}
void App::tabHmap(TabPtr wp, size_t id)
{
	int size = getHMapSizeTab();
	if (valTerTriSize){  valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }
}

void App::editTerErrorNorm(MyGUI::EditPtr ed)
{
	Real r = std::max(0.f, s2r(ed->getCaption()) );
	sc->td.errorNorm = r;  UpdTerErr();
}


//  - - - -  Hmap tools  - - - -
const char* App::getHMapNew()
{
	static String name = TrkDir() + "heightmap-new.f32";
	return name.c_str();
}

//----------------------------------------------------------------------------------------------------------
void App::btnTerrainNew(WP)
{
	int size = getHMapSizeTab();
	if (valTerTriSize){  valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }

	sc->td.iVertsX = size+1;  sc->td.UpdVals();  // new hf

	float* hfData = new float[sc->td.iVertsX * sc->td.iVertsY];
	int siz = sc->td.iVertsX * sc->td.iVertsY * sizeof(float);
	
	//  generate Hmap
	for (int j=0; j < sc->td.iVertsY; ++j)
	{
		int a = j * sc->td.iVertsX;
		for (int i=0; i < sc->td.iVertsX; ++i,++a)
			hfData[a] = 0.f;  //sc->td.getHeight(i,j);
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	bNewHmap = true;	UpdateTrack();
}

//  Terrain  generate  --------------------------------
void App::btnTerGenerate(WP)
{
	float* hfData = new float[sc->td.iVertsX * sc->td.iVertsY];
	int siz = sc->td.iVertsX * sc->td.iVertsY * sizeof(float);
	float s = sc->td.fTriangleSize*0.001f,
		ox = pSet->gen_ofsx *s*sc->td.iVertsX, oy = pSet->gen_ofsy *s*sc->td.iVertsY;

	//  generate noise terrain hmap
	for (int j=0; j < sc->td.iVertsY; ++j)
	{	int a = j * sc->td.iVertsX;
		for (int i=0; i < sc->td.iVertsX; ++i,++a)
		{
			float y = Noise(i*s-oy, j*s+ox, pSet->gen_freq, pSet->gen_oct, pSet->gen_persist);
			y = y >= 0.f ? powf(y, pSet->gen_pow) : -powf(-y, pSet->gen_pow);
			hfData[a] = y * pSet->gen_scale;
		}
	}

	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	bNewHmap = true;	UpdateTrack();
}

//  Terrain  half  --------------------------------
void App::btnTerrainHalf(WP)
{
	int size = getHMapSizeTab() / 2;
	if (valTerTriSize){ valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }

	int halfSize = (sc->td.iVertsX-1) / 2 +1;
	float* hfData = new float[halfSize * halfSize];
	int siz = halfSize * halfSize * sizeof(float);
	
	//  resize Hmap by half
	for (int j=0; j < halfSize; ++j)
	{
		int a = j * halfSize, a2 = j*2 * sc->td.iVertsX;
		for (int i=0; i < halfSize; ++i,++a)
		{	hfData[a] = sc->td.hfHeight[a2];  a2+=2;  }
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();
	delete[] hfData;

	sc->td.fTriangleSize *= 2.f;
	sc->td.iVertsX = halfSize;  sc->td.UpdVals();
	bNewHmap = true;	UpdateTrack();
}

//  Terrain  double  --------------------------------
#if 1
void App::btnTerrainDouble(WP)
{
	int size = getHMapSizeTab() / 2;
	if (valTerTriSize){ valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }

	int dblSize = (sc->td.iVertsX-1) * 2 +1;
	float* hfData = new float[dblSize * dblSize];
	int siz = dblSize * dblSize * sizeof(float);
	
	//  resize Hmap by half
	for (int j=0; j < sc->td.iVertsX; ++j)
	{
		int a = (j +dblSize/4) * dblSize + dblSize/4, a2 = j * sc->td.iVertsX;
		for (int i=0; i < sc->td.iVertsX; ++i,++a)
		{	hfData[a] = sc->td.hfHeight[a2];  ++a2;  }
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();
	delete[] hfData;

	sc->td.iVertsX = dblSize;  sc->td.UpdVals();
	bNewHmap = true;	UpdateTrack();
}
#else
//  Terrain  resize  --------------------------------
void App::btnTerrainDouble(WP)
{
	int size = getHMapSizeTab() / 2;
	if (valTerTriSize){ valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }

	int oldSize = sc->td.iVertsX, osi = oldSize*oldSize,
		newSize = (oldSize-1) * 2 +1;
	float scale = 1.f / 2.f / 2.f;
	float* hfData = new float[si];
	for (int i=0; i < si; ++i)  hfData[i] = 0.f;  // clear out
	
	//  resize
	register int i,j,x,y,a;
	for (j=0; j < newSize; ++j)
	{
		y = scale * j;
		a = j * newSize;
		for (i=0; i < newSize; ++i,++a)
		{
			x = y * oldSize + scale * i;
			x = std::max(0, std::min(osi-1, x));
			hfData[a] = sc->td.hfHeight[ x ];
		}
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	sc->td.fTriangleSize * scale * 2.f;
	sc->td.iVertsX = newSize;  sc->td.UpdVals();
	bNewHmap = true;	UpdateTrack();  SetGuiFromXmls();
}
#endif

//  Terrain  move  --------------------------------
void App::btnTerrainMove(WP)
{
	EditPtr ex = (EditPtr)mWndEdit->findWidget("TerMoveX");
	EditPtr ey = (EditPtr)mWndEdit->findWidget("TerMoveY");
	int mx = ex ? s2i(ex->getCaption()) : 0;
	int my = ey ?-s2i(ey->getCaption()) : 0;
	
	int newSize = sc->td.iVertsX, si = newSize * newSize;
	float* hfData = new float[si];
	
	//  resize
	register int i,j,a,aa;
	for (j=0; j < newSize; ++j)
	{
		a = j * newSize;
		for (i=0; i < newSize; ++i,++a)
		{
			aa = std::max(0, std::min(si-1, (j-mx) * newSize + i+my));
			hfData[a] = sc->td.hfHeight[aa];
		}
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	road->SelAll();
	road->Move(Vector3(my,0,mx) * -sc->td.fTriangleSize);
	road->SelClear();
	//start,objects-

	bNewHmap = true;	UpdateTrack();
}

//  Terrain  height scale  --------------------------------
void App::btnScaleTerH(WP)
{
	if (!edScaleTerHMul || !road)  return;
	Real sf = std::max(0.1f, s2r(edScaleTerHMul->getCaption()) );  // scale mul

	float* hfData = new float[sc->td.iVertsX * sc->td.iVertsY];
	int siz = sc->td.iVertsX * sc->td.iVertsY * sizeof(float);
	
	//  generate Hmap
	for (int j=0; j < sc->td.iVertsY; ++j)
	{
		int a = j * sc->td.iVertsX;
		for (int i=0; i < sc->td.iVertsX; ++i,++a)
			hfData[a] = sc->td.hfHeight[a] * sf;
	}
	std::ofstream of;
	of.open(getHMapNew(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	bNewHmap = true;	UpdateTrack();
	// !onTer road points..
}
//----------------------------------------------------------------------------------------------------------


void App::slTerGenScale(SL)
{
	float v = 160.f * powf(val, 2.f);	if (bGI)  pSet->gen_scale = v;
	if (valTerGenScale){	valTerGenScale->setCaption(fToStr(v,2,4));  }
}
void App::slTerGenOfsX(SL)
{
	float v = -2.f + 4.f * val;		if (bGI)  pSet->gen_ofsx = v;
	if (valTerGenOfsX){	valTerGenOfsX->setCaption(fToStr(v,3,5));  }
}
void App::slTerGenOfsY(SL)
{
	float v = -2.f + 4.f * val;		if (bGI)  pSet->gen_ofsy = v;
	if (valTerGenOfsY){	valTerGenOfsY->setCaption(fToStr(v,3,5));  }
}

void App::slTerGenFreq(SL)
{
	float v = 0.7f * val;	if (bGI)  pSet->gen_freq = v;
	if (valTerGenFreq){	valTerGenFreq->setCaption(fToStr(v,3,5));  }
}
void App::slTerGenOct(SL)
{
	int v = val * 9.f +slHalf;
	if (bGI)  pSet->gen_oct = v;
	if (valTerGenOct){	valTerGenOct->setCaption(toStr(v));  }
}
void App::slTerGenPers(SL)
{
	float v = 0.7f * val;	if (bGI)  pSet->gen_persist = v;
	if (valTerGenPers){	valTerGenPers->setCaption(fToStr(v,3,5));  }
}
void App::slTerGenPow(SL)
{
	float v = 6.f * powf(val, 2.f);		if (bGI)  pSet->gen_pow = v;
	if (valTerGenPow){	valTerGenPow->setCaption(fToStr(v,3,5));  }
}



///  Terrain layers  -----------------------------
//
void App::chkTerLayOn(WP wp)
{
	if (!bTerLay)  return;
	sc->td.layersAll[idTerLay].on = !sc->td.layersAll[idTerLay].on;
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(sc->td.layersAll[idTerLay].on);
	sc->td.UpdLayers();
	SetUsedStr(valTerLAll, sc->td.layers.size(), 3);
	//  force update, blendmap sliders crash if not, !! this doesnt save hmap if changed  todo..
	UpdateTrack();
}

void App::chkTerLayTriplOn(WP wp)
{
	if (!bTerLay)  return;
	sc->td.layersAll[idTerLay].triplanar = !sc->td.layersAll[idTerLay].triplanar;
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(sc->td.layersAll[idTerLay].triplanar);
	sc->td.UpdLayers();
}

void App::chkTexNormAutoOn(WP wp)
{
	bTexNormAuto = !bTexNormAuto;
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(bTexNormAuto);
}

void App::comboTexDiff(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	if (bTerLay)  sc->td.layersAll[idTerLay].texFile = s;

	String sTex,sNorm, sExt;
	StringUtil::splitBaseFilename(s,sTex,sExt);
	sNorm = sTex + "_nh." + sExt;  //same ext

	//  auto norm
	if (bTexNormAuto)
	{	size_t id = cmbTexNorm->findItemIndexWith(sNorm);
		if (id != ITEM_NONE)  // set only if found
			cmbTexNorm->setIndexSelected(id);
		if (bTerLay)  sc->td.layersAll[idTerLay].texNorm = sNorm;  }
	    
	//  tex image
    imgTexDiff->setImageTexture(sTex + "_prv.png");
}

void App::comboTexNorm(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	if (bTerLay)  sc->td.layersAll[idTerLay].texNorm = s;
}

void App::editTerLScale(EditPtr ed)
{
	Real r = std::max(0.01f, s2r(ed->getCaption()) );
	if (bTerLay)  sc->td.layersAll[idTerLay].tiling = r;

	float v = std::min(1.f,std::max(0.f, powf((r - 2.0f)/24.0f, 1.f/1.5f) ));
	if (sldTerLScale)  sldTerLScale->setValue(v);
}
// |
void App::slTerLScale(SL)  //  scale layer
{
	Real v = 2.0f + 24.0f * powf(val, 1.5f);  // 0.1 + 89.9, 1 + 19
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].tiling = v;
	if (edTerLScale)  edTerLScale->setCaption(toStr(v));  // set edit
}


///  Terrain BlendMap  -----------------------------
//
void App::slTerLAngMin(SL)
{
	float v = 90.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].angMin = v;
	if (valTerLAngMin){	valTerLAngMin->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;  //initBlendMaps(terrain);
}
void App::slTerLAngMax(SL)
{
	float v = 90.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].angMax = v;
	if (valTerLAngMax){	valTerLAngMax->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}
void App::slTerLAngSm(SL)
{
	float v = 90.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].angSm = v;
	if (valTerLAngSm){	valTerLAngSm->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}

void App::slTerLHMin(SL)
{
	float v = -300.f + 600.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].hMin = v;
	if (valTerLHMin){	valTerLHMin->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}
void App::slTerLHMax(SL)
{
	float v = -300.f + 600.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].hMax = v;
	if (valTerLHMax){	valTerLHMax->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}
void App::slTerLHSm(SL)
{
	float v = 200.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].hSm = v;
	if (valTerLHSm){	valTerLHSm->setCaption(fToStr(v,0,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}

void App::slTerLNoise(SL)
{
	float v = -2.f + 4.f * val;
	if (bTerLay && bGI)  sc->td.layersAll[idTerLay].noise = v;
	if (valTerLNoise){	valTerLNoise->setCaption(fToStr(v,2,4));  }
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}

void App::chkTerLNoiseOnlyOn(WP wp)
{
	if (!bTerLay)  return;
	sc->td.layersAll[idTerLay].bNoiseOnly = !sc->td.layersAll[idTerLay].bNoiseOnly;
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(sc->td.layersAll[idTerLay].bNoiseOnly);
	if (terrain && bGI && !noBlendUpd)  bTerUpdBlend = true;
}


///  Terrain Particles  -----------------------------
//
void App::editLDust(EditPtr ed)
{
	Real r = s2r(ed->getCaption());
	TerLayer* l = !bTerLay ? &sc->td.layerRoad : &sc->td.layersAll[idTerLay];
	String n = ed->getName();

		 if (n=="LDust")   l->dust = r;		else if (n=="LDustS")  l->dustS = r;
	else if (n=="LMud")    l->mud = r;		else if (n=="LSmoke")  l->smoke = r;
}
void App::editLTrlClr(EditPtr ed)
{
	ColourValue c = s2c(ed->getCaption());
	if (!bTerLay)   sc->td.layerRoad.tclr = c;
	else  sc->td.layersAll[idTerLay].tclr = c;
	if (clrTrail)  clrTrail->setColour(Colour(c.r,c.g,c.b));
}

void App::comboParDust(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	String n = cmb->getName();
		 
		 if (n=="CmbParDust")   sc->sParDust = s;
	else if (n=="CmbParMud")    sc->sParMud = s;
	else if (n=="CmbParSmoke")  sc->sParSmoke = s;
}


///  Terrain Surface  -----------------------------
//
void App::comboSurface(ComboBoxPtr cmb, size_t val)
{
	std::string s = cmb->getItemNameAt(val);
	if (!bTerLay)
		sc->td.layerRoad.surfName = s;
	else
		sc->td.layersAll[idTerLay].surfName = s;
	UpdSurfInfo();
}

void App::UpdSurfInfo()
{
	std::string s = cmbSurface->getCaption();
	int id = surf_map[s]-1;
	if (id == -1)  return;  //not found..
	const TRACKSURFACE& su = surfaces[id];

	txtSurfTire->setCaption(su.tireName);
	txtSuBumpWave->setCaption(fToStr(su.bumpWaveLength, 1,3));
	txtSuBumpAmp->setCaption(fToStr(su.bumpAmplitude, 2,4));
	txtSuRollDrag->setCaption(fToStr(su.rollingDrag, 1,3));
	txtSuFrict->setCaption(fToStr(su.frictionTread, 2,4));
	txtSurfType->setCaption(csTRKsurf[su.type]);
}


///  Surfaces  all in data/cars/surfaces.cfg
//------------------------------------------------------------------------------------------------------------------------------
bool App::LoadAllSurfaces()
{
	surfaces.clear();
	surf_map.clear();

	std::string path = PATHMANAGER::CarSim() + "/normal/surfaces.cfg";
	CONFIGFILE param;
	if (!param.Load(path))
	{
		LogO("Can't find surfaces configfile: " + path);
		return false;
	}
	
	std::list <std::string> sectionlist;
	param.GetSectionList(sectionlist);
	
	for (std::list<std::string>::const_iterator section = sectionlist.begin(); section != sectionlist.end(); ++section)
	{
		TRACKSURFACE surf;
		surf.name = *section;
		
		int id;
		param.GetParam(*section + ".ID", id);  // for sound..
		//-assert(indexnum >= 0 && indexnum < (int)tracksurfaces.size());
		surf.setType(id);
		
		float temp = 0.0;
		param.GetParam(*section + ".BumpWaveLength", temp);
		surf.bumpWaveLength = temp;
		
		param.GetParam(*section + ".BumpAmplitude", temp);
		surf.bumpAmplitude = temp;
		
		//param.GetParam(*section + ".FrictionNonTread", temp);  //not used
		//surf.frictionNonTread = temp;
		
		param.GetParam(*section + ".FrictionTread", temp);
		surf.frictionTread = temp;
		
		if (param.GetParam(*section + ".RollResistance", temp))
			surf.rollingResist = temp;
		
		param.GetParam(*section + ".RollingDrag", temp);
		surf.rollingDrag = temp;

		///---  Tire  ---
		std::string tireFile;
		if (!param.GetParam(*section + "." + "Tire", tireFile))
			tireFile = "Default";  // default surface if not found
		surf.tireName = tireFile;
		///---
		
		surfaces.push_back(surf);
		surf_map[surf.name] = (int)surfaces.size();  //+1, 0 = not found
	}

	return true;
}
