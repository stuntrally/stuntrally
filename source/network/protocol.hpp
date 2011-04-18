#pragma once
/**
 * This file describes the protocol that is used for network communication.
 * It contains structures and serializations that are transmitted.
 */

#include <stdint.h>

namespace protocol {

const unsigned DEFAULT_PORT = 4243;

/**
 * This enum contains all possible message types.
 * It will be transmitted as char.
 */
enum PacketType {
	HANDSHAKE = 0,
	PING,
	PONG,
	REQUEST_PEER_INFO,
	PEER_INFO,
	TEXT_MESSAGE,
	STATE_UPDATE,
	REQUEST_GAME_LIST,
	UPDATE_GAME_STATUS
};

struct Packet {
	PacketType type;
	uint32_t length;
	uint8_t* data;

	Packet(PacketType t, uint32_t l, uint8_t* d): type(t), length(l), data(d) {}
};

struct GameInfo {
	uint32_t id;
	std::string name;
	std::string address;
	uint8_t players;
	std::string track;
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
