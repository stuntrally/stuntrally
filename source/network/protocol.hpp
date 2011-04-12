#pragma once
/**
 * This file describes the protocol that is used for network communication.
 * It contains structures and serializations that are transmitted.
 */

namespace protocol {

const unsigned DEFAULT_PORT = 4243;

/**
 * This enum contains all possible message types.
 * It will be transmitted as char.
 */
enum PacketType {
	HANDSHAKE,
	PING,
	PONG,
	REQUEST_PEER_INFO,
	PEER_INFO,
	TEXT_MESSAGE,
	STATE_UPDATE
};


/**
 * Contains the car state.
 */
struct CarStatePackage {
	float x, y, z; // FIXME: Should we use some vector types?
	float vx, vy, vz;
	float rotx, roty, rotz; // FIXME: What's the best way to transmit orientation?

	CarStatePackage(/* TODO: Parameters. */) {}
};

}
