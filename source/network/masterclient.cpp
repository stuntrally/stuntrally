#include "masterclient.hpp"


MasterClient::MasterClient(): m_mutex(), m_client(*this), m_gameId(0)
{
}

void MasterClient::connect(const std::string& address, int port)
{
	m_client.connect(address, port);
}

void MasterClient::refreshList()
{
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_games.clear();
	}
	protocol::GameInfo game;
	game.packet_type = protocol::GAME_LIST;
	m_client.broadcast(game, net::PACKET_RELIABLE);
}

void MasterClient::updateGame(const std::string& name, const std::string& track, int players)
{
	protocol::GameInfo game;
	game.packet_type = protocol::GAME_STATUS;
	game.id = m_gameId;
	// FIXME: This memcpy stuff is really hairy
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
			boost::mutex::scoped_lock lock(m_mutex);
			m_games[game.id] = game;
			break;
		}
		case protocol::GAME_ACCEPTED: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_gameId = game.id;
			std::cout << "Game accepted with id " << m_gameId << std::endl;
			break;
		}
		default: {
			std::cout << "Unknown packet type received (MasterClient)" << std::endl;
			break;
		}
	}
}
