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
	net::Address address; ///< Address
	std::string name; ///< Nickname
	std::string car; ///< Car
	short peers; ///< Amount of peers connected
	bool ready; ///< Ready state
	unsigned ping; ///< Average packet round-trip time
	enum ConnectionState { DISCONNECTED = 0, CONNECTING = 1, CONNECTED = 2 } connection; ///< Connection state

	PeerInfo(net::Address addr = net::Address()): address(addr), name(), car(), ready(), ping(0), connection(DISCONNECTED) {}

	PeerInfo& operator=(const protocol::PlayerInfoPacket& pip) {
		name = pip.name; car = pip.car; ready = pip.ready;
		return *this;
	}

	operator protocol::PlayerInfoPacket() {
		protocol::PlayerInfoPacket pip;
		// FIXME: Yack, memcpy
		memcpy(pip.name, name.c_str(), 16);
		memcpy(pip.car, car.c_str(), 10);
		pip.peers = peers;
		pip.ready = ready;
	}
};

typedef std::map<std::string, PeerInfo> PeerMap;


/**
 * @brief Callback class for P2PGameClient events.
 *
 * Inherit and implement this to get events about arriving peer
 * info. Pass the implementation instance to the P2PGameClient.
 */
struct GameClientCallback {
	/// Called when there is a new connection
	/// @param peer the new peer
	virtual void peerConnected(PeerInfo peer) {};

	/// Called when a peer disconnects
	/// @param peer the disconnected peer
	virtual void peerDisconnected(PeerInfo peer) {};

	/// Called when a text message arrives
	/// @param peer the new peer
	/// @param msg the message body
	virtual void peerMessage(PeerInfo peer, std::string msg) {};
};


/**
 * @brief High-level networking implentation using p2p.
 *
 * This class is responsible for forming a peer-to-peer network,
 * maintaining it and handling high-level messaging between the peers.
 */
class P2PGameClient: public net::NetworkListener {
public:
	P2PGameClient(const std::string& nickname, GameClientCallback* callback = NULL, int port = protocol::DEFAULT_PORT);

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

	/// Thread that periodically broadcasts peer info, don't call directly
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
	GameClientCallback* m_callback;
	net::NetworkObject m_client;
	PeerMap m_peers;
	enum State { DISCONNECTED, LOBBY, GAME } m_state;
	boost::thread m_peerInfoSenderThread;
	mutable boost::mutex m_mutex;
	const std::string& m_name;
};
