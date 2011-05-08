#include "masterclient.hpp"


MasterClient::MasterClient(MasterClientCallback* callback, int updateInterval)
	: m_callback(callback), m_mutex(), m_client(*this), m_game(), m_updateInterval(updateInterval), m_sendUpdates()
{
	m_game.packet_type = protocol::GAME_STATUS;
}

MasterClient::~MasterClient()
{
	terminate();
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
	if (m_callback) m_callback->listChanged(m_games);
}

void MasterClient::updateGame(const std::string& name, const std::string& track, int players, int port)
{
	{
		boost::mutex::scoped_lock lock(m_mutex);
		// FIXME: This memcpy stuff is really hairy
		memcpy(m_game.name, name.c_str(), 32);
		memcpy(m_game.track, track.c_str(), 32);
		m_game.players = players;
		m_game.port = port;
		m_sendUpdates = true;
	}
	// Start updater thread if it is not already running
	if (!m_gameInfoSenderThread.joinable())
		m_gameInfoSenderThread = boost::thread(boost::bind(&MasterClient::gameInfoSenderThread, boost::ref(*this)));
}

void MasterClient::terminate()
{
	// Shuts down possibly running thread
	m_sendUpdates = false;
	m_gameInfoSenderThread.join();
}

void MasterClient::gameInfoSenderThread()
{
	while (m_sendUpdates) {
		{
			// Broadcast info
			boost::mutex::scoped_lock lock(m_mutex);
			m_client.broadcast(m_game, net::PACKET_RELIABLE);
		}
		// Wait some
		// FIXME: Use Conditions to get rid of the wait time in case of destruction
		boost::this_thread::sleep(boost::posix_time::milliseconds(m_updateInterval));
	}
}

void MasterClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connection to master server established" << std::endl;
	// Send refresh request automatically after connection is established
	refreshList();
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
			if (m_callback) m_callback->listChanged(m_games);
			break;
		}
		case protocol::GAME_ACCEPTED: {
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_game.id = game.id;
			std::cout << "Game accepted with id " << m_game.id << std::endl;
			break;
		}
		default: {
			std::cout << "Unknown packet type received (MasterClient)" << std::endl;
			break;
		}
	}
}
