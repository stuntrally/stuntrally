#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
//using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//  [Multiplayer]
//---------------------------------------------------------------------

namespace
{
	std::string yesno(bool cond)
	{
		return cond ? TR("#{Yes}") : TR("#{No}");
	}

	void inline raiseError(const std::string& what, const std::string& title = TR("#{Error}"))
	{
		Message::createMessageBox("Message", title, what, MessageBoxStyle::IconError | MessageBoxStyle::Ok);
	}
}


void App::rebuildGameList()
{
	if (!listServers || !mMasterClient)  return;
	listServers->removeAllItems();

	protocol::GameList list = mMasterClient->getList();
	const static char* sBoost[4] = {"#{Never}","#{FuelLap}","#{FuelTime}","#{Always}"};
	
	for (protocol::GameList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		listServers->addItem("#C0FFC0"+UString(it->second.name));  int l = listServers->getItemCount()-1;
		listServers->setSubItemNameAt(1, l, "#50FF50"+std::string(it->second.track));
		listServers->setSubItemNameAt(2, l, "#80FFC0"+toStr((int)it->second.laps));
		listServers->setSubItemNameAt(3, l, "#FFFF00"+toStr((int)it->second.players));
		listServers->setSubItemNameAt(4, l, "#80FFFF"+yesno((bool)it->second.collisions));
		listServers->setSubItemNameAt(5, l, "#A0D0FF"+TR(sBoost[it->second.boost_type]));
		listServers->setSubItemNameAt(iColLock, l, "#FF6060"+yesno((bool)it->second.locked));
		listServers->setSubItemNameAt(iColHost, l, "#FF9000"+net::IPv4(it->second.address));
		listServers->setSubItemNameAt(iColPort, l, "#FFB000"+toStr((int)it->second.port));
	}
}

void App::rebuildPlayerList()
{
	if (!listPlayers || !mClient)  return;
	listPlayers->removeAllItems();

	//  Add self
	unsigned peerCount = mClient->getPeerCount();
	listPlayers->addItem("#C0E0FF"+pSet->nickname);
	listPlayers->setSubItemNameAt(1, 0, "#80FFFF"+sListCar); // Car
	listPlayers->setSubItemNameAt(2, 0, "#C0C0FF"+toStr(peerCount)); // Peers
	listPlayers->setSubItemNameAt(3, 0, "#C0FFFF""0");  bool rd = mClient->isReady(); // Ping
	listPlayers->setSubItemNameAt(4, 0, (rd?"#80FF80":"#FF8080")+yesno(rd)); // Ready state

	//  Add others
	bool allReady = true;
	const PeerMap peers = mClient->getPeers();
	for (PeerMap::const_iterator it = peers.begin(); it != peers.end(); ++it)
	{
		if (it->second.name.empty() || it->second.connection == PeerInfo::DISCONNECTED)
			continue;
		// Determine if everyone is ready and connected
		if (it->second.peers != peerCount || !it->second.ready)
			allReady = false;

		// Add list item
		listPlayers->addItem("#C0E0FF"+it->second.name);  int l = listPlayers->getItemCount()-1;
		listPlayers->setSubItemNameAt(1, l, "#80FFFF"+it->second.car);
		listPlayers->setSubItemNameAt(2, l, "#C0C0FF"+toStr(it->second.peers));
		listPlayers->setSubItemNameAt(3, l, "#C0FFFF"+toStr(it->second.ping));  bool rd = it->second.ready;
		listPlayers->setSubItemNameAt(4, l, (rd?"#80FF80":"#FF8080")+yesno(rd));
	}
	//  Allow host to start the game
	if (mLobbyState == HOSTING)
		btnNetReady->setEnabled(allReady);
}

void App::updateGameInfo()
{
	//  set game config
	if (netGameInfo.name && edNetGameName)
	{	std::string name(netGameInfo.name);
		edNetGameName->setCaption(name);
	}
	if (netGameInfo.track)
	{	std::string track(netGameInfo.track);
		sListTrack = track;
		ReadTrkStats();
	}
	updateGameSet();
	updateGameInfoGUI();
}

void App::updateGameSet()
{
	pSet->game.collis_cars = netGameInfo.collisions;
	pSet->game.num_laps = netGameInfo.laps;		LogO("== Netw laps num: " + toStr(pSet->game.num_laps));
	pSet->game.flip_type = netGameInfo.flip_type;
	pSet->game.boost_type = netGameInfo.boost_type;
	pSet->game.boost_power = netGameInfo.boost_power;
	pSet->game.trackreverse = netGameInfo.reversed;
}

void App::updateGameInfoGUI()
{
	//  update track info
	if (valNetTrack)
		valNetTrack->setCaption("Track: " + sListTrack);
	if (imgNetTrack)
		imgNetTrack->setImageTexture(sListTrack+".jpg");
	if (edNetTrackInfo && trkDesc)
		edNetTrackInfo->setCaption(trkDesc[0]->getCaption());
	// todo: probably should also update on gui collis,boost,reverse,... (but not in pSet.gui)
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

	game.collisions = pSet->gui.collis_cars;  // game set
	game.laps = pSet->gui.num_laps;				LogO("== Netw laps num: " + toStr(pSet->gui.num_laps));
	game.flip_type = pSet->gui.flip_type;
	game.boost_type = pSet->gui.boost_type;
	game.boost_power = pSet->gui.boost_power;
	game.reversed = pSet->gui.trackreverse;

	game.port = pSet->local_port;
	game.locked = !edNetPassword->getCaption().empty();
	mMasterClient->updateGame(game); // Upload to master server
	if (mClient) // Send to peers
		mClient->broadcastGameInfo(game);
}

void App::setNetGuiHosting(bool enabled)
{
	edNetGameName->setEnabled(enabled);
	edNetPassword->setEnabled(enabled);
	edNetPassword->setVisible(enabled);
	valNetPassword->setVisible(enabled);
	btnNetReady->setEnabled(!enabled);
	btnNetReady->setCaption(enabled ? TR("#{NetStart}") : TR("#{NetReady}"));
}

void App::gameListChanged(protocol::GameList list)
{
	(void)list;
	boost::mutex::scoped_lock lock(netGuiMutex);
	bRebuildGameList = true;
}

//  add new msg at end
void App::AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add)
{
	if (!isFocGui)  // if not in gui, show on hud
	{	sChatLast1 = sChatLast2;
		sChatLast2 = msg;
		iChatMove = 0;
	}
	sChatBuffer = add ? (sChatBuffer + clr + msg + "\n") : (clr + msg + "\n");
	bUpdChat = true;
}

void App::peerConnected(PeerInfo peer)
{
	// Master server player count update
	if (mLobbyState == HOSTING)  uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#00FF00", UString("Connected: ") + peer.name);
	bRebuildPlayerList = true;
}

void App::peerDisconnected(PeerInfo peer)
{
	if (peer.name.empty())  return;
	// Master server player count update
	if (mLobbyState == HOSTING)  uploadGameInfo();
	// Schedule Gui updates
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#FF8000", UString("Disconnected: ") + peer.name);
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

void App::peerState(PeerInfo peer, uint8_t state)
{
	(void)peer;
	boost::mutex::scoped_lock lock(netGuiMutex);
	if (state == protocol::START_GAME)
		bStartGame = true;
}

void App::gameInfo(protocol::GameInfo game)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	netGameInfo = game;
	bUpdateGameInfo = true;
}

void App::startRace()
{
	LogO("== Netw startRace +");
	pGame->timer.waiting = false;
}

///  Lap time got from network
void App::timeInfo(ClientID id, uint8_t lap, double time)
{
	//if (!mClient)  return;
	if (id == 0)  id = mClient->getId();

	LogO("== Netw Lap " +toStr(lap) +" finished by " +toStr(id)+ " time:"+ toStr(float(time)));
	if (id >= carModels.size() || id < 0)
	{	LogO("== Netw Lap id wrong !" );  return;  }
	
	//pGame->timer.Lap(id, 0,0, true, pSet->game.trackreverse/*<, pSet->boost_type*/);
	pGame->timer.LapNetworkTime(id, time);  // is the same as above but sets client's time
	//carModels[id]->trackPercent = 0.f;
	//carPoses[id].percent = 0.f;
}

void App::error(string what)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	AddChatMsg("#FF3030", UString("ERROR! ") + what);
}

void App::join(std::string host, std::string port, std::string password)
{
	try
	{	mClient.reset(new P2PGameClient(this, pSet->local_port));
		mClient->updatePlayerInfo(pSet->nickname, sListCar);
		mClient->connect(host, boost::lexical_cast<int>(port), password); // Lobby phase started automatically
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
	if (!listServers || !pSet)  return;
	size_t i = listServers->getIndexSelected();
	if (i == ITEM_NONE)  return;

	//  TODO: Comparing against localized string is EVIL!
	if (listServers->getSubItemNameAt(iColLock, i).substr(7) == TR("#{No}"))
	{
		std::string host = listServers->getSubItemNameAt(iColHost, i).substr(7);
		std::string port = listServers->getSubItemNameAt(iColPort, i).substr(7);
		
		join(host, port, "");
	}else{
		popup.Show(newDelegate(this, &App::evBtnNetJoinLockedClose),
			TR("#{NetJoinLocked}"), true,
			TR("#{NetPassword}"), "", "", "",
			"", "", "","",
			TR("#{MessageBox_Ok}"), TR("#{MessageBox_Cancel}"), "", "");
	}
}

void App::evBtnNetJoinLockedClose()
{
	popup.Hide();
	if (popup.btnResult != 0 || !listServers || !pSet)  return;
	size_t i = listServers->getIndexSelected();  if (i == ITEM_NONE)  return;

	std::string host = listServers->getSubItemNameAt(iColHost, i);
	std::string port = listServers->getSubItemNameAt(iColPort, i);
	
	join(host, port, popup.edit0);  // host, port, password
}

void App::evBtnNetCreate(WP)
{
	//  create game ..
	if (mLobbyState == DISCONNECTED)
	{	try
		{
			if (pSet) mClient.reset(new P2PGameClient(this, pSet->local_port));
			mClient->updatePlayerInfo(pSet->nickname, sListCar);
			mClient->startLobby();
		}
		catch (...)
		{	raiseError(TR("Failed to initialize networking.\nTry different local port and make sure your firewall is properly configured."), TR("Network Error"));
			return;
		}
		mLobbyState = HOSTING;
		if (!mMasterClient)
		{
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
		AddChatMsg("#00FFC0", TR("Listening on port ")  + toStr(pSet->local_port) + "...", false);  //clears chat
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
		TR("#{NetAddress}"), TR("#{NetPort}"), TR("#{NetPassword}"), "",
		"localhost", toStr(protocol::DEFAULT_PORT), "","",
		TR("#{MessageBox_Ok}"), TR("#{MessageBox_Cancel}"), "", "");
}

void App::evBtnNetDirectClose()
{
	popup.Hide();
	if (popup.btnResult != 0)  return;

	join(popup.edit0, popup.edit1, popup.edit2);  // host, port, password
}

void App::evBtnNetReady(WP)
{
	if (!mClient)  return;

	mClient->toggleReady();
	if (mClient->isReady())
	{
		if (mLobbyState == HOSTING)
		{
			if (mMasterClient) mMasterClient->signalStart();
			boost::mutex::scoped_lock lock(netGuiMutex);
			bStartGame = true;
		}else
			btnNetReady->setCaption( TR("#{NetWaiting}") );
	}else
		btnNetReady->setCaption( TR("#{NetReady}") );

	rebuildPlayerList();
}


void App::chatSendMsg()
{
	if (!mClient || !edNetChatMsg)  return;
	if (edNetChatMsg->getCaption().empty())  return;

	mClient->sendMessage(edNetChatMsg->getCaption());
	edNetChatMsg->setCaption("");
}

void App::evEdNetGameName(EditPtr ed)
{
	//  game name text changed
	pSet->netGameName = ed->getCaption();
	if (mLobbyState != HOSTING || !mMasterClient || !mClient)  return;
	uploadGameInfo();
}

void App::evEdNetPassword(EditPtr)
{
	//  password changed
	if (mLobbyState != HOSTING || !mMasterClient || !mClient)  return;

	mClient->setPassword(edNetPassword->getCaption());
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
