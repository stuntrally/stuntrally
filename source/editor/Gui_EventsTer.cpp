#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "settings.h"
#include "CGui.h"
#include "CApp.h"
#include "../ogre/common/GuiCom.h"
#include "../road/Road.h"
#include "../ogre/common/Slider.h"
#include "../vdrift/pathmanager.h"
#include <fstream>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;


///  [Terrain]
//-----------------------------------------------------------------------------------------------------------

///  Change terrain texture layer, update values
//
void CGui::tabTerLayer(Tab wp, size_t id)
{
	idTerLay = id;  // help vars
	bTerLay = id < sc->td.ciNumLay;
	TerLayer* lay = bTerLay ? &sc->td.layersAll[idTerLay] : &sc->td.layerRoad;

	noBlendUpd = true;
	SldUpd_TerL();

	cmbTexDiff->setVisible(bTerLay);  cmbTexNorm->setVisible(bTerLay);
	ckTerLayOn.setVisible(bTerLay);   ckTexNormAuto.setVisible(bTerLay);  ckTerLayTripl.setVisible(bTerLay);
	imgTexDiff->setVisible(bTerLay);
	
	if (bTerLay)
	{
		cmbTexDiff->setIndexSelected( cmbTexDiff->findItemIndexWith(lay->texFile) );
		cmbTexNorm->setIndexSelected( cmbTexNorm->findItemIndexWith(lay->texNorm) );
		bool noNorm = cmbTexNorm->getIndexSelected() == ITEM_NONE;

		//  auto norm check  if  norm = tex + _nh
		String sTex,sNorm, sExt;
		StringUtil::splitBaseFilename(lay->texFile,sTex,sExt);
		StringUtil::splitBaseFilename(lay->texNorm,sNorm,sExt);
		bool bAuto = !sNorm.empty() && !noNorm && (sTex + "_nh" == sNorm);
		bTexNormAuto = bAuto;
		ckTexNormAuto.Upd();
		//  tex image
	    imgTexDiff->setImageTexture(sTex + "_prv.jpg");
	}

	//  scale layer
	svTerLScale.setVisible(bTerLay);
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

void CGui::SldUpd_TerL()
{
	TerLayer* lay = bTerLay ? &sc->td.layersAll[idTerLay] : &sc->td.layerRoad;
	ckTerLayOn.Upd(&lay->on);
	ckTerLNoiseOnly.Upd(&lay->bNoiseOnly);
	ckTerLayTripl.Upd(&lay->triplanar);
	
	svTerLScale.UpdF(&lay->tiling);
	svTerLAngMin.UpdF(&lay->angMin);  svTerLHMin.UpdF(&lay->hMin);
	svTerLAngMax.UpdF(&lay->angMax);  svTerLHMax.UpdF(&lay->hMax);
	svTerLAngSm.UpdF(&lay->angSm);    svTerLHSm.UpdF(&lay->hSm);
	svTerLNoise.UpdF(&lay->noise);
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

void CGui::editTerErrorNorm(Ed ed)
{
	Real r = std::max(0.f, s2r(ed->getCaption()) );
	sc->td.errorNorm = r;  app->UpdTerErr();
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
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], si * sizeof(float));
	of.close();
	delete[] hfData;
	
	app->road->SelAll();
	app->road->Move(Vector3(my,0,mx) * -sc->td.fTriangleSize);
	app->road->SelClear();
	//start, objects-

	app->bNewHmap = true;	app->UpdateTrack();
}

//  Terrain  height scale  --------------------------------
void CGui::btnScaleTerH(WP)
{
	if (!app->road)  return;
	Real sf = std::max(0.1f, fScaleTer);  // scale mul

	//  road
	for (int i=0; i < app->road->getNumPoints(); ++i)
		app->road->Scale1(i, 0.f, sf);
	app->road->bSelChng = true;
	
	//  fluids
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.y *= sf;  fb.size.y *= sf;
	}
	
	//  objs h
	for (int i=0; i < sc->objects.size(); ++i)
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
		for (int i=0; i < sc->td.iVertsX; ++i,++a)
			hfData[a] = sc->td.hfHeight[a] * sf;
	}
	std::ofstream of;
	of.open(getHMapNew().c_str(), std::ios_base::binary);
	of.write((const char*)&hfData[0], siz);
	of.close();

	delete[] hfData;
	app->bNewHmap = true;	app->UpdateTrack();

	//  road upd
	if (0) //road)  // doesnt work here..
	{	app->road->UpdPointsH();
		app->road->RebuildRoad(true);
	}

	//  start pos
	const int n = 0;  // 1st..
	app->vStartPos[n][2] *= sf;  app->UpdStartPos();
}
//----------------------------------------------------------------------------------------------------------


//  generator  . . . . . . .
void CGui::slTerGen(SV*)
{
	app->bUpdTerPrv = true;
}


///  Terrain layers  -----------------------------
//
void CGui::chkTerLayOn(Ck*)
{
	sc->td.UpdLayers();
	SetUsedStr(valTerLAll, sc->td.layers.size(), 3);
	//  force update, blendmap sliders crash if not, !! this doesnt save hmap if changed  todo..
	app->UpdateTrack();
}

void CGui::chkTerLayTripl(Ck*)
{
	sc->td.UpdLayers();
}

void CGui::comboTexDiff(Cmb cmb, size_t val)
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
		if (bTerLay)  sc->td.layersAll[idTerLay].texNorm = sNorm;
	}
	//  tex image
    imgTexDiff->setImageTexture(sTex + "_prv.jpg");
}

void CGui::comboTexNorm(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	if (bTerLay)  sc->td.layersAll[idTerLay].texNorm = s;
}

//  Terrain BlendMap
void CGui::slTerLay(SV*)
{
	app->bTerUpdBlend = true;
}

void CGui::chkTerLNoiseOnly(Ck*)
{
	if (/*app->terrain &&*/ bGI /*&& !noBlendUpd*/)  app->bTerUpdBlend = true;
}


///  Terrain Particles  -----------------------------
//
void CGui::editLDust(Ed ed)
{
	Real r = s2r(ed->getCaption());
	TerLayer* l = !bTerLay ? &sc->td.layerRoad : &sc->td.layersAll[idTerLay];
	String n = ed->getName();

		 if (n=="LDust")   l->dust = r;		else if (n=="LDustS")  l->dustS = r;
	else if (n=="LMud")    l->mud = r;		else if (n=="LSmoke")  l->smoke = r;
}
void CGui::editLTrlClr(Ed ed)
{
	ColourValue c = s2c(ed->getCaption());
	if (!bTerLay)   sc->td.layerRoad.tclr = c;
	else  sc->td.layersAll[idTerLay].tclr = c;
	if (clrTrail)  clrTrail->setColour(Colour(c.r,c.g,c.b));
}

void CGui::comboParDust(Cmb cmb, size_t val)
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
	std::string s = cmb->getItemNameAt(val);
	if (!bTerLay)
		sc->td.layerRoad.surfName = s;
	else
		sc->td.layersAll[idTerLay].surfName = s;
	UpdSurfInfo();
}

void CGui::UpdSurfInfo()
{
	std::string s = cmbSurface->getCaption();
	int id = app->surf_map[s]-1;
	if (id == -1)  return;  //not found..
	const TRACKSURFACE& su = app->surfaces[id];

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
