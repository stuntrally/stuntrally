#include "network.hpp"


P2PGameClient::P2PGameClient(int port): m_client(*this, port)
{
	//TODO
}

void P2PGameClient::connect(const std::string& address, int port)
{
	m_client.connect(address, port);
}

void P2PGameClient::broadcast(const std::string& msg)
{
	//protocol::Packet packet(protocol::TEXT_MESSAGE, msg.length(), msg.c_str());
	m_client.broadcast(msg);
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Connection id=" << e.peer_id << std::endl;
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Disconnected id=" << e.peer_id << std::endl;
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Traffic id=" << e.peer_id << ", content: " << std::string((char*)e.packet_data, e.packet_length) << std::endl;
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
	std::cout << "Connection to master server established." << std::endl;
}

void MasterClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected from master server." << std::endl;
}

void MasterClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::GAME_STATUS: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			std::cout << "Available game: " << game.name << std::endl;
		}
		case protocol::GAME_ACCEPTED: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_gameId = game.id;
			std::cout << "Game accepted with id " << m_gameId << std::endl;
		}
		default: {
			std::cout << "Unknown packet type received." << std::endl;
		}
	}
}
