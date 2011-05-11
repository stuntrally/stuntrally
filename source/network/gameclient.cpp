#include "gameclient.hpp"
#include "xtime.hpp"


P2PGameClient::P2PGameClient(GameClientCallback* callback, int port)
	: m_callback(callback), m_client(*this, port), m_state(DISCONNECTED), m_mutex(), m_cond(), m_playerInfo()
{
}

P2PGameClient::~P2PGameClient()
{
	// Shuts down possibly running threads
	m_state = DISCONNECTED;
	m_cond.notify_all();
	m_peerInfoSenderThread.join();
}

void P2PGameClient::connect(const std::string& address, int port)
{
	startLobby();
	m_client.connect(address, port);
}

void P2PGameClient::updatePlayerInfo(const std::string& name, const std::string& car)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.name = name;
	m_playerInfo.car = car;
}

void P2PGameClient::toggleReady()
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.ready = !m_playerInfo.ready;
}


bool P2PGameClient::isReady() const
{
	return m_playerInfo.ready;
}


void P2PGameClient::sendPeerInfo()
{
	boost::mutex::scoped_lock lock(m_mutex);
	// Send the player info
	protocol::PlayerInfoPacket pip = (protocol::PlayerInfoPacket)m_playerInfo;
	m_client.broadcast(pip);
	// Peer address info sending
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
		protocol::PeerAddressPacket pap(it->second.address);
		m_client.broadcast(pap);
	}
}

void P2PGameClient::sendMessage(const std::string& msg)
{
	m_client.broadcast(char(protocol::TEXT_MESSAGE) + msg, net::PACKET_RELIABLE);
	if (m_callback) {
		PeerInfo pi;
		pi.name = m_playerInfo.name;
		m_callback->peerMessage(pi, msg);
	}
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
		if (pi.connection != PeerInfo::CONNECTED || pi.name.empty()) {
			m_peers.erase(it++);
		} else ++it;
	}
}

void P2PGameClient::peerInfoSenderThread() {
	while (m_state == LOBBY) {
		// Broadcast info
		sendPeerInfo();
		// Check if we should try connecting to someone
		{
			boost::mutex::scoped_lock lock(m_mutex);
			for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
				PeerInfo& pi = it->second;
				if (pi.connection == PeerInfo::DISCONNECTED) {
					std::cout << "Connecting to " << pi.address << std::endl;
					pi.connection = PeerInfo::CONNECTING;
					m_client.connect(pi.address);
				}
			}
			// Wait some
			m_cond.timed_wait(lock, now() + 2.0);
		}
	}
}

size_t P2PGameClient::getPeerCount() const
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_playerInfo.peers;
}

// Mutex should be already locked when this is called
void P2PGameClient::recountPeers()
{
	m_playerInfo.peers = 0;
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
		if (it->second.connection == PeerInfo::CONNECTED && !it->second.name.empty())
			++m_playerInfo.peers;
	}
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connection address=" << e.peer_address << "   id=" << e.peer_id << std::endl;
	if (m_state == LOBBY) {
		boost::mutex::scoped_lock lock(m_mutex);
		PeerInfo& pi = m_peers[e.peer_address];
		pi.address = e.peer_address;
		pi.connection = PeerInfo::CONNECTED;
		pi.ping = e.ping;
		// No connection callback here, because player info is not yet received
	}
	// We'll send the peer info periodically, so no need to do it here
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected address=" << e.peer_address << "   id=" << e.peer_id << std::endl;
	PeerInfo picopy;
	{
		boost::mutex::scoped_lock lock(m_mutex);
		// TODO: Maybe just delete it?
		m_peers[e.peer_address].connection = PeerInfo::DISCONNECTED;
		picopy = m_peers[e.peer_address];
		recountPeers();
	}
	// Callback (mutex unlocked to avoid dead-locks)
	if (m_callback) m_callback->peerDisconnected(picopy);
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::PEER_ADDRESS: {
			if (m_state != LOBBY) break;
			protocol::PeerAddressPacket pap = *reinterpret_cast<protocol::PeerAddressPacket const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);
			m_peers[pap.address].address = pap.address;
			m_peers[e.peer_address].ping = e.ping;
			break;
		}
		case protocol::TEXT_MESSAGE: {
			std::string msg((const char*)e.packet_data, e.packet_length);
			std::cout << "Text message received: " << msg << std::endl;
			if (m_callback) {
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address];
				pi.ping = e.ping;
				PeerInfo picopy = pi;
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				m_callback->peerMessage(picopy, msg);
			}
			break;
		}
		case protocol::PLAYER_INFO: {
			protocol::PlayerInfoPacket pip = *reinterpret_cast<protocol::PlayerInfoPacket const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);
			PeerInfo& pi = m_peers[e.peer_address];
			bool isNew = pi.name.empty();
			pi = pip;
			pi.ping = e.ping;
			// Callback
			if (m_callback) {
				PeerInfo picopy = pi;
				if (isNew) recountPeers();
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				// First info means completed connection
				if (isNew) m_callback->peerConnected(picopy);
				// Callback regardless if the info changed in order to give ping updates
				else m_callback->peerInfo(picopy);
			}
			break;
		}
		default: {
			std::cout << "Received unknown packet type: " << (int)e.packet_data[0] << std::endl;
			break;
		}
	}
}
