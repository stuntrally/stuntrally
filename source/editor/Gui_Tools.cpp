#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../ogre/common/MessageBox/MessageBox.h"
using namespace MyGUI;
using namespace Ogre;


///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  track files
const int cnTrkFm = 4, cnTrkFd = 2, cnTrkFo = 2, cnTrkFp = 3;
const Ogre::String
	csTrkFm[cnTrkFm] = {"/heightmap.f32", "/road.xml", "/scene.xml", "/track.txt"},  // copy, new
	csTrkFo[cnTrkFo] = {"/grassDensity.png", "/waterDepth.png"},
	csTrkFp[cnTrkFp] = {"/view.jpg", "/road.png", "/terrain.jpg"},
	csTrkFd[cnTrkFd] = {"/heightmap-new.f32", "/records.txt"};  // del


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
	if (app->road)  app->road->UpdAllMarkers();
}

//  copy sun, etc.
void CGui::btnCopySun(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->skyMtr = sF.skyMtr;  // sky
	sc->rainEmit = sF.rainEmit;  sc->rain2Emit = sF.rain2Emit;
	sc->rainName = sF.rainName;  sc->rain2Name = sF.rain2Name;

	sc->fogClr = sF.fogClr;  sc->fogClr2 = sF.fogClr2;  sc->fogClrH = sF.fogClrH;
	sc->fogStart = sF.fogStart;  sc->fogEnd = sF.fogEnd;
	sc->fogHStart = sF.fogHStart;  sc->fogHEnd = sF.fogHEnd;
	sc->fogHDensity = sF.fogHDensity;  sc->fogHeight = sF.fogHeight;

	sc->ldPitch = sF.ldPitch;  sc->ldYaw = sF.ldYaw;  // light
	sc->lAmb = sF.lAmb;  sc->lDiff = sF.lDiff;  sc->lSpec = sF.lSpec;
	SetGuiFromXmls();	app->UpdateTrack();
	app->DestroyWeather();  app->CreateWeather();
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

	//  copy grass dens only
	String sto = from + "/objects";  // from
	String to = gcom->TrkDir() + "objects";  // to, new
	Copy(sto + csTrkFo[0], to + csTrkFo[0]);

	SetGuiFromXmls();	app->UpdateTrack();
}

//  copy paged layers
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
	if (!ChkTrkCopy() || !app->road)  return;
	String from = PathCopyTrk();
	app->road->LoadFile(from + "/road.xml");

	SetGuiFromXmls();	app->road->RebuildRoad(true);
	app->UpdPSSMMaterials();	app->road->UpdAllMarkers();
}

//  copy road pars
void CGui::btnCopyRoadPars(WP)
{
	if (!ChkTrkCopy() || !app->road)  return;
	String from = PathCopyTrk();
	SplineRoad rd(app);  rd.LoadFile(from + "/road.xml",false);

	for (int i=0; i < MTRs; ++i)
	{	app->road->sMtrRoad[i] = rd.sMtrRoad[i];
		app->road->SetMtrPipe(i, rd.sMtrPipe[i]);  }

	app->road->tcMul = rd.tcMul;		app->road->colN = rd.colN;
	app->road->lenDiv0 = rd.lenDiv0;	app->road->colR = rd.colR;
	app->road->iw0 =	rd.iw0;			app->road->iwPmul = rd.iwPmul;
	app->road->fHeight =	rd.fHeight;	app->road->ilPmul = rd.ilPmul;
	app->road->skirtLen = rd.skirtLen;	app->road->skirtH = rd.skirtH;
	app->road->setMrgLen = rd.setMrgLen;  app->road->lposLen = rd.lposLen;

	SetGuiFromXmls();	app->road->RebuildRoad(true);
	app->UpdPSSMMaterials();	app->road->UpdAllMarkers();
}


///  tools 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::btnDeleteRoad(WP)
{
	int l = app->road->getNumPoints();
	for (int i=0; i < l; ++i)
	{
		app->road->iChosen = app->road->getNumPoints()-1;
		app->road->Delete();
	}
	//app->road->RebuildRoad(true);
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
	if (!app->road)  return;
	Real sf = std::max(0.1f, fScale);  // scale mul
	
	//  road
	for (int i=0; i < app->road->getNumPoints(); ++i)
	{
		app->road->Scale1(i, sf, 0.f);
		app->road->mP[i].width *= sf;
	}
	app->road->bSelChng = true;
	
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
	{	app->road->UpdPointsH();
		app->road->RebuildRoad(true);
	}

	//  start pos
	const int n = 0;  // 1st entry - all same / edit 4..
	app->vStartPos[n][0] *= sf;
	app->vStartPos[n][1] *= sf;  app->UpdStartPos();
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
	for (i=0; i < cnTrkFd; ++i)  Delete(t + csTrkFd[i]);
	DeleteDir(to);  DeleteDir(tp);  DeleteDir(t);

	String st = pSet->gui.track;
	gcom->FillTrackLists();
	gcom->TrackListUpd();
	if (st != pSet->gui.track)
		app->LoadTrack();  //load 1st if deleted cur
}
