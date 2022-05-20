#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "settings.h"
#include "CGui.h"
#include "CApp.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/data/CData.h"
#include "../road/Road.h"
#include "../ogre/common/Slider.h"
#include "../ogre/common/MultiList2.h"
#include "../vdrift/pathmanager.h"
#include <fstream>
#include <MyGUI.h>
#include <OgreRenderTexture.h>
#include "../shiny/Main/Factory.hpp"
using namespace MyGUI;
using namespace Ogre;


///  [Terrain]
//-----------------------------------------------------------------------------------------------------------

///  Change terrain texture layer, update values
//
void CGui::tabTerLayer(Tab wp, size_t id)
{
	idTerLay = id;  // help var
	TerLayer* l = &sc->td.layersAll[idTerLay];

	noBlendUpd = true;
	SldUpd_TerL();

	//  tex
	String s = l->texFile.substr(0, l->texFile.length()-6);
	btnTexDiff->setCaption(s);  // no _d.jpg
	cmbTexNorm->setIndexSelected( cmbTexNorm->findItemIndexWith(l->texNorm) );

	for (int i=0; i < liTex->getItemCount(); ++i)  /// upd pick
		if (liTex->getSubItemNameAt(1,i).substr(7) == s)
			liTex->setIndexSelected(i);

	//  auto norm check
	String sTex,sNorm, sExt;
	StringUtil::splitBaseFilename(l->texFile,sTex,sExt);
	StringUtil::splitBaseFilename(l->texNorm,sNorm,sExt);

	String sTexN = StringUtil::replaceAll(sTex,"_d","_n");
	bool bAuto = sTexN == sNorm;  //_T
	bTexNormAuto = bAuto;
	ckTexNormAuto.Upd();

	//  image
    imgTexDiff->setImageTexture(l->texFile);

	updUsedTer();
	SldUpd_TerLNvis();

	noBlendUpd = false;
}

void CGui::SldUpd_TerLNvis()
{
	//  upd vis of layer noise sliders
	//  check for valid +1,-1,+2 layers
	int ll = sc->td.layers.size();
	int l1 = -1, last = 8, last_2 = 8,  nu = 0, ncl = 0;
	for (int i=0; i < TerData::ciNumLay; ++i)
	if (sc->td.layersAll[i].on)
	{	++nu;
		if (i==idTerLay)  ncl = nu;
		if (nu==1)  l1 = i;
		if (nu==ll)  last = i;
		if (nu==ll-2)  last_2 = i;
	}
	bool ok = idTerLay >= l1 && idTerLay <= last;
	svTerLNoise.setVisible(ok && idTerLay < last);
	svTerLNprev.setVisible(ok && idTerLay > l1);
	svTerLNnext2.setVisible(ok && idTerLay <= last_2 && nu > 2);
	//  dbg img clr
	const static Colour lc[5] = {Colour::Black, Colour::Red, Colour::Green, Colour::Blue, Colour(0.5,0.5,0.5)};
	dbgLclr->setColour(lc[ncl]);
}

void CGui::SldUpd_TerL()
{
	TerLayer* l = &sc->td.layersAll[idTerLay];
	ckTerLayOn.Upd(&l->on);
	svTerLScale.UpdF(&l->tiling);
	ckTerLayTripl.Upd(&l->triplanar);
	//  blmap
	svTerLAngMin.UpdF(&l->angMin);  svTerLHMin.UpdF(&l->hMin);
	svTerLAngMax.UpdF(&l->angMax);  svTerLHMax.UpdF(&l->hMax);
	svTerLAngSm.UpdF(&l->angSm);    svTerLHSm.UpdF(&l->hSm);
	ckTerLNOnly.Upd(&l->nOnly);
	//  noise
	svTerLNoise.UpdF(&l->noise);  svTerLNprev.UpdF(&l->nprev);
	svTerLNnext2.UpdF(&l->nnext2);
	svTerLDmg.UpdF(&l->fDamage);
	//  noise params
	for (int i=0; i<2; ++i)
	{	svTerLN_Freq[i].UpdF(&l->nFreq[i]);
		svTerLN_Oct[i].UpdI(&l->nOct[i]);
		svTerLN_Pers[i].UpdF(&l->nPers[i]);
		svTerLN_Pow[i].UpdF(&l->nPow[i]);
	}
}

//  Tri size
void CGui::slTerTriSize(SV* sv)
{
	sc->td.UpdVals();
	UpdTxtTerSize();
}

int CGui::UpdTxtTerSize(float mul)
{
	int size = getHMapSizeTab() * mul;
	float res = sc->td.fTriangleSize * size;  // result size
	svTerTriSize.setText(fToStr(res,0,3));
	return size;
}

//  HMap size tab
void CGui::updTabHmap()
{
	static std::map<int,int> h;
	if (h.empty()) {  h[128]=0; h[256]=1; h[512]=2; h[1024]=3; h[2048]=4;  }
	tabsHmap->setIndexSelected( h[ sc->td.iTerSize- 1] );
	tabHmap(0,0);
}
int CGui::getHMapSizeTab()
{
	static std::map<int,int> h;
	if (h.empty()) {  h[0]=128; h[1]=256; h[2]=512; h[3]=1024; h[4]=2048;  }
	return h[ tabsHmap->getIndexSelected() ];
}
void CGui::tabHmap(Tab, size_t)
{
	UpdTxtTerSize();
}

void CGui::slTerErrorNorm(SV*sv)
{
	scn->UpdTerErr();
}

//  upd ter sh par
void CGui::slTerPar(SV*sv)
{
	app->mFactory->setSharedParameter("ter_scaleNormal", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(1.f / sc->td.normScale)));
	app->mFactory->setSharedParameter("ter_specular_pow", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(sc->td.specularPow)));
	app->mFactory->setSharedParameter("ter_specular_pow_em", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(sc->td.specularPowEm)));
}

//  - - - -  Hmap tools  - - - -
String CGui::getHMapNew()
{
	return gcom->TrkDir() + "heightmap-new.f32";
}

//----------------------------------------------------------------------------------------------------------
void CGui::btnTerrainNew(WP)
{
	int size = UpdTxtTerSize();
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	app->bNewHmap = true;	app->UpdateTrack();
}


//  Terrain  half  --------------------------------
void CGui::btnTerrainHalf(WP)
{
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();
	delete[] hfData;

	sc->td.fTriangleSize *= 2.f;
	sc->td.iVertsX = halfSize;  sc->td.UpdVals();
	updTabHmap();  svTerTriSize.Upd();
	app->bNewHmap = true;	app->UpdateTrack();
}

//  Terrain  double  --------------------------------
#if 1
void CGui::btnTerrainDouble(WP)
{
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();
	delete[] hfData;

	sc->td.iVertsX = dblSize;  sc->td.UpdVals();
	updTabHmap();
	app->bNewHmap = true;	app->UpdateTrack();
}
#else
//  Terrain  resize ..  --------------------------------
void CGui::btnTerrainDouble(WP)
{
	int size = getHMapSizeTab() / 2;
	if (valTerTriSize){ valTerTriSize->setCaption(fToStr(sc->td.fTriangleSize * size,2,4));  }

	int oldSize = sc->td.iVertsX, osi = oldSize*oldSize,
		newSize = (oldSize-1) * 2 +1;
	float scale = 1.f / 2.f / 2.f;
	float* hfData = new float[si];
	for (int i=0; i < si; ++i)  hfData[i] = 0.f;  // clear out
	
	//  resize
	int i,j,x,y,a;
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	sc->td.fTriangleSize * scale * 2.f;
	sc->td.iVertsX = newSize;  sc->td.UpdVals();
	updTabHmap();  svTerTriSize.Upd();
	app->bNewHmap = true;	app->UpdateTrack();  //SetGuiFromXmls();
}
#endif

//  Terrain  move  --------------------------------
void CGui::btnTerrainMove(WP)
{
	Ed ex = (Ed)app->mWndEdit->findWidget("TerMoveX");
	Ed ey = (Ed)app->mWndEdit->findWidget("TerMoveY");
	int mx = ex ? s2i(ex->getCaption()) : 0;
	int my = ey ?-s2i(ey->getCaption()) : 0;
	
	int newSize = sc->td.iVertsX, si = newSize * newSize;
	float* hfData = new float[si];
	
	//  resize
	int i,j,a,aa;
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	app->scn->road->SelAll();
	app->scn->road->Move(Vector3(my,0,mx) * -sc->td.fTriangleSize);
	app->scn->road->SelClear();
	//start, objects-

	app->bNewHmap = true;	app->UpdateTrack();
}

//  Terrain  height scale  --------------------------------
void CGui::btnScaleTerH(WP)
{
	if (!app->scn->road)  return;
	Real sf = std::max(0.1f, fScaleTer);  // scale mul
	int i;

	//  road
	for (i=0; i < app->scn->road->getNumPoints(); ++i)
		app->scn->road->Scale1(i, 0.f, sf);
	app->scn->road->bSelChng = true;
	
	//  fluids
	for (i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.y *= sf;  fb.size.y *= sf;
	}
	
	//  objs h
	for (i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		o.pos[2] *= sf;
		o.SetFromBlt();
	}

	//  ter  ---
	float* hfData = new float[sc->td.iVertsX * sc->td.iVertsY];
	int siz = sc->td.iVertsX * sc->td.iVertsY * sizeof(float);
	
	//  generate Hmap
	for (int j=0; j < sc->td.iVertsY; ++j)
	{
		int a = j * sc->td.iVertsX;
		for (i=0; i < sc->td.iVertsX; ++i,++a)
			hfData[a] = sc->td.hfHeight[a] * sf;
	}
	std::ofstream of;
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	app->bNewHmap = true;	app->UpdateTrack();

	//  start,end pos
	for (i=0; i < 2; ++i)
		app->scn->sc->startPos[i][2] *= sf;
	app->UpdStartPos();
}
//----------------------------------------------------------------------------------------------------------


//  generator  . . . . . . .
void CGui::slTerGen(SV*)
{
	app->bUpdTerPrv = true;
}

//  debug
void CGui::chkDebugBlend(Ck*)
{
	app->mFactory->setGlobalSetting("debug_blend", b2s(bDebugBlend));
	dbgLclr->setVisible(bDebugBlend);
}


///  Terrain layers  -----------------------------
//
void CGui::chkTerLayOn(Ck* ck)
{
	sc->td.UpdLayers();
	if (sc->td.layers.size()==0)  // would crash, surf
	{
		*ck->pBool = true;  // turn it back on
		ck->Upd();
	}

	sc->td.UpdLayers();
	updUsedTer();

	//todo..  !! save hmap if changed
	app->UpdateTrack();

	SldUpd_TerLNvis();
	UpdSurfList();
}

void CGui::updUsedTer()
{
	SetUsedStr(valTerLAll, sc->td.layers.size(), 3);
	int t = sc->td.triplCnt;
	valTriplAll->setCaption(TR("#{Used}") + ": " + toStr(t));
	valTriplAll->setTextColour(sUsedClr[ t > 2 ? 6 : (t + 2)]);
}

void CGui::chkTerLayTripl(Ck*)
{
	sc->td.UpdLayers();
	updUsedTer();
}

void CGui::comboTexNorm(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	sc->td.layersAll[idTerLay].texNorm = s;
}

//  Terrain BlendMap
void CGui::slTerLay(SV*)
{
	//app->bTerUpdBlend = true;
	scn->UpdLayerPars();
	if (scn->angleRTT.rnd)  scn->angleRTT.rnd->update();
	if (scn->blendRTT.rnd)  scn->blendRTT.rnd->update();
}
void CGui::chkTerLNOnly(Ck*)
{
	slTerLay(0);
}

//  move layer order
void CGui::btnTerLmoveL(WP)  // -1
{
	if (idTerLay <= 0)  return;

	TerLayer& t = sc->td.layersAll[idTerLay], st,
			&t1 = sc->td.layersAll[idTerLay-1];
	st = t;  t = t1;  t1 = st;
	
	sc->td.UpdLayers();  NumTabNext(-1);
	app->scn->UpdBlendmap();
}

void CGui::btnTerLmoveR(WP)  // +1
{
	if (idTerLay >= TerData::ciNumLay-1)  return;

	TerLayer& t = sc->td.layersAll[idTerLay], st,
			&t1 = sc->td.layersAll[idTerLay+1];
	st = t;  t = t1;  t1 = st;
	
	sc->td.UpdLayers();  NumTabNext(1);
	app->scn->UpdBlendmap();
}

///  Noise preset buttons
void CGui::radN1(WP) {  Radio2(bRn1, bRn2, bRn2->getStateSelected());  }
void CGui::radN2(WP) {  Radio2(bRn1, bRn2, bRn2->getStateSelected());  }

const static float ns[15][4] = {  //  freq, oct, pers, pow
{ 30.4f, 3, 0.33f, 1.5f },{ 36.6f, 4, 0.49f, 1.9f },{ 30.7f, 3, 0.30f, 1.5f },{ 29.5f, 2, 0.13f, 1.8f },{ 40.5f, 3, 0.43f, 2.0f },
{ 25.3f, 3, 0.30f, 1.2f },{ 31.3f, 5, 0.70f, 2.0f },{ 28.4f, 4, 0.70f, 1.5f },{ 34.5f, 4, 0.40f, 0.9f },{ 34.3f, 4, 0.54f, 1.0f },
{ 44.6f, 2, 0.30f, 1.1f },{ 48.2f, 3, 0.12f, 1.6f },{ 56.6f, 4, 0.49f, 2.0f },{ 60.4f, 4, 0.51f, 2.0f },{ 62.6f, 3, 0.12f, 2.1f }};

void CGui::btnNpreset(WP wp)
{
	int l = bRn2->getStateSelected() ? 1 : 0;
	String s = wp->getName();  //"TerLN_"
	int i = s2i(s.substr(6));

	TerLayer& t = sc->td.layersAll[idTerLay];
	t.nFreq[l] = ns[i][0];
	t.nOct[l]  = int(ns[i][1]);
	t.nPers[l] = ns[i][2];
	t.nPow[l]  = ns[i][3];
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}
void CGui::btnNrandom(WP wp)
{
	int l = bRn2->getStateSelected() ? 1 : 0;

	TerLayer& t = sc->td.layersAll[idTerLay];
	t.nFreq[l] = Math::RangeRandom(20.f,70.f);
	t.nOct[l]  = Math::RangeRandom(2.f,5.f);
	t.nPers[l] = Math::RangeRandom(0.1f,0.7f);
	t.nPow[l]  = Math::RangeRandom(0.8f,2.4f);
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}

//  swap noise 1 and 2 params
void CGui::btnNswap(WP wp)
{
	TerLayer& t = sc->td.layersAll[idTerLay];
	std::swap(t.nFreq[0], t.nFreq[1]);
	std::swap(t.nOct[0] , t.nOct[1] );
	std::swap(t.nPers[0], t.nPers[1]);
	std::swap(t.nPow[0] , t.nPow[1] );
	SldUpd_TerL();
	app->scn->UpdBlendmap();
}


///  Terrain Particles  -----------------------------
//
void CGui::comboParDust(Cmb cmb, size_t val)  // par type
{
	String s = cmb->getItemNameAt(val);
	String n = cmb->getName();
		 
		 if (n=="CmbParDust")   sc->sParDust = s;
	else if (n=="CmbParMud")    sc->sParMud = s;
	else if (n=="CmbParSmoke")  sc->sParSmoke = s;
}


///  Terrain Surface  -----------------------------
//
void CGui::comboSurface(Cmb cmb, size_t val)
{
	TerLayer* l = GetTerRdLay();
	//std::string s = cmb->getItemNameAt(val);
	std::string s = cmbSurface->getCaption();
	LogO("SURF cmb  = "+s);
	l->surfName = s;
	UpdSurfInfo();
}

void CGui::UpdSurfInfo()
{
	TerLayer* l = GetTerRdLay();
	int id = app->surf_map[l->surfName]-1;
	if (id == -1)  return;  //not found..
	const TRACKSURFACE& su = app->surfaces[id];

	txtSurfTire->setCaption(su.tireName);
	txtSuBumpWave->setCaption(fToStr(su.bumpWaveLength, 1,3));
	txtSuBumpAmp->setCaption(fToStr(su.bumpAmplitude, 2,4));
	txtSuRollDrag->setCaption(fToStr(su.rollingDrag, 1,3));
	txtSuFrict->setCaption(fToStr(su.friction, 2,4));
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
	
	for (auto section : sectionlist)
	{
		TRACKSURFACE surf;
		surf.name = section;
		
		int id;
		param.GetParam(section + ".ID", id);  // for sound..
		//-assert(indexnum >= 0 && indexnum < (int)tracksurfaces.size());
		surf.setType(id);
		
		float temp = 0.0;
		param.GetParam(section + ".BumpWaveLength", temp);		surf.bumpWaveLength = temp;
		param.GetParam(section + ".BumpAmplitude", temp);		surf.bumpAmplitude = temp;

		//frictionX, frictionY, bumpWaveLength2, bumpAmplitude2, not shown ..
		param.GetParam(section + ".FrictionTread", temp);		surf.friction = temp;
		
		if (param.GetParam(section + ".RollResistance", temp))	surf.rollingResist = temp;
		param.GetParam(section + ".RollingDrag", temp);		surf.rollingDrag = temp;

		///---  Tire  ---
		std::string tireFile;
		if (!param.GetParam(section + "." + "Tire", tireFile))
			tireFile = "Default";  // default surface if not found
		surf.tireName = tireFile;
		///---
		
		surfaces.push_back(surf);
		surf_map[surf.name] = (int)surfaces.size();  //+1, 0 = not found
	}

	return true;
}
