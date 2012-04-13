#include "masterclient.hpp"
#include "xtime.hpp"
#include "../ogre/common/Defines.h"

#include <MyGUI_LanguageManager.h>


MasterClient::MasterClient(MasterClientCallback* callback, int updateInterval)
	: m_callback(callback), m_mutex(), m_cond(), m_client(*this), m_game(),
	m_password(), m_error(), m_updateInterval(updateInterval), m_sendUpdates(), m_connectionOk()
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
	if (!m_connectionOk)
		return;
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_games.clear();
	}
	protocol::GameInfo game;
	game.packet_type = protocol::GAME_LIST;
	m_client.broadcast(net::convert(game), net::PACKET_RELIABLE);
	// Callback for cleared list in order to show the user something happened
	if (m_callback) m_callback->gameListChanged(m_games);
}

void MasterClient::updateGame(const protocol::GameInfo &game, std::string password)
{
	{
		boost::mutex::scoped_lock lock(m_mutex);
		uint32_t id = m_game.id;
		m_game = game;
		m_game.id = id;
		m_password = password;
		m_sendUpdates = true;
	}
	// Start updater thread if it is not already running
	if (!m_gameInfoSenderThread.joinable())
		m_gameInfoSenderThread = boost::thread(boost::bind(&MasterClient::gameInfoSenderThread, boost::ref(*this)));
}

void MasterClient::signalStart()
{
	if (m_connectionOk)
		m_client.broadcast(char(protocol::START_GAME) + std::string(" "), net::PACKET_RELIABLE);
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
		if (m_connectionOk)
			m_client.broadcast(net::convert(m_game), net::PACKET_RELIABLE);
		// Wait some
		m_cond.timed_wait(lock, now() + (m_updateInterval / 1000.0));
	}
}

std::string MasterClient::getError()
{
	boost::mutex::scoped_lock lock(m_mutex);
	std::string error = m_error;
	m_error = "";
	return error;
}

void MasterClient::connectionEvent(net::NetworkTraffic const& e)
{
	LogO("== Netw Connection to master server established");
	// We send any requests only after master server acknowledges protocol version
}

void MasterClient::disconnectEvent(net::NetworkTraffic const& e)
{
	if (e.event_data == protocol::INCOMPATIBLE_MASTER_PROTOCOL) {
		boost::mutex::scoped_lock lock(m_mutex);
		m_error = TR("#{NetMasterProtocolError}");
	} else if (e.event_data == 0 && !m_connectionOk) {
		// If we got here, we couldn't reach the master server
		boost::mutex::scoped_lock lock(m_mutex);
		m_error = TR("#{NetMasterConnectionError}");
	}
	LogO("== Netw Disconnected from master server");
}

void MasterClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::HANDSHAKE: {
			// Handshake means everything is ok
			m_connectionOk = true; // This should be atomic
			LogO("== Netw Master server connection accepted");
			refreshList(); // Auto-request new list
			break;
		}
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
