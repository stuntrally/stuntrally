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
			"Message", "Copy Track", "No source track selected.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	if (sCopyTrack == pSet->gui.track && bCopyTrackU == (pSet->gui.track_user ? 1 : 0))
	{
		Message::createMessageBox(
			"Message", "Copy Track", "Source track and current track are the same.",
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
	if (app->scn->road)  app->scn->road->UpdAllMarkers();
}

//  copy sun, etc.
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
	app->scn->DestroyWeather();  app->scn->CreateWeather();
}

//  copy ter layers
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

//  copy veget
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

//  copy road
void CGui::btnCopyRoad(WP)
{
	if (!ChkTrkCopy() || !app->scn->road)  return;
	String from = PathCopyTrk();
	app->scn->road->LoadFile(from + "/road.xml");

	SetGuiFromXmls();	app->scn->road->RebuildRoad(true);
	scn->UpdPSSMMaterials();	app->scn->road->UpdAllMarkers();
}

//  copy road pars
void CGui::btnCopyRoadPars(WP)
{
	if (!ChkTrkCopy() || !app->scn->road)  return;
	String from = PathCopyTrk();
	SplineRoad rd(app);  rd.LoadFile(from + "/road.xml",false);

	for (int i=0; i < MTRs; ++i)
	{	app->scn->road->sMtrRoad[i] = rd.sMtrRoad[i];
		app->scn->road->SetMtrPipe(i, rd.sMtrPipe[i]);  }

	app->scn->road->tcMul = rd.tcMul;		app->scn->road->colN = rd.colN;
	app->scn->road->lenDiv0 = rd.lenDiv0;	app->scn->road->colR = rd.colR;
	app->scn->road->iw0 =	rd.iw0;			app->scn->road->iwPmul = rd.iwPmul;
	app->scn->road->fHeight =	rd.fHeight;	app->scn->road->ilPmul = rd.ilPmul;
	app->scn->road->skirtLen = rd.skirtLen;	app->scn->road->skirtH = rd.skirtH;
	app->scn->road->setMrgLen = rd.setMrgLen;  app->scn->road->lposLen = rd.lposLen;

	SetGuiFromXmls();	app->scn->road->RebuildRoad(true);
	scn->UpdPSSMMaterials();	app->scn->road->UpdAllMarkers();
}


///  tools 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::btnDeleteRoad(WP)
{
	int l = app->scn->road->getNumPoints();
	for (int i=0; i < l; ++i)
	{
		app->scn->road->iChosen = app->scn->road->getNumPoints()-1;
		app->scn->road->Delete();
	}
	//app->scn->road->RebuildRoad(true);
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

//  Scale track  --------------------------------
void CGui::btnScaleAll(WP)
{
	if (!app->scn->road)  return;
	Real sf = std::max(0.1f, fScale);  // scale mul
	
	//  road
	for (int i=0; i < app->scn->road->getNumPoints(); ++i)
	{
		app->scn->road->Scale1(i, sf, 0.f);
		app->scn->road->mP[i].width *= sf;
	}
	app->scn->road->bSelChng = true;
	
	//  fluids
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.x *= sf;  fb.pos.z *= sf;
		fb.size.x *= sf;  fb.size.z *= sf;
	}
	
	//  objs
	for (int i=0; i < sc->objects.size(); ++i)
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
	{	app->scn->road->UpdPointsH();
		app->scn->road->RebuildRoad(true);
	}

	//  start pos
	app->scn->sc->startPos[0] *= sf;
	app->scn->sc->startPos[1] *= sf;  app->UpdStartPos();
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
			"Message", "New Track", "Track " + name + " already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	sc->baseTrk = gcom->sListTrack;
	String st = gcom->PathListTrk(),  t = gcom->pathTrk[1] + name,
		sto = st + "/objects", stp = st + "/preview",  // from
		to = t + "/objects",   tp = t + "/preview";  // to,new

	//  Copy
	CreateDir(t);  CreateDir(to);  CreateDir(tp);
	int i;
	for (i=0; i < cnTrkFm; ++i)  Copy(st + csTrkFm[i], t + csTrkFm[i]);
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

	/*if (bListTrackU==0)  {  // could force when originals writable..
		Message::createMessageBox(
			"Message", "Rename Track", "Track " + name + " is original and can't be renamed.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;  }/**/

	if (gcom->TrackExists(name))  {
		Message::createMessageBox(
			"Message", "Rename Track", "Track " + name + " already exists.",
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
		"Message", gcom->bListTrackU==0 ? "Delete original Track ?" : "Delete Track ?", gcom->sListTrack,
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
	for (i=0; i < cnTrkFo; ++i)  Delete(to + csTrkFo[i]);
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
