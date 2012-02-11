#include "gameclient.hpp"
#include "xtime.hpp"

namespace {
	std::string formulateProtocolVersionError(uint32_t remote_version) {
		std::ostringstream oss;
		oss << "Connection refused due to incompatible protocol versions "
			<< "(my: " << protocol::GAME_PROTOCOL_VERSION
			<< " hers: " << remote_version << ")!";
		return oss.str();
	}
}


P2PGameClient::P2PGameClient(GameClientCallback* callback, int port)
	: m_callback(callback), m_client(*this, port), m_state(DISCONNECTED), m_mutex(), m_cond(), m_playerInfo(), m_game(), m_carState()
{
	m_playerInfo.random_id = std::rand(); // Client id is based on this
	m_game.packet_type = -1; // Invalidate until set
	m_carState.packet_type = -1; // Invalidate until set
}

P2PGameClient::~P2PGameClient()
{
	// Shuts down possibly running threads
	m_state = DISCONNECTED;
	m_cond.notify_all();
	m_senderThread.join();
}

void P2PGameClient::connect(const std::string& address, int port, std::string password)
{
	setPassword(password);
	startLobby();
	m_client.connect(address, port, NULL);
}

void P2PGameClient::updatePlayerInfo(const std::string& name, const std::string& car)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.name = name;
	m_playerInfo.car = car;
	m_playerInfo.address = m_client.getAddress();
}

void P2PGameClient::setPassword(const std::string& password)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.password = password;
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
	if (!m_senderThread.joinable())
		m_senderThread = boost::thread(boost::bind(&P2PGameClient::senderThread, boost::ref(*this)));
}

void P2PGameClient::startGame(bool broadcast)
{
	if (m_state == GAME) return;
	m_state = GAME;
	{
		// Clean up all zombie peers
		boost::mutex::scoped_lock lock(m_mutex);
		PeerMap::iterator it = m_peers.begin();
		while (it != m_peers.end()) {
			PeerInfo& pi = it->second;
			pi.loaded = false; // Reset loading status to be sure
			// Check condition
			if (pi.connection != PeerInfo::CONNECTED || pi.name.empty() || !pi.authenticated) {
				if (pi.peer_id != 0)
					m_client.disconnect(pi.peer_id, true);
				m_peers.erase(it++);
			}
			else ++it;
		}
		m_playerInfo.loaded = false;
		// Assign id numbers
		recountPeersAndAssignIds(true);
	}
	// Send notification
	if (broadcast)
		m_client.broadcast(char(protocol::START_GAME) + std::string(" "), net::PACKET_RELIABLE);
}

void P2PGameClient::loadingFinished()
{
	m_client.broadcast(char(protocol::START_COUNTDOWN) + std::string(" "), net::PACKET_RELIABLE);
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.loaded = true;
	// Check for callback, because we might be the last one to finish loading
	if (!m_callback) return;
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it)
		if (!it->second.loaded) return;
	lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
	// Wait a tiny bit to give the network some time
	boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	m_callback->startRace();
}

void P2PGameClient::senderThread() {
	do {
		boost::mutex::scoped_lock lock(m_mutex);
		if (m_state == LOBBY)
		{
			// Broadcast local player's meta info
			protocol::PlayerInfoPacket pip = (protocol::PlayerInfoPacket)m_playerInfo;
			broadcastToAuthed(net::convert(pip));
			// If game info is set, broadcast it
			if (m_game.packet_type == protocol::GAME_STATUS)
				broadcastToAuthed(net::convert(m_game));
			// Loop all peers
			for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
				PeerInfo& pi = it->second;
				// Check if we should try connecting to the peer
				// TODO: Should we also do this during GAME phase
				if (pi.connection == PeerInfo::DISCONNECTED) {
					std::cout << "Connecting to " << pi.address << std::endl;
					pi.connection = PeerInfo::CONNECTING;
					m_client.connect(pi.address, NULL);
				}
				// Broadcast peer's info
				protocol::PeerAddressPacket pap(it->second.address);
				broadcastToAuthed(net::convert(pap));
			}
			// Wait some
			m_cond.timed_wait(lock, now() + 2.0);
		}
		else if (m_state == GAME)
		{
			// Broadcast car state
			if ((bool)m_carState)
				m_client.broadcast(net::convert(m_carState));
			// Wait some
			m_cond.timed_wait(lock, now() + 0.050); // 20 FPS
		}
		if (m_state == DISCONNECTED) break;
	} while (true);
}

void P2PGameClient::setLocalCarState(protocol::CarStatePackage const& cs)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_carState = cs;
}

protocol::CarStates P2PGameClient::getReceivedCarStates()
{
	boost::mutex::scoped_lock lock(m_mutex);
	protocol::CarStates states = m_receivedCarStates;
	m_receivedCarStates.clear();
	return states;
}

size_t P2PGameClient::getPeerCount() const
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_playerInfo.peers;
}

PeerInfo P2PGameClient::getPeer(ClientID id) const
{
	boost::mutex::scoped_lock lock(m_mutex);
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.id == id) return it->second;
	return PeerInfo();
}

void P2PGameClient::broadcastGameInfo(const protocol::GameInfo &game)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_game = game;
	m_game.packet_type = protocol::GAME_STATUS; // Just to make sure
}

// Mutex should be already locked when this is called
void P2PGameClient::recountPeersAndAssignIds(bool validate)
{
	m_playerInfo.address = m_client.getAddress();
	m_playerInfo.peers = 0;
	m_playerInfo.id = -1;
	typedef std::map<int32_t, PeerInfo*> IDSorter;
	IDSorter idsorter;
	idsorter[m_playerInfo.random_id] = &m_playerInfo;
	for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it) {
		if (it->second.connection == PeerInfo::CONNECTED
			&& !it->second.name.empty()
			&& it->second.authenticated)
		{
			idsorter[it->second.random_id] = &it->second;
			++m_playerInfo.peers;
		}
		it->second.id = -1;
	}
	// Assign IDs
	ClientID id = 0;
	for (IDSorter::iterator it = idsorter.begin(); it != idsorter.end(); ++it, ++id) {
		it->second->id = id;
	}
	if (!validate) return;
	// Validate unique IDs
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.id == -1)
			throw std::runtime_error("Unassigned or duplicate client ID!");
}

// Mutex should be already locked when this is called
void P2PGameClient::broadcastToAuthed(net::NetworkTraffic const& msg, int flags)
{
	for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.authenticated) m_client.send(it->second.peer_id, msg, flags);
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	std::cout << "Connected " << e.peer_address << std::endl;
	if (m_state == LOBBY) {
		boost::mutex::scoped_lock lock(m_mutex);
		PeerInfo& pi = m_peers[e.peer_address];
		pi.address = e.peer_address;
		pi.peer_id = e.peer_id;
		pi.connection = PeerInfo::CONNECTED;
		pi.ping = e.ping;
		protocol::HandshakePackage hp(m_playerInfo.password);
		m_client.send(e.peer_id, net::convert(hp), net::PACKET_RELIABLE);
		// No connection callback here, because player info is not yet received
	}
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	std::cout << "Disconnected " << e.peer_address << std::endl;
	if (m_callback && e.event_data == protocol::INCOMPATIBLE_GAME_PROTOCOL)
		m_callback->error(formulateProtocolVersionError(e.event_data));
	if (m_callback && e.event_data == protocol::WRONG_PASSWORD)
		m_callback->error("Wrong password.");
	PeerInfo picopy;
	{
		boost::mutex::scoped_lock lock(m_mutex);
		// We probably don't want to delete it right away,
		// since we could try reconnecting
		m_peers[e.peer_address].connection = PeerInfo::DISCONNECTED;
		picopy = m_peers[e.peer_address];
		recountPeersAndAssignIds();
	}
	// Callback (mutex unlocked to avoid dead-locks)
	if (m_callback) m_callback->peerDisconnected(picopy);
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data) return;
	switch (e.packet_data[0]) {
		case protocol::HANDSHAKE: {
			if (m_state != LOBBY) break;
			protocol::HandshakePackage hp = *reinterpret_cast<protocol::HandshakePackage const*>(e.packet_data);
			if (hp.game_protocol_version != protocol::GAME_PROTOCOL_VERSION) {
				m_client.disconnect(e.peer_id, false, protocol::INCOMPATIBLE_GAME_PROTOCOL);
				if (m_callback) m_callback->error(formulateProtocolVersionError(e.event_data));
				break;
			}
			boost::mutex::scoped_lock lock(m_mutex);
			if (std::string(m_playerInfo.password) != std::string(hp.password)) {
				m_client.disconnect(e.peer_id, false, protocol::WRONG_PASSWORD);
				break;
			}
			PeerInfo& pi = m_peers[e.peer_address];
			pi.authenticated = true;
			pi.ping = e.ping;
			break;
		}
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
				if (isNew) recountPeersAndAssignIds();
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				// First info means completed connection
				if (isNew) m_callback->peerConnected(picopy);
				// Callback regardless if the info changed in order to give ping updates
				else m_callback->peerInfo(picopy);
			}
			break;
		}
		case protocol::START_GAME: {
			startGame(false);
			if (m_callback) {
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address];
				pi.ping = e.ping;
				PeerInfo picopy = pi;
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				m_callback->peerState(picopy, e.packet_data[0]);
			}
			break;
		}
		case protocol::START_COUNTDOWN: {
			if (m_callback) {
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address];
				pi.ping = e.ping;
				pi.loaded = true;
				if (!m_playerInfo.loaded) break;
				for (PeerMap::const_iterator it = m_peers.begin(); it != m_peers.end(); ++it)
					if (!it->second.loaded) break;
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				m_callback->startRace();
			}
			break;
		}
		case protocol::CAR_UPDATE: {
			if (m_state != GAME) break;
			protocol::CarStatePackage csp = *reinterpret_cast<protocol::CarStatePackage const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);
			PeerInfo& pi = m_peers[e.peer_address];
			pi.ping = e.ping;
			if (pi.id < 0) break;
			m_receivedCarStates[pi.id] = csp;
			break;
		}
		case protocol::GAME_STATUS: {
			if (m_state != LOBBY) break;
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			if (m_callback) m_callback->gameInfo(game);
			break;
		}
		default: {
			std::cout << "Received unknown packet type: " << (int)e.packet_data[0] << std::endl;
			break;
		}
	}
}
