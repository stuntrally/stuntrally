#pragma once

/**
 * @file
 * Communication with the master server.
 */

#include <boost/thread/condition.hpp>
#include "enet-wrapper.hpp"
#include "protocol.hpp"
#include "networkcallbacks.hpp"
#include "MyGUI_UString.h"

/**
 * @brief Client for connecting to the master server.
 *
 * This class handles the messaging with the master server
 * (which is responsible for announcing games).
 *
 * The main function is to get info on available games.
 * It is up to the application to request refresh, and
 * the list may also fill gradually, not completely at once.
 *
 * To get events, give it a MasterClientCallback instance pointer.
 * Note that the events are received from a separate thread.
 *
 * This class is thread-safe.
 */
class MasterClient: public net::NetworkListener
{
public:
	MasterClient(MasterClientCallback* callback = NULL, int updateInterval = 2000);

	~MasterClient();

	/// Connects to the master server
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	/// Updates the hosted game state to master server, starts updater thread if necessary
	void updateGame(const protocol::GameInfo& game, std::string password = "");

	/// Signals the server that the game has started
	void signalStart();

	/// Clears cache, requests new game listing, doesn't wait for it to arrive
	void refreshList();

	/// Returns cached list of games
	protocol::GameList getList() const { return m_games; }

	/// Terminates the updater
	void terminate();

	/// Thread that periodically broadcasts game info, don't call directly
	void gameInfoSenderThread();

	/// Get error message, if any. Clears it also.
	MyGUI::UString getError();

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	MasterClientCallback* m_callback;
	boost::thread m_gameInfoSenderThread;
	mutable boost::mutex m_mutex;
	
	boost::condition m_cond;
	net::NetworkObject m_client;
	
	protocol::GameList m_games;
	protocol::GameInfo m_game;
	
	std::string m_password;
	MyGUI::UString m_error;
	
	int m_updateInterval;
	bool m_sendUpdates;
	volatile bool m_connectionOk;
};
