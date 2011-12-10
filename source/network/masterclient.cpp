#include "masterclient.hpp"
#include "xtime.hpp"


MasterClient::MasterClient(MasterClientCallback* callback, int updateInterval)
	: m_callback(callback), m_mutex(), m_cond(), m_client(*this), m_game(), m_updateInterval(updateInterval), m_sendUpdates()
{
	m_game.packet_type = protocol::GAME_STATUS;
}

MasterClient::~MasterClient()
{
	terminate();
}

void MasterClient::connect(const std::string& address, int port)
{
	m_client.connect(address, port, NULL, protocol::MASTER_PROTOCOL_VERSION);
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
	// Callback for cleared list in order to show the user something happened
	if (m_callback) m_callback->gameListChanged(m_games);
}

void MasterClient::updateGame(const protocol::GameInfo &game, std::string password)
{
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_game = game;
		m_password = password;
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
	m_cond.notify_all();
	m_gameInfoSenderThread.join();
}

void MasterClient::gameInfoSenderThread()
{
	while (m_sendUpdates) {
		// Broadcast info
		boost::mutex::scoped_lock lock(m_mutex);
		m_client.broadcast(m_game, net::PACKET_RELIABLE);
		// Wait some
		m_cond.timed_wait(lock, now() + (m_updateInterval / 1000.0));
	}
}

void MasterClient::connectionEvent(net::NetworkTraffic const& e)
{
	if (e.event_data != protocol::MASTER_PROTOCOL_VERSION) {
		std::cout << "Incompatible protocol versions "
				<< "(my: " << protocol::MASTER_PROTOCOL_VERSION
				<< " hers: " << e.event_data << ")!" << std::endl;
		// TODO: Real disconnect
		terminate();
		return;
	}
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
			// Update a game in the list
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);
			m_games[game.id] = game;
			if (m_callback) m_callback->gameListChanged(m_games);
			break;
		}
		case protocol::GAME_ACCEPTED: {
			// Confirmation of accepted game and its id
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_game.id = game.id;
			break;
		}
		default: {
			std::cout << "Unknown packet type received (MasterClient)" << std::endl;
			break;
		}
	}
}
