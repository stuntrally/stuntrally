#pragma once
/**
 * This file describes the protocol that is used for network communication.
 * It contains structures and serializations that are transmitted.
 */

#include <stdint.h>
#include "enet-wrapper.hpp"

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
	TEXT_MESSAGE,       // Text string that should be displayed somewhere
	STATE_UPDATE,
	GAME_LIST,          // Client requests master server to list games
	GAME_ACCEPTED,      // Master server sends response for newly accepted games
	GAME_STATUS         // An available game (either client updates, or server reports)
};

struct Packet {
	PacketType type;
	uint32_t length;
	uint8_t* data;

	Packet(PacketType t, uint32_t l, uint8_t* d): type(t), length(l), data(d) {}
};


/**
 * Contains information about one game that is available for joining.
 */
struct GameInfo: public net::SimpleSerializer<GameInfo> {
	uint8_t packet_type;
	uint32_t id;        // Set by server
	uint32_t address;   // Set by server
	uint16_t port;      // Set by server
	uint8_t players;    // Set by client
	char name[32];      // Set by client
	char track[32];     // Set by client
};

typedef std::map<uint32_t, protocol::GameInfo> GameList;


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
