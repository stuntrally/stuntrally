#pragma once

/**
 * @file
 * This file contains the game specific networking client classes.
 */

#include "enet-wrapper.hpp"
#include "protocol.hpp"


/**
 * @brief Application specific peer information.
 */
struct PeerInfo {
	net::Address address;
	std::string name;
	bool connected;

	PeerInfo(net::Address addr = net::Address()): address(addr), name("Unknown"), connected(false) {}
};

typedef std::map<net::peer_id_t, PeerInfo> PeerMap;


/**
 * @brief High-level networking implentation using p2p.
 *
 * This class is responsible for forming a peer-to-peer network,
 * maintaining it and handling high-level messaging between the peers.
 */
class P2PGameClient: public net::NetworkListener {
public:
	P2PGameClient(const std::string& nickname, int port = protocol::DEFAULT_PORT);

	~P2PGameClient();

	/// Connects to a peer
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	/// Broadcasts all known peer infos to every one connected
	void sendPeerInfo();

	/// Broadcasts a chat message to all peers
	void sendMessage(const std::string& msg);

	/// Starts the network forming phase
	void startLobby();

	/// Shuts down the network forming phase
	void startGame();

	/// Thread that periodically proadcasts peer info, don't call directly
	void peerInfoSenderThread();

	/// How many peers are connected
	size_t getPeerCount() { return m_peers.size(); }

	/// Get copy of peer infos
	PeerMap getPeers() { return m_peers; }

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	net::NetworkObject m_client;
	PeerMap m_peers;
	enum State { DISCONNECTED, LOBBY, GAME } m_state;
	boost::thread m_peerInfoSenderThread;
	mutable boost::mutex m_mutex;
	const std::string& m_name;
};


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
