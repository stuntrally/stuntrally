#pragma once
/**
 * @file
 * Communication with the master server.
 */

#include "enet-wrapper.hpp"
#include "protocol.hpp"


/**
 * @brief Client for connecting to the master server.
 *
 * This class handles the messaging with the master server
 * (which is responsible for announcing games).
 *
 * The main function is to get info on available games.
 * It is up to the application to request refresh, and
 * the list may also fill gradually, not completely at once.
 */
class MasterClient: public net::NetworkListener {
public:
	MasterClient();

	/// Connects to the master server
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	/// Updates the hosted game state to master server
	void updateGame(const std::string& name, const std::string& track, int players);

	/// Clears cache, requests new game listing, doesn't wait for it to arrive
	void refreshList();

	/// Returns cached list of games
	protocol::GameList getList() const { return m_games; };

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	mutable boost::mutex m_mutex;
	net::NetworkObject m_client;
	protocol::GameList m_games;
	int m_gameId;
};
