#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../ogre/common/MessageBox/MessageBox.h"
using namespace MyGUI;
using namespace Ogre;


///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  track files  for copy, new 
const int cnTrkFm = 3, cnTrkFo = 1, cnTrkFp = 3;
const Ogre::String
	csTrkFm[cnTrkFm] = {"/heightmap.f32", "/road.xml", "/scene.xml"},  // in / 
	csTrkFo[cnTrkFo] = {"/waterDepth.png"},							// in /objects/
	csTrkFp[cnTrkFp] = {"/view.jpg", "/road.png", "/terrain.jpg"};	// in /preview/


void CGui::btnTrkCopySel(WP)  // set copy source
{
	sCopyTrack = gcom->sListTrack;
	bCopyTrackU = gcom->bListTrackU;
	if (valTrkCpySel)  valTrkCpySel->setCaption(sCopyTrack);
}

bool CGui::ChkTrkCopy()
{
	if (sCopyTrack == "")  // none
	{
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Copy}"), TR("#{CopyTrackNoSource}"),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	if (sCopyTrack == pSet->gui.track && bCopyTrackU == (pSet->gui.track_user ? 1 : 0))
	{
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Copy}"), TR("#{CopyTrackSourceSame}"),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	return true;
}

///  copy Hmap
void CGui::btnCopyTerHmap(WP)
{
	if (!ChkTrkCopy())  return;

	String from = PathCopyTrk(),
		name = gcom->TrkDir() + "heightmap-new.f32";
	Copy(from + "/heightmap.f32", name);
	
	Scene sF;  sF.LoadXml(from + "/scene.xml");
	sc->td.iVertsX = sF.td.iTerSize;  //  ter sizes
	sc->td.fTriangleSize = sF.td.fTriangleSize;
	sc->td.UpdVals();
	app->bNewHmap = true;
	SetGuiFromXmls();	app->UpdateTrack();
	if (scn->road)  scn->road->UpdAllMarkers();
}

//  copy Sun, etc.  can you copy a star
void CGui::btnCopySun(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->skyMtr = sF.skyMtr;  // sky
	sc->skyYaw = sF.skyYaw;
	sc->rainEmit = sF.rainEmit;  sc->rain2Emit = sF.rain2Emit;
	sc->rainName = sF.rainName;  sc->rain2Name = sF.rain2Name;

	sc->fogClr = sF.fogClr;  sc->fogClr2 = sF.fogClr2;  sc->fogClrH = sF.fogClrH;
	sc->fogStart = sF.fogStart;  sc->fogEnd = sF.fogEnd;
	sc->fogHStart = sF.fogHStart;  sc->fogHEnd = sF.fogHEnd;
	sc->fogHDensity = sF.fogHDensity;  sc->fogHeight = sF.fogHeight;

	sc->ldPitch = sF.ldPitch;  sc->ldYaw = sF.ldYaw;  // light
	sc->lAmb = sF.lAmb;  sc->lDiff = sF.lDiff;  sc->lSpec = sF.lSpec;

	SetGuiFromXmls();	app->UpdateTrack();
	scn->DestroyWeather();  scn->CreateWeather();
}

//  copy Ter layers
void CGui::btnCopyTerLayers(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	for (int i=0; i < sc->td.ciNumLay; ++i)
		sc->td.layersAll[i] = sF.td.layersAll[i];
	sc->sParDust = sF.sParDust;  sc->sParMud = sF.sParMud;
	sc->sParSmoke = sF.sParSmoke;
	sc->td.UpdLayers();

	SetGuiFromXmls();	app->UpdateTrack();
}

//  copy Veget
void CGui::btnCopyVeget(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->densGrass = sF.densGrass;  sc->densTrees = sF.densTrees;
	sc->trPage = sF.trPage;  sc->trDist = sF.trDist;
	sc->trRdDist = sF.trRdDist;  sc->trDistImp = sF.trDistImp;

	sc->grPage = sF.grPage;  sc->grDist = sF.grDist;

	for (int i=0; i < sc->ciNumGrLay; ++i)
		sc->grLayersAll[i] = sF.grLayersAll[i];

	for (int i=0; i < sc->ciNumPgLay; ++i)
		sc->pgLayersAll[i] = sF.pgLayersAll[i];

	SetGuiFromXmls();	app->UpdateTrack();
}

//  copy Road
void CGui::btnCopyRoad(WP)
{
	if (!ChkTrkCopy() || !scn->road)  return;
	String from = PathCopyTrk();
	scn->road->LoadFile(from + "/road.xml");  // todo: other roads cmb?

	SetGuiFromXmls();	scn->road->Rebuild(true);
	scn->UpdPSSMMaterials();	scn->road->UpdAllMarkers();
}

//  copy Road pars
void CGui::btnCopyRoadPars(WP)
{
	SplineRoad* r = scn->road;
	if (!ChkTrkCopy() || !r)  return;
	String from = PathCopyTrk();
	SplineRoad rd(app);  rd.LoadFile(from + "/road.xml",false);  // todo: other roads

	for (int i=0; i < MTRs; ++i)
	{	r->sMtrRoad[i] = rd.sMtrRoad[i];
		r->SetMtrPipe(i, rd.sMtrPipe[i]);
	}
	r->g_tcMul  = rd.g_tcMul;	r->g_tcMulW = rd.g_tcMulW;
	r->g_tcMulP = rd.g_tcMulP;	r->g_tcMulPW= rd.g_tcMulPW;
	r->g_tcMulC = rd.g_tcMulC;
	r->g_LenDim0 = rd.g_LenDim0;    r->g_iWidthDiv0 = rd.g_iWidthDiv0;
	r->g_ColNSides = rd.g_ColNSides;  r->g_ColRadius = rd.g_ColRadius;
	r->g_P_iw_mul = rd.g_P_iw_mul;  r->g_P_il_mul = rd.g_P_il_mul;
	r->g_Height = rd.g_Height;
	r->g_SkirtLen = rd.g_SkirtLen;  r->g_SkirtH = rd.g_SkirtH;
	r->g_MergeLen = rd.g_MergeLen;  r->g_LodPntLen = rd.g_LodPntLen;
	r->g_VisDist = rd.g_VisDist;    r->g_VisBehind = rd.g_VisBehind;

	SetGuiFromXmls();
	scn->road->Rebuild(true);
	scn->UpdPSSMMaterials();	scn->road->UpdAllMarkers();
}


///  tools 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::btnDeleteRoad(WP)
{
	//scn->DestroyRoads();
	//scn->road = new

	int l = scn->road->getNumPoints();
	for (int i=0; i < l; ++i)
	{
		scn->road->iChosen = scn->road->getNumPoints()-1;
		scn->road->Delete();
	}
	//scn->road->Rebuild(true);
}
void CGui::btnDeleteFluids(WP)
{
	sc->fluids.clear();
	app->bRecreateFluids = true;
}
void CGui::btnDeleteObjects(WP)
{
	app->DestroyObjects(true);
	app->iObjCur = -1;
}
void CGui::btnDeleteParticles(WP)
{
	scn->DestroyEmitters(true);
}


//  Scale track  --------------------------------
void CGui::btnScaleAll(WP)
{
	if (!scn->road)  return;
	Real sf = std::max(0.1f, fScale);  // scale mul
	int i;
	
	//  roads
	for (auto& r : scn->roads)
	for (i=0; i < r->getNumPoints(); ++i)
	{
		r->Scale1(i, sf, 0.f);
		r->mP[i].width *= sf;
	}
	scn->road->bSelChng = true;
	
	//  fluids
	for (i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.x *= sf;  fb.pos.z *= sf;
		fb.size.x *= sf;  fb.size.z *= sf;
	}
	
	//  objs
	for (i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		o.pos[0] *= sf;  o.pos[1] *= sf;
		o.SetFromBlt();
	}

	//  ter  ---
	sc->td.fTriangleSize *= sf;  sc->td.UpdVals();
	
	SetGuiFromXmls();	app->UpdateTrack();
	
	//  road upd
	if (0) //road)  // doesnt work here..
	{	scn->road->UpdPointsH();
		scn->road->Rebuild(true);
	}

	//  start,end pos
	for (i=0; i < 2; ++i)
	{	scn->sc->startPos[i][0] *= sf;
		scn->sc->startPos[i][1] *= sf;  }
	app->UpdStartPos();
}


///  track 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//-----------------------------------------------------------------------------------------------------------

String CGui::PathCopyTrk(int user) {
	int u = user == -1 ? bCopyTrackU : user;	return gcom->pathTrk[u] + sCopyTrack;  }


///  New (duplicate)
void CGui::btnTrackNew(WP)
{
	String name = trkName->getCaption();
	name = StringUtil::replaceAll(name, "*", "");

	if (gcom->TrackExists(name))  {
		Message::createMessageBox(
			"Message", TR("#{Track} - #{NewDup}"), TR("#{Track}: ") + name + TR(" #{AlreadyExists}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	sc->baseTrk = gcom->sListTrack;
	String st = gcom->PathListTrk(),  t = gcom->pathTrk[1] + name,
		sto = st + "/objects", stp = st + "/preview",  // from
		to = t + "/objects",   tp = t + "/preview";  // to,new

	//  Copy
	CreateDir(t);  CreateDir(to);  CreateDir(tp);
	int i;
	for (i=0; i < cnTrkFm; ++i)  Copy(st + csTrkFm[i], t + csTrkFm[i]);  // todo: other roads too?
	for (i=0; i < cnTrkFo; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);
	for (i=1; i < cnTrkFp; ++i)  Copy(stp + csTrkFp[i], tp + csTrkFp[i]);  // 1-not view.jpg

	gcom->sListTrack = name;  pSet->gui.track = name;  pSet->gui.track_user = 1;
	app->UpdWndTitle();
	gcom->FillTrackLists();  gcom->TrackListUpd();
}

///  Rename
void CGui::btnTrackRename(WP)
{
	String name = trkName->getCaption();
	if (name == gcom->sListTrack)  return;

	if (!pSet->allow_save)  // could force when originals writable..
	if (gcom->bListTrackU==0)  {
		return;  }

	if (gcom->TrackExists(name))  {
		Message::createMessageBox(
			"Message", TR("#{Track} - #{Rename}"), TR("#{AlreadyExists}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }
	
	//  Rename
	Rename(gcom->PathListTrk(), gcom->pathTrk[/*1*/gcom->bListTrackU ? 1 : 0] + name);
	
	gcom->sListTrack = name;  pSet->gui.track = name;  pSet->gui.track_user = 1;/**/
	app->UpdWndTitle();
	gcom->FillTrackLists();  gcom->TrackListUpd();  //gcom->listTrackChng(trkList,0);
}

///  Delete
void CGui::btnTrackDel(WP)
{
	Message* message = Message::createMessageBox(
		"Message", TR("#{DeleteTrack}"), gcom->sListTrack,
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &CGui::msgTrackDel);
	//message->setUserString("FileName", fileName);
}
void CGui::msgTrackDel(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)
		return;
	String t = gcom->PathListTrk(),
		to = t + "/objects", tp = t + "/preview";
	int i;
	for (i=0; i < cnTrkFo; ++i)  Delete(to + csTrkFo[i]);  // todo: other roads! list whole dir..
	for (i=0; i < cnTrkFp; ++i)  Delete(tp + csTrkFp[i]);
	for (i=0; i < cnTrkFm; ++i)  Delete(t + csTrkFm[i]);
	Delete(t + "/heightmap-new.f32");
	DeleteDir(to);  DeleteDir(tp);  DeleteDir(t);

	String st = pSet->gui.track;
	gcom->FillTrackLists();
	gcom->TrackListUpd();
	if (st != pSet->gui.track)
		app->LoadTrack();  //load 1st if deleted cur
}
