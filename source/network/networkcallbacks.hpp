#pragma once

/**
 * @file
 * Contains callback classes for the network clients.
 * Inherit these to receive networking events.
 */
 
#include "protocol.hpp"
#include "peerinfo.hpp"

/**
 * @brief Callback class for MasterClient events.
 *
 * Inherit and implement this to get events about arriving game
 * info. Pass the implementation instance to the MasterClient.
 */
struct MasterClientCallback {
	/// Called when the list has changed
	/// @param list how the complete list looks now after the change
	virtual void gameListChanged(protocol::GameList list) {}
};


/**
 * @brief Callback class for P2PGameClient events.
 *
 * Inherit and implement this to get events about arriving peer
 * info. Pass the implementation instance to the P2PGameClient.
 */
struct GameClientCallback
{
	/// Called when there is a new connection
	/// @param peer the new peer
	virtual void peerConnected(PeerInfo peer) {}

	/// Called when a peer disconnects
	/// @param peer the disconnected peer
	virtual void peerDisconnected(PeerInfo peer) {}

	/// Called when a player info is received
	/// @param peer the updated peer
	virtual void peerInfo(PeerInfo peer) {}

	/// Called when a text message arrives
	/// @param peer the new peer
	/// @param msg the message body
	virtual void peerMessage(PeerInfo peer, std::string msg) {}

	/// Called when a state change arrives
	/// @param peer the sender peer
	/// @param state the state code
	virtual void peerState(PeerInfo peer, uint8_t state) {}

	/// Called when game info update arrives
	/// @param peer the updated peer
	virtual void gameInfo(protocol::GameInfo game) {}

	/// Called when all peers have finished loading and are ready to start racing
	virtual void startRace() {}

	/// Called when timing info has arrived
	virtual void timeInfo(ClientID id, uint8_t lap, double time) {}
	
	/// Called when host closed game (continue on new track)
	virtual void returnToLobby() {}

	/// Called when an error occured
	/// @param what the error description
	virtual void error(std::string what) {}
};
