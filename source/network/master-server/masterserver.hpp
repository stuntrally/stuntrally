#pragma once

/**
 * @file
 * This file describes the protocol used for master server communication.
 */

#include <boost/lexical_cast.hpp>
#include "../types.hpp"
#include "../address.hpp"

namespace masterserver {

const uint32_t PROTOCOL_VERSION = 6;

const unsigned DEFAULT_PORT = 4242;

/**
 * @brief Contains all possible message types.
 * It will be transmitted as 8-bit unsigned int.
 */
enum PacketType {
	LIST,          // Client requests master server to list games
	UPDATE,        // An available game (either client updates, or server reports)
	START,         // Client signals a game is started
	SUCCESS,       // Ack
	ERROR          // Error response
};

/**
 * @brief Contains error codes that tell the reason of failures.
 */
enum ErrorCodes {
	NO_ERROR = 0,
	INCOMPATIBLE_PROTOCOL_VERSION,
	INVALID_REQUEST,
	SERVER_ERROR
};

/**
 * @brief Contains information about one game that is available for joining.
 */
struct GameInfo {
	uint16_t version;
	uint8_t packet_type;
	uint32_t id;        // Set by server
	uint32_t address;   // Set by server
	uint16_t port;      // Set by client
	uint32_t timestamp; // Set by server

	uint8_t num_properties;
	void* payload;
	
	GameInfo(): version(PROTOCOL_VERSION) {}
};

typedef std::map<uint32_t, GameInfo> GameList;

} // namespace masterserver
