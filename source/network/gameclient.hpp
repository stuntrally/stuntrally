#pragma once

/**
 * @file
 * This file contains the game specific networking client classes.
 */

#include <boost/thread/condition.hpp>
#include "enet-wrapper.hpp"
#include "protocol.hpp"


/**
 * @brief Application specific peer information.
 */
struct PeerInfo {
	int8_t id; ///< ID number, used with car state updates
	net::Address address; ///< Address
	std::string name; ///< Nickname
	std::string car; ///< Car
	std::string password; ///< Password for connecting
	short peers; ///< Amount of peers connected
	bool ready; ///< Ready state
	unsigned ping; ///< Average packet round-trip time
	enum ConnectionState { DISCONNECTED = 0, CONNECTING = 1, CONNECTED = 2 } connection; ///< Connection state

	PeerInfo(net::Address addr = net::Address()): id(-1), address(addr), name(), car(), peers(), ready(), ping(0), connection(DISCONNECTED) {}

	PeerInfo& operator=(const protocol::PlayerInfoPacket& pip) {
		name = std::string(pip.name); car = std::string(pip.car); password = std::string(pip.password);
		peers = pip.peers; ready = pip.ready;
		return *this;
	}

	/* Not currently used, so disabled to avoid accidents
	bool operator==(const protocol::PlayerInfoPacket& pip) {
		return name == std::string(pip.name) && car == std::string(pip.car) && peers == pip.peers && ready == pip.ready;
	}
	bool operator!=(const protocol::PlayerInfoPacket& pip) { return !(*this == pip); }
	*/

	operator protocol::PlayerInfoPacket() {
		protocol::PlayerInfoPacket pip;
		// FIXME: Yack, memcpy
		memcpy(pip.name, name.c_str(), 16);
		memcpy(pip.car, car.c_str(), 10);
		memcpy(pip.password, password.c_str(), 16);
		pip.peers = peers;
		pip.ready = ready;
		return pip;
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

	/// Called when a player info is received
	/// @param peer the updated peer
	virtual void peerInfo(PeerInfo peer) {};

	/// Called when a text message arrives
	/// @param peer the new peer
	/// @param msg the message body
	virtual void peerMessage(PeerInfo peer, std::string msg) {};

	/// Called when a state change arrives
	/// @param peer the sender peer
	/// @param state the state code
	virtual void peerState(PeerInfo peer, uint8_t state) {};
};


/**
 * @brief High-level networking implentation using p2p.
 *
 * This class is responsible for forming a peer-to-peer network,
 * maintaining it and handling high-level messaging between the peers.
 *
 * To get events, give it a GameClientCallback instance pointer.
 * Note that the events are received from a separate thread.
 *
 * This class is thread-safe.
 */
class P2PGameClient: public net::NetworkListener {
public:
	enum State { DISCONNECTED, LOBBY, GAME };

	P2PGameClient(GameClientCallback* callback = NULL, int port = protocol::DEFAULT_PORT);

	~P2PGameClient();

	/// Connects to a peer
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT, std::string password = "");

	/// Updates the player info
	void updatePlayerInfo(const std::string& name, const std::string& car);

	/// Toggles the ready switch
	void toggleReady();

	/// Returns the ready state
	bool isReady() const;

	/// Broadcasts a chat message to all peers
	void sendMessage(const std::string& msg);

	/// Starts the network forming phase
	void startLobby();

	/// Shuts down the network forming phase
	/// @param broadcast set to true if the state change should be broadcasted to peers
	void startGame(bool broadcast = true);

	/// Thread that periodically broadcasts peer and game state info, don't call directly
	void senderThread();

	/// How many peers are connected
	size_t getPeerCount() const;

	/// Get copy of peer infos
	PeerMap getPeers() const { return m_peers; }

	/// Return client state
	State getState() const { return m_state; }

	/// Update the local car state
	void setLocalCarState(protocol::CarStatePackage const& cs);

	/// Return the latest unhandled car states and clear them
	protocol::CarStates getReceivedCarStates();

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	void recountPeers();

	GameClientCallback* m_callback;
	net::NetworkObject m_client;
	PeerMap m_peers;
	protocol::CarStates m_receivedCarStates;
	State m_state;
	boost::thread m_senderThread;
	mutable boost::mutex m_mutex;
	boost::condition m_cond;
	PeerInfo m_playerInfo;
	protocol::CarStatePackage m_carState;
};
