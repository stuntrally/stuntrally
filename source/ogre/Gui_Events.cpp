#include "pch.h"
#include "Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "SplitScreen.h"
#include "common/Gui_Def.h"
#include "common/RenderConst.h"

#include <MyGUI_PointerManager.h>
#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreMaterialManager.h>
#include <OgreOverlay.h>
#include <OgreRenderWindow.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//  [Multiplayer]
//---------------------------------------------------------------------

namespace {
	std::string yesno(bool cond) {
		if (cond) return TR("#{Yes}");
		else return TR("#{No}");
	}

	void inline raiseError(const std::string& what, const std::string& title = TR("#{Error}")) {
		Message::createMessageBox("Message", title, what, MessageBoxStyle::IconError | MessageBoxStyle::Ok);
	}
}


void App::rebuildGameList() {
	if (!listServers || !mMasterClient) return;
	protocol::GameList list = mMasterClient->getList();
	listServers->removeAllItems();
	for (protocol::GameList::const_iterator it = list.begin(); it != list.end(); ++it) {
		listServers->addItem(it->second.name);
		int l = listServers->getItemCount()-1;
		listServers->setSubItemNameAt(1, l, std::string(it->second.track));
		listServers->setSubItemNameAt(2, l, boost::lexical_cast<std::string>((int)it->second.players));
		listServers->setSubItemNameAt(3, l, yesno((bool)it->second.collisions));
		listServers->setSubItemNameAt(4, l, yesno((bool)it->second.locked));
		listServers->setSubItemNameAt(5, l, net::IPv4(it->second.address));
		listServers->setSubItemNameAt(6, l, boost::lexical_cast<std::string>((int)it->second.port));
	}
}

void App::rebuildPlayerList()
{
	if (!listPlayers || !mClient) return;
	listPlayers->removeAllItems();
	// Add self
	unsigned peerCount = mClient->getPeerCount();
	listPlayers->addItem(pSet->nickname);
	listPlayers->setSubItemNameAt(1, 0, sListCar); // Car
	listPlayers->setSubItemNameAt(2, 0, boost::lexical_cast<std::string>(peerCount)); // Peers
	listPlayers->setSubItemNameAt(3, 0, "0"); // Ping
	listPlayers->setSubItemNameAt(4, 0, yesno(mClient->isReady())); // Ready state
	// Add others
	bool allReady = true;
	const PeerMap peers = mClient->getPeers();
	for (PeerMap::const_iterator it = peers.begin(); it != peers.end(); ++it) {
		if (it->second.name.empty() || it->second.connection == PeerInfo::DISCONNECTED)
			continue;
		// Determine if everyone is ready and connected
		if (it->second.peers != peerCount || !it->second.ready) allReady = false;
		// Add list item
		listPlayers->addItem(it->second.name);
		int l = listPlayers->getItemCount()-1;
		listPlayers->setSubItemNameAt(1, l, it->second.car);
		listPlayers->setSubItemNameAt(2, l, boost::lexical_cast<std::string>(it->second.peers));
		listPlayers->setSubItemNameAt(3, l, boost::lexical_cast<std::string>(it->second.ping));
		listPlayers->setSubItemNameAt(4, l, yesno(it->second.ready));
	}
	// Allow host to start the game
	if (mLobbyState == HOSTING) {
		if (allReady) btnNetReady->setEnabled(true);
		else btnNetReady->setEnabled(false);
	}
}

void App::updateGameInfo()
{
	if (netGameInfo.name && edNetGameName) {
		std::string name(netGameInfo.name);
		edNetGameName->setCaption(name);
	}
	if (netGameInfo.track) {
		std::string track(netGameInfo.track);
		sListTrack = track;
		ReadTrkStats();
	}
	// FIXME: These should not modify global settings, only for one game
	pSet->collis_cars = netGameInfo.collisions;
	pSet->num_laps = netGameInfo.laps;
	updateGameInfoGUI();
}

void App::updateGameInfoGUI()
{
	//  update track info
	if (valNetTrack)
		valNetTrack->setCaption("Track: " + sListTrack);
	if (imgNetTrack)
		imgNetTrack->setImageTexture(sListTrack+".jpg");
	if (edNetTrackInfo && trkDesc)
		edNetTrackInfo->setCaption(trkDesc->getCaption());
}

void App::uploadGameInfo()
{
	if (!mMasterClient || !mClient || !edNetGameName || !pSet)
		return;
	protocol::GameInfo game;
	std::string gamename = edNetGameName->getCaption();
	std::string trackname = sListTrack;
	std::memcpy(game.name, gamename.c_str(), 32);
	std::memcpy(game.track, trackname.c_str(), 32);
	game.players = mClient->getPeerCount()+1;
	game.collisions = pSet->collis_cars;
	game.laps = pSet->num_laps;
	game.port = pSet->local_port;
	game.locked = false;
	mMasterClient->updateGame(game); // Upload to master server
	if (mClient) // Send to peers
		mClient->broadcastGameInfo(game);
}

void App::setNetGuiHosting(bool enabled)
{
	edNetGameName->setEnabled(enabled);
	btnNetReady->setEnabled(!enabled);
	btnNetReady->setCaption(enabled ? TR("#{NetStart}") : TR("#{NetReady}"));
}

void App::gameListChanged(protocol::GameList list)
{
	(void)list;
	boost::mutex::scoped_lock lock(netGuiMutex);
	bRebuildGameList = true;
}

void App::peerConnected(PeerInfo peer)
{
	// Master server player count update
	if (mLobbyState == HOSTING) uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	sChatBuffer = sChatBuffer + "Connected: " + peer.name + "\n";
	bRebuildPlayerList = true;
}

void App::peerDisconnected(PeerInfo peer)
{
	if (peer.name.empty()) return;
	// Master server player count update
	if (mLobbyState == HOSTING) uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	sChatBuffer = sChatBuffer + "Disconnected: " + peer.name + "\n";
	bRebuildPlayerList = true;
}

void App::peerInfo(PeerInfo peer)
{
	(void)peer;
	boost::mutex::scoped_lock lock(netGuiMutex);
	bRebuildPlayerList = true;
}

void App::peerMessage(PeerInfo peer, std::string msg)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	sChatBuffer = sChatBuffer + peer.name + ": " + msg + "\n";
	bRebuildPlayerList = true; // For ping updates in the list
}

void App::peerState(PeerInfo peer, uint8_t state)
{
	(void)peer;
	boost::mutex::scoped_lock lock(netGuiMutex);
	if (state == protocol::START_GAME) bStartGame = true;
}

void App::gameInfo(protocol::GameInfo game)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	netGameInfo = game;
	bUpdateGameInfo = true;
}

void App::error(string what)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	sChatBuffer = sChatBuffer + "ERROR! " + what + "\n";
}

void App::join(std::string host, std::string port)
{
	try {
		mClient.reset(new P2PGameClient(this, pSet->local_port));
		mClient->updatePlayerInfo(pSet->nickname, sListCar);
		mClient->connect(host, boost::lexical_cast<int>(port)); // Lobby phase started automatically
		boost::mutex::scoped_lock lock(netGuiMutex);
		sChatBuffer = TR("Connecting to ") + host + ":" + port + "\n";
	} catch (...) {
		raiseError(TR("Failed to initialize networking.\nTry different local port and make sure your firewall is properly configured."), TR("Network Error"));
		return;
	}

	updateGameInfoGUI();
	rebuildPlayerList();
	setNetGuiHosting(false);
	tabsNet->setIndexSelected(1);
	panelNetServer->setVisible(true);
	panelNetGame->setVisible(false);
	panelNetTrack->setVisible(true);

}

void App::evBtnNetRefresh(WP)
{
	mMasterClient.reset(new MasterClient(this));
	mMasterClient->connect(pSet->master_server_address, pSet->master_server_port);
	// The actual refresh will be requested automatically when the connection is made
}

void App::evBtnNetJoin(WP)
{
	//  join selected game
	if (!listServers || !pSet) return;

	size_t i = listServers->getIndexSelected();
	if (i == ITEM_NONE) return;

	std::string host = listServers->getSubItemNameAt(5, i);
	std::string port = listServers->getSubItemNameAt(6, i);
	join(host, port);
}

void App::evBtnNetCreate(WP)
{
	//  create game ..
	if (mLobbyState == DISCONNECTED) {
		try {
			if (pSet) mClient.reset(new P2PGameClient(this, pSet->local_port));
			mClient->updatePlayerInfo(pSet->nickname, sListCar);
			mClient->startLobby();
		} catch (...) {
			raiseError(TR("Failed to initialize networking.\nTry different local port and make sure your firewall is properly configured."), TR("Network Error"));
			return;
		}
		mLobbyState = HOSTING;
		if (!mMasterClient) {
			mMasterClient.reset(new MasterClient(this));
			mMasterClient->connect(pSet->master_server_address, pSet->master_server_port);
		}
		uploadGameInfo();
		updateGameInfoGUI();
		rebuildPlayerList();
		setNetGuiHosting(true);
		tabsNet->setIndexSelected(1);
		panelNetServer->setVisible(true);
		panelNetGame->setVisible(false);
		panelNetTrack->setVisible(false);
		boost::mutex::scoped_lock lock(netGuiMutex);
		sChatBuffer = TR("Listening on port ")  + boost::lexical_cast<std::string>(pSet->local_port) + "...\n";
	}
}

void App::evBtnNetLeave(WP)
{
	//  leave current game
	mLobbyState = DISCONNECTED;
	mClient.reset();
	mMasterClient.reset();
	setNetGuiHosting(false);
	tabsNet->setIndexSelected(0);
	panelNetServer->setVisible(false);
	panelNetGame->setVisible(true);
	panelNetTrack->setVisible(false);
}

void App::evBtnNetDirect(WP)
{
	popup.Show(newDelegate(this, &App::evBtnNetDirectClose),
		TR("#{NetDirectConnect}"), true,
		TR("#{NetAddress}"), TR("#{NetPort}"), "", "",
		"localhost", toStr(protocol::DEFAULT_PORT), "","",
		TR("#{MessageBox_Ok}"),	TR("#{MessageBox_Cancel}"), "", "");
}

void App::evBtnNetDirectClose()
{
	popup.Hide();
	if (popup.btnResult != 0)  return;
	join(popup.edit0, popup.edit1);  // host, port
}

void App::evBtnNetReady(WP)
{
	if (!mClient) return;

	mClient->toggleReady();
	if (mClient->isReady()) {
		if (mLobbyState == HOSTING) {
			boost::mutex::scoped_lock lock(netGuiMutex);
			bStartGame = true;
		} else btnNetReady->setCaption( TR("#{NetWaiting}") );
	} else
		btnNetReady->setCaption( TR("#{NetReady}") );

	rebuildPlayerList();
}


	// info texts
	//valNetGames
	//valNetChat

void App::chatSendMsg()
{
	/*  test  *
	if (!edNetChat) return;
	edNetChat->setCaption(edNetChat->getCaption()+ pSet->nickname + ": " + edNetChatMsg->getCaption() + "\n");
	/**/
	if (!mClient || !edNetChatMsg)  return;
	if (edNetChatMsg->getCaption().empty()) return;

	mClient->sendMessage(edNetChatMsg->getCaption());
	edNetChatMsg->setCaption("");
}

void App::evEdNetGameName(EditPtr ed)
{
	// game name text changed
	if (mLobbyState != HOSTING || !mMasterClient || !mClient) return;
	uploadGameInfo();
}

//  net settings

void App::evEdNetNick(EditPtr ed)
{
	pSet->nickname = ed->getCaption();
	if (mClient) mClient->updatePlayerInfo(pSet->nickname, sListCar);
}

void App::evEdNetServerIP(EditPtr ed)
{
	pSet->master_server_address = ed->getCaption();
}

void App::evEdNetServerPort(EditPtr ed)
{
	pSet->master_server_port = s2i(ed->getCaption());
}

void App::evEdNetLocalPort(EditPtr ed)
{
	pSet->local_port = s2i(ed->getCaption());
}

///--------------------------------------------------------------------------------------------------------------------------------

#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
	chk->setStateSelected(pSet->var);  }


//  [Setup]
//    [Car]
void App::chkAbs(WP wp){		ChkEv(abs);		if (pGame)  pGame->ProcessNewSettings();	}
void App::chkTcs(WP wp){		ChkEv(tcs);		if (pGame)  pGame->ProcessNewSettings();	}

void App::chkGear(WP wp){		ChkEv(autoshift);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRear(WP wp){		ChkEv(autorear);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRearInv(WP wp){	ChkEv(rear_inv);	if (pGame)  pGame->ProcessNewSettings();	}
//    [Game]
void App::chkVegetCollis(WP wp){	ChkEv(collis_veget);	}
void App::chkCarCollis(WP wp){		ChkEv(collis_cars);		}

//  boost, flip
void App::comboBoost(CMB)
{
	pSet->boost_type = val;  ShowHUD();
}
void App::comboFlip(CMB)
{
	pSet->flip_type = val;
}
	
void App::btnNumPlayers(WP wp)
{
	if      (wp->getName() == "btnPlayers1")  pSet->local_players = 1;
	else if (wp->getName() == "btnPlayers2")  pSet->local_players = 2;
	else if (wp->getName() == "btnPlayers3")  pSet->local_players = 3;
	else if (wp->getName() == "btnPlayers4")  pSet->local_players = 4;
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->local_players));
}
void App::chkSplitVert(WP wp)
{
	ChkEv(split_vertically); 
}

void App::slNumLaps(SL)
{
	int v = 20.f * val/res + 1;  if (bGI)  pSet->num_laps = v;
	if (valNumLaps){  Fmt(s, "%d", v);	valNumLaps->setCaption(s);  }
}

void App::tabPlayer(TabPtr wp, size_t id)
{
	iCurCar = id;
	//  update gui for this car (color h,s,v, name, img)
	size_t i = carList->findItemIndexWith(pSet->car[iCurCar]);
	if (i != ITEM_NONE)
	{	carList->setIndexSelected(i);
		listCarChng(carList, i);
	}
	UpdCarClrSld(false);  // no car color change
}

//  car color
void App::slCarClrH(SL)
{
	Real v = val/res;  if (bGI)  pSet->car_hue[iCurCar] = v;
	if (valCarClrH){	Fmt(s, "%4.2f", v);	valCarClrH->setCaption(s);  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrS(SL)
{
	Real v = val/res;  if (bGI)  pSet->car_sat[iCurCar] = v;
	if (valCarClrS){	Fmt(s, "%4.2f", v);	valCarClrS->setCaption(s);  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrV(SL)
{
	Real v = val/res;  if (bGI)  pSet->car_val[iCurCar] = v;
	if (valCarClrV){	Fmt(s, "%4.2f", v);	valCarClrV->setCaption(s);  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}

void App::imgBtnCarClr(WP img)
{
	pSet->car_hue[iCurCar] = s2r(img->getUserString("h"));
	pSet->car_sat[iCurCar] = s2r(img->getUserString("s"));
	pSet->car_val[iCurCar] = s2r(img->getUserString("v"));
	UpdCarClrSld();
}
void App::btnCarClrRandom(WP)
{
	pSet->car_hue[iCurCar] = Math::UnitRandom();
	pSet->car_sat[iCurCar] = Math::UnitRandom();
	pSet->car_val[iCurCar] = Math::UnitRandom();
	UpdCarClrSld();
}


//  [Graphics]
//---------------------------------------------------------------------

//  particles/trails
void App::slParticles(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->particles_len = v;
	if (valParticles){	Fmt(s, "%4.2f", v);	valParticles->setCaption(s);  }
}
void App::slTrails(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->trails_len = v;
	if (valTrails){	Fmt(s, "%4.2f", v);	valTrails->setCaption(s);  }
}

//  reflect
void App::slReflSkip(SL)
{
	int v = 1000.f * powf(val/res, 2.f);	if (bGI)  pSet->refl_skip = v;
	if (valReflSkip)  valReflSkip->setCaption(toStr(v));
}
void App::slReflSize(SL)
{
	int v = std::max( 0.0f, std::min((float) ciShadowNumSizes-1, ciShadowNumSizes * val/res));
	if (bGI)  pSet->refl_size = v;
	if (valReflSize)  valReflSize->setCaption(toStr(ciShadowSizesA[v]));
}
void App::slReflFaces(SL)
{
	if (bGI)  pSet->refl_faces = val;
	if (valReflFaces)  valReflFaces->setCaption(toStr(val));
}
void App::slReflDist(SL)
{
	float v = 20.f + 1480.f * powf(val/res, 2.f);	if (bGI)  pSet->refl_dist = v;
	if (valReflDist){	Fmt(s, "%4.0f m", v);	valReflDist->setCaption(s);  }
}
void App::slReflMode(SL)
{
	std::string old = pSet->refl_mode;
	
	if (val == 0)  pSet->refl_mode = "static";  //enums..
	if (val == 1)  pSet->refl_mode = "single";
	if (val == 2)  pSet->refl_mode = "full";
	
	if (pSet->refl_mode != old)
		recreateReflections();
		
	if (valReflMode)
	{
		valReflMode->setCaption( TR("#{ReflMode_" + pSet->refl_mode + "}") );
		if (pSet->refl_mode == "static")  valReflMode->setTextColour(MyGUI::Colour(0.0, 1.0, 0.0)); 
		else if (pSet->refl_mode == "single")  valReflMode->setTextColour(MyGUI::Colour(1.0, 0.5, 0.0));
		else if (pSet->refl_mode == "full")  valReflMode->setTextColour(MyGUI::Colour(1.0, 0.0, 0.0));
	}
}
void App::recreateReflections()
{
	for (std::vector<CarModel*>::iterator it = carModels.begin(); it!=carModels.end(); it++)
	{	
		delete (*it)->pReflect;
		(*it)->CreateReflection();
	}
}


//  [View] size
void App::slSizeGaug(SL)
{
	float v = 0.1f + 0.15f * val/res;	if (bGI)  {  pSet->size_gauges = v;  SizeHUD(true);  }
	if (valSizeGaug){	Fmt(s, "%4.3f", v);	valSizeGaug->setCaption(s);  }
}
void App::slSizeArrow(SL)
{
	float v = val/res;	if (bGI)  {  pSet->size_arrow = v;  }
	if (valSizeArrow){	Fmt(s, "%4.3f", v);	valSizeArrow->setCaption(s);  }
	if (arrowNode) arrowRotNode->setScale(v/2.f, v/2.f, v/2.f);
}
//  minimap
void App::slSizeMinimap(SL)
{
	float v = 0.05f + 0.25f * val/res;	if (bGI)  {  pSet->size_minimap = v;  SizeHUD(true);  }
	if (valSizeMinimap){	Fmt(s, "%4.3f", v);	valSizeMinimap->setCaption(s);  }
}
void App::slZoomMinimap(SL)
{
	float v = 1.f + 9.f * powf(val/res, 2.f);	if (bGI)  {  pSet->zoom_minimap = v;  SizeHUD(true);  }
	if (valZoomMinimap){	Fmt(s, "%4.3f", v);	valZoomMinimap->setCaption(s);  }
}


//  [Sound]
void App::slVolMaster(SL)
{
	Real v = 1.6f * val/res;	if (bGI)  {  pSet->vol_master = v;  pGame->ProcessNewSettings();  }
	if (valVolMaster){  Fmt(s, "%4.2f", v);	valVolMaster->setCaption(s);  }
}
void App::slVolEngine(SL)
{
	Real v = 1.4f * val/res;	if (bGI)  pSet->vol_engine = v;
	if (valVolEngine){  Fmt(s, "%4.2f", v);	valVolEngine->setCaption(s);  }
}
void App::slVolTires(SL)
{
	Real v = 1.4f * val/res;	if (bGI)  pSet->vol_tires = v;
	if (valVolTires){  Fmt(s, "%4.2f", v);	valVolTires->setCaption(s);  }
}
void App::slVolEnv(SL)
{
	Real v = 1.4f * val/res;	if (bGI)  pSet->vol_env = v;
	if (valVolEnv){  Fmt(s, "%4.2f", v);	valVolEnv->setCaption(s);  }
}


//  [Game] 	. . . . . . . . . . . . . . . . . . . .    --- lists ----    . . . . . . . . . . . . . . . . . . . .

//  car
void App::listCarChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i);	sListCar = sl;

	if (imgCar)  imgCar->setImageTexture(sListCar+".jpg");
	if (mClient) mClient->updatePlayerInfo(pSet->nickname, sListCar);
}
void App::btnChgCar(WP)
{
	if (valCar){  valCar->setCaption(TR("#{Car}: ") + sListCar);	pSet->car[iCurCar] = sListCar;  }
}

//  track
void App::btnChgTrack(WP)
{
	pSet->track = sListTrack;
	pSet->track_user = bListTrackU;
	if (valTrk)  valTrk->setCaption(TR("#{Track}: ") + sListTrack);

	if (mMasterClient) {
		uploadGameInfo();
		updateGameInfoGUI();
	}
}

//  new game
void App::btnNewGame(WP)
{
	NewGame();  isFocGui = false;  // off gui
	if (mWndOpts)  mWndOpts->setVisible(isFocGui);
	if (mWndRpl)  mWndRpl->setVisible(false);//
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	PointerManager::getInstance().setVisible(isFocGuiOrRpl());
	mToolTip->setVisible(false);
}
void App::btnNewGameStart(WP wp)
{
	btnChgTrack(wp);
	btnNewGame(wp);
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

void App::chkDigits(WP wp){ 		ChkEv(show_digits); ShowHUD();   }

void App::chkReverse(WP wp){		ChkEv(trackreverse);	ReadTrkStats();  }

void App::chkParticles(WP wp)
{		
	ChkEv(particles);
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	//? if ((*it)->eType != CarModel::CT_GHOST)
		(*it)->UpdParsTrails();
}
void App::chkTrails(WP wp)
{			
	ChkEv(trails);		
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void App::chkFps(WP wp){			ChkEv(show_fps);	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();	}

void App::chkGauges(WP wp){			ChkEv(show_gauges);	ShowHUD();	}
void App::chkArrow(WP wp){			ChkEv(check_arrow); if (arrowRotNode) arrowRotNode->setVisible(pSet->check_arrow);  }
void App::chkMinimap(WP wp){		ChkEv(trackmap);	if (ndMap)  ndMap->setVisible(pSet->trackmap);	}
void App::chkMiniZoom(WP wp){		ChkEv(mini_zoomed);		}
void App::chkMiniRot(WP wp){		ChkEv(mini_rotated);	}
void App::chkMiniTer(WP wp){		ChkEv(mini_terrain);	UpdMiniTer();  }
void App::chkTimes(WP wp){			ChkEv(show_times);	ShowHUD();	}

//void App::chkRacingLine(WP wp){		ChkEv(racingline);	if (ndLine)  ndLine->setVisible(pSet->racingline);	}
void App::chkCamInfo(WP wp){		ChkEv(show_cam);	ShowHUD();	}

void App::chkCarDbgBars(WP wp){		ChkEv(car_dbgbars);	ShowHUD();	}
void App::chkCarDbgTxt(WP wp){		ChkEv(car_dbgtxt);	ShowHUD();	}
void App::chkBltDebug(WP wp){		ChkEv(bltDebug);	}
void App::chkBltProfilerTxt(WP wp){	ChkEv(bltProfilerTxt);	}

void App::radKmh(WP wp){	bRkmh->setStateSelected(true);  bRmph->setStateSelected(false);  pSet->show_mph = false;  ShowHUD();  }
void App::radMph(WP wp){	bRkmh->setStateSelected(false);  bRmph->setStateSelected(true);  pSet->show_mph = true;   ShowHUD();  }

//  Startup
void App::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}
void App::chkAutoStart(WP wp){		ChkEv(autostart);	}
void App::chkEscQuits(WP wp){		ChkEv(escquit);		}
void App::chkBltLines(WP wp){		ChkEv(bltLines);	}

void App::chkLoadPics(WP wp){		ChkEv(loadingbackground);	}


//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void App::chkVidEffects(WP wp)
{
	ChkEv(all_effects);  recreateCompositor();  //refreshCompositor();
}
void App::chkVidBloom(WP wp)
{		
	ChkEv(bloom);  refreshCompositor();
}
void App::chkVidHDR(WP wp)
{			
	ChkEv(hdr);  refreshCompositor();
}
void App::chkVidBlur(WP wp)
{		
	ChkEv(motionblur);  refreshCompositor();
}
void App::chkVidSSAA(WP wp)
{
	ChkEv(ssaa);  refreshCompositor();
}
void App::chkVidSSAO(WP wp)
{		
	ChkEv(ssao);  refreshCompositor();
}

void App::slBloomInt(SL)
{
	Real v = val/res;  if (bGI)  pSet->bloomintensity = v;
	if (valBloomInt){	Fmt(s, "%4.2f", v);	valBloomInt->setCaption(s);  }
	if (bGI)  refreshCompositor();
}
void App::slBloomOrig(SL)
{
	Real v = val/res;  if (bGI)  pSet->bloomorig = v;
	if (valBloomOrig){	Fmt(s, "%4.2f", v);	valBloomOrig->setCaption(s);  }
	if (bGI)  refreshCompositor();
}
void App::slBlurIntens(SL)
{
	Real v = val/res;  if (bGI)  pSet->motionblurintensity = v;
	if (valBlurIntens){	Fmt(s, "%4.2f", v);	valBlurIntens->setCaption(s);  }
	// if (bGI)  refreshCompositor();   // intensity is set every frame in UpdateHUD
}



//-----------------------------------------------------------------------------------------------------------
//  Key pressed
//-----------------------------------------------------------------------------------------------------------

// util
bool App::actionIsActive(std::string name, std::string pressed)
{
	std::string actionKey = GetInputName(mOISBsys->lookupAction("General/" + name)->mBindings[0]->mBindables[0].second->getBindableName());
	boost::to_lower(actionKey);
	boost::to_lower(pressed);
	return actionKey == pressed;
}

bool App::keyPressed( const OIS::KeyEvent &arg )
{
	// update all keystates
	OISB::System::getSingleton().process(0.001/*?0*/);
	
	// action key == pressed key
	#define action(s)  actionIsActive(s, mKeyboard->getAsString(arg.key))

	if (!bAssignKey)
	{
		//  change gui tabs
		if (mWndTabs && isFocGui)
		{	int num = mWndTabs->getItemCount();
			if (action("PrevTab")) {		mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() - 1 + num) % num ); return true;	}
			else if (action("NextTab")) {	mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() + 1) % num );	      return true;	}
		}
		
		//  gui on/off
		if (action("ShowOptions"))
		{	toggleGui();  return false;  }
	
		//  new game - reload
		if (action("RestartGame"))
		{	NewGame();  return false;	}

		//  new game - fast (same track & cars)
		if (action("ResetGame"))
		{
			for (int c=0; c < carModels.size(); ++c)
			{
				if (carModels[c]->pCar)  carModels[c]->pCar->ResetPos(true);
				if (carModels[c]->fCam)  carModels[c]->fCam->first = true;
				carModels[c]->ResetChecks();
			}
			pGame->timer.Reset(0);
		}
	}
	
	using namespace OIS;
	if (!bAssignKey)
	{
		switch (arg.key)
		{
			case KC_ESCAPE:
				if (pSet->escquit)
					mShutDown = true;	// quit
				else
					toggleGui();		// gui on/off
				return true;


			case KC_BACK:	// replay controls
				if (mWndRpl && !isFocGui)
				{	//mWndRpl->setVisible(!mWndRpl->isVisible());
					bRplWnd = !bRplWnd;  // ^set in sizehud
					return true;  }
				break;

			case KC_P:		// replay play/pause
				if (bRplPlay && !isFocGui)
				{	bRplPause = !bRplPause;  UpdRplPlayBtn();
					return true;  }
				break;
				

			case KC_F9:		// car debug text/bars
				if (shift)	{	WP wp = chDbgT;  ChkEv(car_dbgtxt);  ShowHUD();  }
				else		{	WP wp = chDbgB;  ChkEv(car_dbgbars);   ShowHUD();  }
				return true;

			case KC_F11:	//  fps
			if (!shift)
			{	WP wp = chFps;  ChkEv(show_fps); 
				if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();
				return false;
			}	break;

			case KC_F10:	//  blt debug, txt
			if (shift)
			{	WP wp = chBltTxt;  ChkEv(bltProfilerTxt);  return false;  }
			else if (ctrl)
			{	WP wp = chBlt;  ChkEv(bltDebug);  return false;  }
			break;

			case KC_F7:		// Times
			{	WP wp = chTimes;  ChkEv(show_times);  ShowHUD();  }
				return false;
				
			case KC_F8:		// Minimap
			{	WP wp = chMinimp;  ChkEv(trackmap);  if (ndMap)  ndMap->setVisible(pSet->trackmap);
			}	return false;
			
			case KC_F5:		//  new game
			//if (ctrl)
			{	NewGame();  return false;
			}	break;
			
			case KC_RETURN:	//  chng trk + new game  after up/dn
			if (isFocGui)
			{	size_t tab = mWndTabs->getIndexSelected();
				switch (tab)
				{
				case 0:
					btnChgTrack(0);
					btnNewGame(0);  break;
				case 1:
					btnChgCar(0);
					btnNewGame(0);  break;
				case 2:
					chatSendMsg();  break;
				case 3:
					btnRplLoad(0);  break;
			}	}
			return false;
		}
	}

	InputBind(arg.key);
	
	if (!BaseApp::keyPressed(arg))
		return true;

	return true;
}


void App::toggleGui()
{
	if (alt)  return;
	isFocGui = !isFocGui;
	if (mWndOpts)	mWndOpts->setVisible(isFocGui);
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	if (mGUI)	PointerManager::getInstance().setVisible(isFocGuiOrRpl());
	if (!isFocGui)  mToolTip->setVisible(false);
}
