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
	enum ConnectionState { DISCONNECTED = 0, CONNECTING = 1, CONNECTED = 2 } connection;

	PeerInfo(net::Address addr = net::Address()): address(addr), name(), connection(DISCONNECTED) {}
};

typedef std::map<std::string, PeerInfo> PeerMap;


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
