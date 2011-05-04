#include "network.hpp"


P2PGameClient::P2PGameClient(const std::string& nickname, int port): m_client(*this, port), m_state(DISCONNECTED), m_mutex(), m_name(nickname)
{
}

P2PGameClient::~P2PGameClient()
{
	// Shuts down possibly running threads
	m_state = DISCONNECTED;
}

void P2PGameClient::connect(const std::string& address, int port)
{
	startLobby();
	m_client.connect(address, port);
}

void P2PGameClient::sendPeerInfo()
{
	// Send my nickname
	// TODO: Better place to do this?
	m_client.broadcast(char(protocol::NICK) + m_name);
	// Peer address info sending
	boost::mutex::scoped_lock lock(m_mutex);
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
		protocol::PeerAddressPacket pap(it->second.address);
		m_client.broadcast(pap, net::PACKET_RELIABLE);
	}
}

void P2PGameClient::sendMessage(const std::string& msg)
{
	m_client.broadcast(char(protocol::TEXT_MESSAGE) + msg);
}

void P2PGameClient::startLobby()
{
	if (m_state == LOBBY) return;
	else m_state = LOBBY;
	if (!m_peerInfoSenderThread.joinable())
		m_peerInfoSenderThread = boost::thread(boost::bind(&P2PGameClient::peerInfoSenderThread, boost::ref(*this)));
}

void P2PGameClient::startGame()
{
	m_state = GAME;
	// Clean up all zombie peers
	boost::mutex::scoped_lock lock(m_mutex);
	PeerMap::iterator it = m_peers.begin();
	while (it != m_peers.end()) {
		PeerInfo pi = it->second;
		// Check condition
		if (!pi.connected || pi.name.empty()) m_peers.erase(it++);
		else ++it;
	}
}

void P2PGameClient::peerInfoSenderThread() {
	std::cout << "Started peerInfoSenderThread" << std::endl;
	while (m_state == LOBBY) {
		// Check if we should try connecting to someone
		{
			boost::mutex::scoped_lock lock(m_mutex);
			for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
				PeerInfo pi = it->second;
				if (!pi.connected) {
					std::cout << "Connecting to " << pi.address << std::endl;
					m_client.connect(pi.address);
				}
			}
		}
		// Wait some
		boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
		// Broadcast info
		sendPeerInfo();
	}
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connection address=" << e.peer_address << "   id=" << e.peer_id << std::endl;
	if (m_state == LOBBY) {
		boost::mutex::scoped_lock lock(m_mutex);
		PeerInfo& pi = m_peers[e.peer_address];
		pi.address = e.peer_address;
		pi.connected = true;
	}
	// We'll send the peer info periodically, so no need to do it here
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected address=" << e.peer_address << "   id=" << e.peer_id << std::endl;
	boost::mutex::scoped_lock lock(m_mutex);
	m_peers.erase(e.peer_address);
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::PEER_INFO: {
			if (m_state != LOBBY) break;
			protocol::PeerAddressPacket pap = *reinterpret_cast<protocol::PeerAddressPacket const*>(e.packet_data);
			std::cout << "Peer info received for " << pap.address << std::endl;
			boost::mutex::scoped_lock lock(m_mutex);
			m_peers[pap.address].address = pap.address;
			break;
		}
		case protocol::TEXT_MESSAGE: {
			std::string msg((const char*)e.packet_data, e.packet_length);
			std::cout << "Text message received: " << msg << std::endl;
			break;
		}
		case protocol::NICK: {
			if (e.packet_length > 1) {
				std::string nick((const char*)e.packet_data + 1, e.packet_length - 1);
				std::cout << "Nick received: " << nick << std::endl;
				boost::mutex::scoped_lock lock(m_mutex);
				m_peers[e.peer_address].name = nick;
			}
			break;
		}
		default: {
			std::cout << "Received unknown packet type: " << (int)e.packet_data[0] << std::endl;
			break;
		}
	}
}




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
