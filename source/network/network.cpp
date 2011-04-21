#include "network.hpp"


P2PGameClient::P2PGameClient(int port): m_client(*this, port)
{
	//TODO
}

void P2PGameClient::connect(const std::string& address, int port)
{
	m_client.connect(address, port);
}


void P2PGameClient::sendPeerInfo()
{
	for (protocol::PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
		m_client.broadcast(it->second, net::PACKET_RELIABLE);
	}
}

void P2PGameClient::sendMessage(const std::string& msg)
{
	m_client.broadcast(char(protocol::TEXT_MESSAGE) + msg);
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connection id=" << e.peer_id << std::endl;
	// We'll send the peer info periodically, so no need to do it here
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected id=" << e.peer_id << std::endl;
	m_peers.erase(boost::lexical_cast<std::string>(e.peer_id));
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	std::cout << "Traffic from id=" << e.peer_id << std::endl;
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::PEER_INFO: {
			protocol::PeerInfo pi = *reinterpret_cast<protocol::PeerInfo const*>(e.packet_data);
			// TODO: Check for local address
			m_peers[boost::lexical_cast<std::string>(e.peer_id)] = pi;
			std::cout << "Peer info received for " << pi.name << std::endl;
			break;
		}
		case protocol::TEXT_MESSAGE: {
			std::string msg((const char*)e.packet_data, e.packet_length);
			std::cout << "Text message received: " << msg << std::endl;
			break;
		}
		default: {
			std::cout << "Received unknown packet type: " << (int)e.packet_data[0] << std::endl;
			break;
		}
	}
}




MasterClient::MasterClient(): m_client(*this), m_gameId(0)
{
	//TODO
}

void MasterClient::connect(const std::string& address, int port)
{
	m_client.connect(address, port);
}

void MasterClient::refreshList()
{
	protocol::GameInfo game;
	game.packet_type = protocol::GAME_LIST;
	m_client.broadcast(game, net::PACKET_RELIABLE);
}

void MasterClient::updateGame(const std::string& name, const std::string& track, int players)
{
	protocol::GameInfo game;
	game.packet_type = protocol::GAME_STATUS;
	game.id = m_gameId;
	memcpy(game.name, name.c_str(), 32);
	memcpy(game.track, track.c_str(), 32);
	game.players = players;
	m_client.broadcast(game, net::PACKET_RELIABLE);
}

void MasterClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connection to master server established" << std::endl;
}

void MasterClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected from master server" << std::endl;
}

void MasterClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::GAME_STATUS: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			std::cout << "Available game: " << game.name << std::endl;
			break;
		}
		case protocol::GAME_ACCEPTED: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_gameId = game.id;
			std::cout << "Game accepted with id " << m_gameId << std::endl;
			break;
		}
		default: {
			std::cout << "Unknown packet type received" << std::endl;
			break;
		}
	}
}
