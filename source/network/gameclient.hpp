#pragma once

/**
 * @file
 * This file contains the game specific networking client classes.
 */

#include <boost/thread/condition.hpp>
#include "enet-wrapper.hpp"
#include "protocol.hpp"
#include "peerinfo.hpp"
#include "networkcallbacks.hpp"

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
class P2PGameClient: public net::NetworkListener
{
public:
	enum State { DISCONNECTED, LOBBY, GAME };

	P2PGameClient(GameClientCallback* callback = NULL, int port = protocol::DEFAULT_PORT);

	~P2PGameClient();

	/// Connects to a peer
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT, std::string password = "");

	/// Updates the player info
	void updatePlayerInfo(const std::string& name, const std::string& car);

	/// Updates password (for host)
	void setPassword(const std::string& password);

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

	/// Signal that loading has finished
	void loadingFinished();

	/// Signal a lap with time
	void lap(uint8_t num, double time);

	/// After race, go back to lobby for another race
	/// Host should call this externally, others will call it through a network message
	/// @param broadcast set to true if the state change should be broadcasted to peers
	void returnToLobby(bool broadcast = true);

	/// Thread that periodically broadcasts peer and game state info, don't call directly
	void senderThread();

	/// How many peers are connected
	size_t getPeerCount() const;

	/// Get copy of peer infos
	PeerMap getPeers() const { return m_peers; }

	/// Get copy of peer info by id
	PeerInfo getPeer(ClientID id) const;

	/// Return client state
	State getState() const { return m_state; }

	/// Returns the id
	ClientID getId() const { return m_playerInfo.id; }

	/// Sets and sends the game info (for hosts)
	void broadcastGameInfo(protocol::GameInfo const& game);

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
	/// Check how many connected and introduced peers there are and assign ids
	void recountPeersAndAssignIds(bool validate = false);

	/// Send something but only to authed peers
	void broadcastToAuthed(net::NetworkTraffic const& msg, int flags = 0);

	GameClientCallback* m_callback;
	net::NetworkObject m_client;
	
	PeerMap m_peers;
	protocol::CarStates m_receivedCarStates;
	State m_state;
	
	boost::thread m_senderThread;
	mutable boost::mutex m_mutex;
	boost::condition m_cond;
	
	PeerInfo m_playerInfo;
	protocol::GameInfo m_game;
	protocol::CarStatePackage m_carState;
};
