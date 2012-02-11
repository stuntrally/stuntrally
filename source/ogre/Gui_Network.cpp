#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
//#include "common/Gui_Def.h"
//#include "common/RenderConst.h"

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
	//  set game config
	if (netGameInfo.name && edNetGameName) {
		std::string name(netGameInfo.name);
		edNetGameName->setCaption(name);
	}
	if (netGameInfo.track) {
		std::string track(netGameInfo.track);
		sListTrack = track;
		ReadTrkStats();
	}
	pSet->game.collis_cars = netGameInfo.collisions;
	pSet->game.collis_veget = true;
	pSet->game.num_laps = netGameInfo.laps;
	pSet->game.flip_type = netGameInfo.flip_type;
	pSet->game.boost_type = netGameInfo.boost_type;
	pSet->game.boost_power = netGameInfo.boost_power;
	pSet->game.trackreverse = netGameInfo.reversed;
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
		edNetTrackInfo->setCaption(trkDesc[0]->getCaption());
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
	game.laps = pSet->gui.num_laps;
	game.flip_type = pSet->game.flip_type;
	game.boost_type = pSet->game.boost_type;
	game.boost_power = pSet->game.boost_power;
	game.reversed = pSet->game.trackreverse;

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

void App::startRace()
{
	pGame->timer.waiting = false;
}

void App::timeInfo(ClientID id, uint8_t lap, double time)
{
	// TODO: Do something with this
}

void App::error(string what)
{
	boost::mutex::scoped_lock lock(netGuiMutex);
	sChatBuffer = sChatBuffer + "ERROR! " + what + "\n";
}

void App::join(std::string host, std::string port, std::string password)
{
	try {
		mClient.reset(new P2PGameClient(this, pSet->local_port));
		mClient->updatePlayerInfo(pSet->nickname, sListCar);
		mClient->connect(host, boost::lexical_cast<int>(port), password); // Lobby phase started automatically
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

	// TODO: Comparing against localized string is EVIL!
	if (listServers->getSubItemNameAt(4, i) == TR("#{No}")) {
		std::string host = listServers->getSubItemNameAt(5, i);
		std::string port = listServers->getSubItemNameAt(6, i);
		join(host, port, "");
	} else {
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
	size_t i = listServers->getIndexSelected();
	if (i == ITEM_NONE) return;

	std::string host = listServers->getSubItemNameAt(5, i);
	std::string port = listServers->getSubItemNameAt(6, i);
	join(host, port, popup.edit0);  // host, port, password
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

void App::evEdNetGameName(EditPtr)
{
	// game name text changed
	if (mLobbyState != HOSTING || !mMasterClient || !mClient) return;
	uploadGameInfo();
}

void App::evEdNetPassword(EditPtr)
{
	// password changed
	if (mLobbyState != HOSTING || !mMasterClient || !mClient) return;
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
