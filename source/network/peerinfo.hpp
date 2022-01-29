#pragma once

/**
 * @file
 * Class containing the "meta data" about a peer.
 */

#include "protocol.hpp"

typedef int8_t ClientID;

/**
 * @brief Application specific peer information.
 */
struct PeerInfo
{
	ClientID id; ///< ID number, used with car state updates
	uint16_t peer_id; ///< ID associated by ENet
	int32_t random_id; ///< random number used to determine id
	net::Address address; ///< Address

	std::string name; ///< Nickname
	std::string car; ///< Car
	std::string password; ///< Password for connecting

	short peers; ///< Amount of peers connected
	bool ready; ///< Ready state
	bool loaded; ///< Can start race?
	unsigned ping; ///< Average packet round-trip time
	bool authenticated; ///< Handshaking completed?
	enum ConnectionState { DISCONNECTED = 0, CONNECTING = 1, CONNECTED = 2 } connection; ///< Connection state

	PeerInfo(net::Address addr = net::Address())
		:id(-1), random_id(-1), peer_id(0), address(addr), name(), car(), peers(), ready()
		,loaded(), ping(0), authenticated(), connection(DISCONNECTED)
	{
	}

	PeerInfo& operator=(const protocol::PlayerInfoPacket& pip)
	{
		random_id = pip.random_id;
		name = std::string(pip.name); car = std::string(pip.car); password = std::string(pip.password);
		peers = pip.peers; ready = pip.ready;
		return *this;
	}

	/* Not currently used, so disabled to avoid accidents
	bool operator==(const protocol::PlayerInfoPacket& pip)
	{
		return name == std::string(pip.name) && car == std::string(pip.car) && peers == pip.peers && ready == pip.ready;
	}
	bool operator!=(const protocol::PlayerInfoPacket& pip)
	{	return !(*this == pip);  }
	*/

	operator protocol::PlayerInfoPacket()
	{
		protocol::PlayerInfoPacket pip;
		memcpy(pip.name, name.c_str(), 16);
		memcpy(pip.car, car.c_str(), 10);
		memcpy(pip.password, password.c_str(), 16);
		pip.random_id = random_id;
		pip.peers = peers;
		pip.ready = ready;
		return pip;
	}
};

typedef std::map<std::string, PeerInfo> PeerMap;
