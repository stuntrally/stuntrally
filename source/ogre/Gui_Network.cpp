#include "pch.h"
#include "common/Def_Str.h"
#include "common/GuiCom.h"
#include "CGame.h"
#include "CGui.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
#include "common/MultiList2.h"
#include "common/Gui_Popup.h"
#include "common/MessageBox/MessageBox.h"
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace MyGUI;


///  Gui Events

//  [Multiplayer]
//---------------------------------------------------------------------

namespace
{
	string yesno(bool cond)
	{
		return cond ? TR("#{Yes}") : TR("#{No}");
	}

	void inline raiseError(const string& what, const string& title = TR("#{Error}"))
	{
		Message::createMessageBox("Message", title, what, MessageBoxStyle::IconError | MessageBoxStyle::Ok);
	}
}


void CGui::rebuildGameList()
{
	if (!listServers || !app->mMasterClient)  return;
	Mli li = listServers;
	li->removeAllItems();

	protocol::GameList list = app->mMasterClient->getList();
	const static char* sBoost[4] = {"#{Never}","#{FuelLap}","#{FuelTime}","#{Always}"};
	
	for (protocol::GameList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		li->addItem("#C0FFC0"+UString(it->second.name));  int l = li->getItemCount()-1;
		li->setSubItemNameAt(1,l, "#50FF50"+ string(it->second.track));
		li->setSubItemNameAt(2,l, "#80FFC0"+ toStr((int)it->second.laps));
		li->setSubItemNameAt(3,l, "#FFFF00"+ toStr((int)it->second.players));
		li->setSubItemNameAt(4,l, "#80FFFF"+ yesno((bool)it->second.collisions));
		li->setSubItemNameAt(5,l, "#D0D0FF"+ string(it->second.sim_mode));
		li->setSubItemNameAt(6,l, "#A0D0FF"+ TR(sBoost[it->second.boost_type]));
		li->setSubItemNameAt(iColLock,l, "#FF6060"+ yesno((bool)it->second.locked));
		li->setSubItemNameAt(iColHost,l, "#FF9000"+ net::IPv4(it->second.address));
		li->setSubItemNameAt(iColPort,l, "#FFB000"+ toStr((int)it->second.port));
	}
}

void CGui::rebuildPlayerList()
{
	if (!listPlayers || !app->mClient)  return;
	Mli li = listPlayers;
	li->removeAllItems();

	//  Add self
	int peerCount = app->mClient->getPeerCount();
	li->addItem("#C0E0FF"+ pSet->nickname);
	li->setSubItemNameAt(1,0, "#80FFFF"+ sListCar);
	li->setSubItemNameAt(2,0, "#F0F060"+ toStr(peerCount));
	li->setSubItemNameAt(3,0, "#C0F0F0" "0");  bool rd = app->mClient->isReady();
	li->setSubItemNameAt(4,0, (rd?"#60FF60":"#FF8080")+ yesno(rd));

	//  Add others
	bool allReady = true;
	const PeerMap peers = app->mClient->getPeers();
	for (PeerMap::const_iterator it = peers.begin(); it != peers.end(); ++it)
	{
		if (it->second.name.empty() || it->second.connection == PeerInfo::DISCONNECTED)
			continue;
		// Determine if everyone is ready and connected
		if (it->second.peers != peerCount || !it->second.ready)
			allReady = false;

		// Add list item
		li->addItem("#C0E0FF"+ it->second.name);  int l = li->getItemCount()-1;
		li->setSubItemNameAt(1,l, "#80FFFF"+ it->second.car);
		li->setSubItemNameAt(2,l, "#F0F060"+ toStr(it->second.peers));
		li->setSubItemNameAt(3,l, "#C0F0F0"+ toStr(it->second.ping));  bool rd = it->second.ready;
		li->setSubItemNameAt(4,l, (rd?"#60FF60":"#FF8080")+ yesno(rd));
	}
	//  Allow host to start the game
	if (app->mLobbyState == HOSTING)
		btnNetReady->setEnabled(allReady);
}

void CGui::updateGameInfo()
{
	//  set game config
	if (netGameInfo.name && edNetGameName)
	{	string name(netGameInfo.name);
		edNetGameName->setCaption(name);
	}
	if (netGameInfo.track)
	{	string track(netGameInfo.track);
		gcom->sListTrack = track;
		gcom->ReadTrkStats();
	}
	updateGameSet();
	updateGameInfoGUI();
}

///  update game info
void CGui::updateGameInfoGUI()
{
	if (!valNetGameInfo)  return;
	using Ogre::String;

	String s;  const protocol::GameInfo& g = netGameInfo;
	s += TR("#40FF40#{Track}:      ") + gcom->sListTrack +"\n";
	s += TR("#60A060#{Reverse}:  ") + yesno(g.reversed) +"\n";
	s += "\n";
	s += TR("#80F0F0#{Laps}:   ") + toStr(g.laps) +"\n";
	s += TR("#409090#{ReverseStartOrder}:  ") + yesno(g.start_order) +"\n";
	s += TR("#F0F040#{Players}:  ") + toStr(g.players) +"\n";
	s += "\n";
	s += TR("#D090E0#{Game}") +"\n";
	s += TR("#90B0E0  #{Simulation}:  ") + String(g.sim_mode) +"\n";
	s += TR("#A0D0D0  #{CarCollis}:  ") + yesno(g.collisions) +"\n";
	s += "\n";
	#define cmbs(cmb, i)  (i>=0 && i < cmb->getItemCount() ? cmb->getItemNameAt(i) : TR("#{Any}"))
	s += TR("#80C0FF  #{Boost}:  ") + "#90D0FF"+ cmbs(cmbBoost, g.boost_type) +"\n";
	s += TR("#6098A0  #{Flip}:  ") + "#7098A0"+ cmbs(cmbFlip, g.flip_type) +"\n";
	s += "\n";
	s += TR("#A090E0  #{Damage}:  ") + "#B090FF"+ cmbs(cmbDamage, g.damage_type) +"\n";
	s += TR("#B080C0  #{InputMapRewind}:  ") + "#C090D0"+ cmbs(cmbRewind, g.rewind_type) +"\n";
	//float boost_power;
	//float damage_lap_dec, boost_lap_inc, rewind_lap_inc;  //todo
	//uint8_t tree_collis;  float tree_mult;

	valNetGameInfo->setCaption(s);
}


///  Receive  set game from host
//---------------------------------------------------------------------
void CGui::updateGameSet()
{
	pSet->game.sim_mode = netGameInfo.sim_mode;  LogO("== Netw sim mode: " + pSet->game.sim_mode);

	pSet->game.collis_cars = netGameInfo.collisions>0;
	pSet->game.num_laps = netGameInfo.laps;		LogO("== Netw laps num: " + toStr(pSet->game.num_laps));
	pSet->game.trackreverse = netGameInfo.reversed>0;

	pSet->game.start_order = netGameInfo.start_order;
	pSet->game.collis_veget = netGameInfo.tree_collis>0;
	pSet->game.trees = netGameInfo.tree_mult;

	pSet->game.BoostDefault();  //
	pSet->game.boost_type = netGameInfo.boost_type;
	pSet->game.boost_power = netGameInfo.boost_power;
	pSet->game.flip_type = netGameInfo.flip_type;

	pSet->game.damage_type = netGameInfo.damage_type;
	pSet->game.damage_dec = netGameInfo.damage_lap_dec;
	pSet->game.rewind_type = netGameInfo.rewind_type;
	//boost_lap_inc, rewind_lap_inc;  //todo
}

///  Send  upload to peers
//---------------------------------------------------------------------
void CGui::uploadGameInfo()
{
	if (!app->mMasterClient || !app->mClient || !edNetGameName || !pSet)
		return;
	protocol::GameInfo game;
	string sGame = edNetGameName->getCaption();
	string sTrack = gcom->sListTrack, sSim = pSet->gui.sim_mode;
	
	memset(game.name, 0, sizeof(game.name));
	memset(game.track, 0, sizeof(game.track));
	memset(game.sim_mode, 0, sizeof(game.sim_mode));
	strcpy(game.name, sGame.c_str());
	strcpy(game.track, sTrack.c_str());
	strcpy(game.sim_mode, sSim.c_str());
	game.players = app->mClient->getPeerCount()+1;

	game.collisions = pSet->gui.collis_cars?1:0;
	game.laps = pSet->gui.num_laps;				LogO("== Netw laps num: " + toStr(pSet->gui.num_laps));
	game.reversed = pSet->gui.trackreverse?1:0;

	game.start_order = pSet->gui.start_order;
	game.tree_collis = pSet->gui.collis_veget?1:0;
	game.tree_mult = pSet->gui.trees;

	game.boost_type = pSet->gui.boost_type;
	game.boost_power = pSet->gui.boost_power;
	game.flip_type = pSet->gui.flip_type;

	game.damage_type = pSet->gui.damage_type;
	game.damage_lap_dec = pSet->gui.damage_dec;
	game.rewind_type = pSet->gui.rewind_type;
	//boost_lap_inc, rewind_lap_inc;  //todo
	
	game.port = pSet->local_port;
	game.locked = !edNetPassword->getCaption().empty();
	{
		//? boost::mutex::scoped_lock lock(netGuiMutex);
		netGameInfo = game;  // for host gui info
	}
	app->mMasterClient->updateGame(game); // Upload to master server
	if (app->mClient)  // Send to peers
		app->mClient->broadcastGameInfo(game);
}
//---------------------------------------------------------------------


void CGui::setNetGuiHosting(bool enabled)
{
	edNetGameName->setEnabled(enabled);
	edNetPassword->setEnabled(enabled);
	edNetPassword->setVisible(enabled);
	valNetPassword->setVisible(enabled);
	btnNetReady->setEnabled(!enabled);
	btnNetReady->setCaption(enabled ? TR("#{NetStart}") : TR("#{NetReady}"));
}

void CGui::gameListChanged(protocol::GameList list)
{
	(void)list;
	boost::mutex::scoped_lock lock(netGuiMutex);
	bRebuildGameList = true;
}

//  add new msg at end
void CGui::AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add)
{
	if (!app->isFocGui)  // if not in gui, show on hud
	{	sChatLast1 = sChatLast2;
		sChatLast2 = msg;
		iChatMove = 0;
	}
	sChatBuffer = add ? (sChatBuffer + clr + msg + "\n") : (clr + msg + "\n");
	bUpdChat = true;
}

void CGui::peerConnected(PeerInfo peer)
{
	// Master server player count update
	if (app->mLobbyState == HOSTING)  uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#00FF00", UString("Connected: ") + peer.name);
	bRebuildPlayerList = true;
}

void CGui::peerDisconnected(PeerInfo peer)
{
	if (peer.name.empty())  return;
	// Master server player count update
	if (app->mLobbyState == HOSTING)  uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#FF8000", UString("Disconnected: ") + peer.name);
	bRebuildPlayerList = true;
}

void CGui::peerInfo(PeerInfo peer)
{
	(void)peer;
	boost::mutex::scoped_lock lock(netGuiMutex);
	bRebuildPlayerList = true;
}

void CGui::peerMessage(PeerInfo peer, string msg)
{
	boost::mutex::scoped_lock lock(netGuiMutex);

	int hc = 0;  // color from name
	int len = peer.name.length();
	hc += len;  hc += peer.name[0];
	const static int num = 16;
	const static char sclr[num][8] = {
		"#80FFC0","#B0FF40","#FF80C0","#C0FF80","#40FFFF","#FF4080","#8080FF","#FF80FF",
		"#3050FF","#80C0FF","#E0F0FF","#FFFFFF","#C080FF","#FFFF40","#FFC040","#FF8080"};

	AddChatMsg(sclr[hc % num], UString(peer.name) + ": " + msg);
	bRebuildPlayerList = true; // For ping updates in the list
}

void CGui::peerState(PeerInfo peer, uint8_t state)
{
	(void)peer;
	boost::mutex::scoped_lock lock(netGuiMutex);
	if (state == protocol::START_GAME)
	{
		bStartGame = true;
		bStartedGame = true;
	}
}

void CGui::gameInfo(protocol::GameInfo game)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	netGameInfo = game;
	bUpdateGameInfo = true;
}

void CGui::returnToLobby()
{
	btnNetReady->setCaption(TR("#{NetReady}"));
	app->isFocGui = true;  // show back gui
	toggleGui(false);
}

void CGui::startRace()
{
	LogO("== Netw startRace +");
	pGame->timer.waiting = false;
	pGame->timer.end_sim = false;
}

///  Lap time got from network
void CGui::timeInfo(ClientID id, uint8_t lap, double time)
{
	//if (!ap->mClient)  return;
	if (id == 0)  id = app->mClient->getId();

	LogO("== Netw Lap " +toStr(lap) +" finished by " +toStr(id)+ " time:"+ toStr(float(time)));
	if (id >= app->carModels.size() || id < 0)
	{	LogO("== Netw Lap id wrong !" );  return;  }
	
	//pGame->timer.Lap(id, 0,0, true, pSet->game.trackreverse/*<, pSet->boost_type*/);
	pGame->timer.LapNetworkTime(id, time);  // is the same as above but sets client's time
	//carModels[id]->trackPercent = 0.f;
	//carPoses[id].percent = 0.f;
}

void CGui::error(string what)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#FF3030", UString("ERROR! ") + what);
}

void CGui::join(string host, string port, string password)
{
	try
	{	app->mClient.reset(new P2PGameClient(this, pSet->local_port));
		app->mClient->updatePlayerInfo(pSet->nickname, sListCar);
		app->mClient->connect(host, boost::lexical_cast<int>(port), password); // Lobby phase started automatically
		boost::mutex::scoped_lock lock(netGuiMutex);
		AddChatMsg("#00FFFF", TR("Connecting to ") + host + ":" + port, false);  // clears chat
	}catch (...)
	{	raiseError(TR("Failed to initialize networking.\nTry different local port and make sure your firewall is properly configured."), TR("Network Error"));
		return;
	}

	updateGameInfoGUI();
	rebuildPlayerList();
	setNetGuiHosting(false);

	tabsNet->setIndexSelected(1);
	panNetServer->setVisible(true);  panNetGame->setVisible(false);
	panNetTrack->setVisible(true);   gcom->trkList->setVisible(false);
}

void CGui::evBtnNetRefresh(WP)
{
	app->mMasterClient.reset(new MasterClient(this));
	app->mMasterClient->connect(pSet->master_server_address, pSet->master_server_port);
	// The actual refresh will be requested automatically when the connection is made
}

void CGui::evBtnNetJoin(WP)
{
	//  join selected game
	if (!listServers || !pSet)  return;
	size_t i = listServers->getIndexSelected();
	if (i == ITEM_NONE)  return;

	//  TODO: Comparing against localized string is EVIL!
	if (listServers->getSubItemNameAt(iColLock, i).substr(7) == TR("#{No}"))
	{
		string host = listServers->getSubItemNameAt(iColHost, i).substr(7);
		string port = listServers->getSubItemNameAt(iColPort, i).substr(7);
		
		join(host, port, "");
	}else
		popup->Show(newDelegate(this, &CGui::evBtnNetJoinLockedClose),
			TR("#{NetJoinLocked}"), true,
			TR("#{NetPassword}"), "", "", "",
			"", "", "","",
			TR("#{MessageBox_Ok}"), TR("#{MessageBox_Cancel}"), "", "");
}

void CGui::evBtnNetJoinLockedClose()
{
	popup->Hide();
	if (popup->btnResult != 0 || !listServers || !pSet)  return;
	size_t i = listServers->getIndexSelected();  if (i == ITEM_NONE)  return;

	string host = listServers->getSubItemNameAt(iColHost, i).substr(7);
	string port = listServers->getSubItemNameAt(iColPort, i).substr(7);

	join(host, port, popup->edit0);  // host, port, password
}

void CGui::evBtnNetCreate(WP)
{
	//  create game ..
	if (app->mLobbyState == DISCONNECTED)
	{	try
		{
			app->mClient.reset(new P2PGameClient(this, pSet->local_port));
			app->mClient->updatePlayerInfo(pSet->nickname, sListCar);
			app->mClient->startLobby();
		}
		catch (...)
		{	raiseError(TR("Failed to initialize networking.\nTry different local port and make sure your firewall is properly configured."), TR("Network Error"));
			return;
		}
		app->mLobbyState = HOSTING;
		if (!app->mMasterClient)
		{
			app->mMasterClient.reset(new MasterClient(this));
			app->mMasterClient->connect(pSet->master_server_address, pSet->master_server_port);
		}
		uploadGameInfo();
		updateGameInfoGUI();
		rebuildPlayerList();
		setNetGuiHosting(true);

		tabsNet->setIndexSelected(1);
		panNetServer->setVisible(true);  panNetGame->setVisible(false);
		panNetTrack->setVisible(false);  gcom->trkList->setVisible(true);

		boost::mutex::scoped_lock lock(netGuiMutex);
		AddChatMsg("#00FFC0", TR("Listening on port ")  + toStr(pSet->local_port) + "...", false);  //clears chat
	}
}

void CGui::evBtnNetLeave(WP)
{
	//  leave current game
	app->mLobbyState = DISCONNECTED;
	app->mClient.reset();
	app->mMasterClient.reset();
	setNetGuiHosting(false);

	tabsNet->setIndexSelected(0);
	panNetServer->setVisible(false);  panNetGame->setVisible(true);
	panNetTrack->setVisible(false);   gcom->trkList->setVisible(true);
}

void CGui::evBtnNetDirect(WP)
{
	popup->Show(newDelegate(this, &CGui::evBtnNetDirectClose),
		TR("#{NetDirectConnect}"), true,
		TR("#{NetAddress}"), TR("#{NetPort}"), TR("#{NetPassword}"), "",
		"localhost", toStr(protocol::DEFAULT_PORT), "","",
		TR("#{MessageBox_Ok}"), TR("#{MessageBox_Cancel}"), "", "");
}

void CGui::evBtnNetDirectClose()
{
	popup->Hide();
	if (popup->btnResult != 0)  return;

	join(popup->edit0, popup->edit1, popup->edit2);  // host, port, password
}

void CGui::evBtnNetReady(WP)
{
	if (!app->mClient)  return;

	app->mClient->toggleReady();
	if (app->mLobbyState == HOSTING)
	{
		LogO("== Netw Ready, hosting...");
		if (!bStartedGame)
		{
			if (app->mMasterClient)  app->mMasterClient->signalStart();
			boost::mutex::scoped_lock lock(netGuiMutex);
			bStartGame = true;
			bStartedGame = true;
			btnNetReady->setCaption( TR("#{NetNew}") );
		}else{
			app->mClient->returnToLobby();
			boost::mutex::scoped_lock lock(netGuiMutex);
			bStartGame = false;
			bStartedGame = false;
			btnNetReady->setCaption( TR("#{NetStart}") );
		}
	}else
	{
		if (app->mClient->isReady())
			btnNetReady->setCaption( TR("#{NetWaiting}") );
		else btnNetReady->setCaption( TR("#{NetReady}") );
	}

	rebuildPlayerList();
}


void CGui::chatSendMsg()
{
	if (!app->mClient || !edNetChatMsg)  return;
	if (edNetChatMsg->getCaption().empty())  return;

	app->mClient->sendMessage(edNetChatMsg->getCaption());
	edNetChatMsg->setCaption("");
}

void CGui::evEdNetGameName(EditPtr ed)
{
	//  game name text changed
	pSet->netGameName = ed->getCaption();
	if (app->mLobbyState != HOSTING || !app->mMasterClient || !app->mClient)  return;
	uploadGameInfo();
}

void CGui::evEdNetPassword(EditPtr)
{
	//  password changed
	if (app->mLobbyState != HOSTING || !app->mMasterClient || !app->mClient)  return;

	app->mClient->setPassword(edNetPassword->getCaption());
	uploadGameInfo();
}

//  net settings

void CGui::evEdNetNick(EditPtr ed)
{
	pSet->nickname = ed->getCaption();
	if (app->mClient)  app->mClient->updatePlayerInfo(pSet->nickname, sListCar);
}

void CGui::evEdNetServerIP(EditPtr ed)
{
	pSet->master_server_address = ed->getCaption();
}

void CGui::evEdNetServerPort(EditPtr ed)
{
	pSet->master_server_port = s2i(ed->getCaption());
}

void CGui::evEdNetLocalPort(EditPtr ed)
{
	pSet->local_port = s2i(ed->getCaption());
}


///  Gui updates from networking
//  handled in ogre thread as MyGUI is not thread-safe
void CGui::UpdGuiNetw()
{
	if (app->isFocGui)
	{
		if (app->mMasterClient) {
			std::string error = app->mMasterClient->getError();
			if (!error.empty())
				Message::createMessageBox("Message", TR("#{Error}"), error,
					MessageBoxStyle::IconError | MessageBoxStyle::Ok);
		}
		boost::mutex::scoped_lock lock(netGuiMutex);
		if (bRebuildGameList) {
			bRebuildGameList = false;  rebuildGameList();  }
		if (bRebuildPlayerList) {
			bRebuildPlayerList = false;  rebuildPlayerList();  }
		if (bUpdateGameInfo) {
			bUpdateGameInfo = false;  updateGameInfo();  }
		if (bUpdChat) {
			bUpdChat = false;  edNetChat->setCaption(sChatBuffer);  }
		if (bStartGame) {
			bStartGame = false;  app->mClient->startGame();  btnNewGameStart(NULL);  }
	}
}
