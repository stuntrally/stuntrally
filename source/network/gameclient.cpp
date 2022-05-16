#include "pch.h"
#include "gameclient.hpp"
#include "xtime.hpp"
#include "../ogre/common/Def_Str.h"

#include <MyGUI_LanguageManager.h>
#include <ctime>


P2PGameClient::P2PGameClient(GameClientCallback* callback, int port)
	: m_callback(callback)
	, m_client(*this, port), m_state(DISCONNECTED)
	, m_mutex(), m_cond()
	, m_playerInfo(), m_game(), m_carState()
{
	std::srand(time(0));  // randomize, based on current time
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
	if (m_callback)
	{
		PeerInfo pi;
		pi.name = m_playerInfo.name;
		m_callback->peerMessage(pi, msg);
	}
}

void P2PGameClient::startLobby()
{
	if (m_state == LOBBY)
	{
		LogO("== Netw startLobby  WRONG already");
		return;
	}else
		m_state = LOBBY;

	if (!m_senderThread.joinable())
		m_senderThread = boost::thread(boost::bind(&P2PGameClient::senderThread, boost::ref(*this)));
	else
		LogO("== Netw startLobby  WRONG? !m_senderThread.joinable()");
}

void P2PGameClient::startGame(bool broadcast)
{
	if (m_state == GAME)
	{
		LogO("== Netw startGame  WRONG already");
		return;
	}
	LogO("== Netw startGame");
	m_state = GAME;
	{
		//  Clean up all zombie peers
		boost::mutex::scoped_lock lock(m_mutex);

		PeerMap::iterator it = m_peers.begin();
		while (it != m_peers.end())
		{
			PeerInfo& pi = it->second;
			pi.loaded = false;  //  Reset loading status to be sure

			//  Check condition
			if (pi.connection != PeerInfo::CONNECTED || pi.name.empty() || !pi.authenticated)
			{
				if (pi.peer_id != 0)
					m_client.disconnect(pi.peer_id, true);
				m_peers.erase(it++);
			}else
				++it;
		}
		m_playerInfo.loaded = false;

		//  Assign id numbers
		recountPeersAndAssignIds(true);
	}
	//  Send notification
	if (broadcast)
	{
		LogO("== Netw startGame broadcast");
		m_client.broadcast(char(protocol::START_GAME) + std::string(" "), net::PACKET_RELIABLE);
	}else
		LogO("== Netw startGame !broadcast");
}

void P2PGameClient::loadingFinished()
{
	LogO("== Netw loadingFinished");
	m_client.broadcast(char(protocol::START_COUNTDOWN) + std::string(" "), net::PACKET_RELIABLE);
	boost::mutex::scoped_lock lock(m_mutex);
	m_playerInfo.loaded = true;

	// Check for callback, because we might be the last one to finish loading
	if (!m_callback)
	{
		LogO("== Netw loadingFinished  WRONG !m_callback");
		return;
	}
	for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
		if (!it->second.loaded)
		{
			LogO("== Netw loadingFinished (but others not ready yet)");
			return;
		}
	lock.unlock(); // Mutex unlocked in callback to avoid dead-locks

	// Wait a tiny bit to give the network some time
	boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	m_callback->startRace();
}

void P2PGameClient::lap(uint8_t num, double time)
{
	protocol::TimeInfoPackage tip(num, time);
	m_client.broadcast(net::convert(tip), net::PACKET_RELIABLE);
	LogO("== Netw Lap " +toStr(num) +" finished by U time:"+ toStr(float(time)));
}

void P2PGameClient::returnToLobby(bool broadcast)
{
	if (m_state != GAME)
	{
		LogO("== Netw returnToLobby  WRONG we should be in-game");
		return;
	}
	LogO("== Netw returnToLobby");
	boost::mutex::scoped_lock lock(m_mutex);
	m_state = LOBBY;
	m_playerInfo.loaded = false;
	m_playerInfo.ready = false;

	for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
		it->second.loaded = false;
	if (broadcast)
		m_client.broadcast(char(protocol::RETURN_LOBBY) + std::string(" "), net::PACKET_RELIABLE);
}

void P2PGameClient::senderThread()
{
	do
	{	boost::mutex::scoped_lock lock(m_mutex);
		if (m_state == LOBBY)
		{
			// Broadcast local player's meta info
			protocol::PlayerInfoPacket pip = (protocol::PlayerInfoPacket)m_playerInfo;
			broadcastToAuthed(net::convert(pip));

			// If game info is set, broadcast it
			if (m_game.packet_type == protocol::GAME_STATUS)
				broadcastToAuthed(net::convert(m_game));

			// Loop all peers
			for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
			{	PeerInfo& pi = it->second;

				// Check if we should try connecting to the peer
				// TODO: Should we also do this during GAME phase ?..
				if (pi.connection == PeerInfo::DISCONNECTED)
				{
					//LogO("== Netw  Connecting to "+pi.address);
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
		if (m_state == DISCONNECTED)
		{	//LogO("== Netw DISCONNECTED");
			break;
		}
	}
	while (true);
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
	for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.id == id)
			return it->second;
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
	LogO("== Netw recountPeersAndAssignIds");
	m_playerInfo.address = m_client.getAddress();
	m_playerInfo.peers = 0;
	m_playerInfo.id = -1;
	
	typedef std::map<int32_t, PeerInfo*> IDSorter;
	IDSorter idsorter;
	idsorter[m_playerInfo.random_id] = &m_playerInfo;

	for (PeerMap::iterator it = m_peers.begin(); it != m_peers.end(); ++it)
	{
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
	for (IDSorter::iterator it = idsorter.begin(); it != idsorter.end(); ++it, ++id)
		it->second->id = id;

	if (!validate)
	{	LogO("== Netw recountPeersAndAssignIds  !validate");
		return;
	}
	// Validate unique IDs
	for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.id == -1)
		{	LogO("== Netw recountPeersAndAssignIds  Unassigned or duplicate client ID!");
			throw std::runtime_error("Unassigned or duplicate client ID!");
		}
}

// Mutex should be already locked when this is called
void P2PGameClient::broadcastToAuthed(net::NetworkTraffic const& msg, int flags)
{
	for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
		if (it->second.authenticated)
			m_client.send(it->second.peer_id, msg, flags);
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	LogO("== Netw  Connected "+e.peer_address.str());
	if (m_state == LOBBY)
	{
		if (m_playerInfo.address == e.peer_address)
		{
			LogO("== Netw  CConnected  reject, same !!");
			return;
		}
		boost::mutex::scoped_lock lock(m_mutex);
		PeerInfo& pi = m_peers[e.peer_address.str()];
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
	LogO("== Netw  Disconnected "+e.peer_address.str());
	if (m_callback && e.event_data == protocol::INCOMPATIBLE_GAME_PROTOCOL)
		m_callback->error(TR("#{NetGameProtocolError}"));
	if (m_callback && e.event_data == protocol::WRONG_PASSWORD)
		m_callback->error(TR("#{NetWrongPassword}"));
	PeerInfo picopy;
	{
		boost::mutex::scoped_lock lock(m_mutex);
		picopy = m_peers[e.peer_address.str()];
		picopy.connection = PeerInfo::DISCONNECTED;
		m_peers.erase(e.peer_address.str());
		recountPeersAndAssignIds();
	}
	// Callback (mutex unlocked to avoid dead-locks)
	if (m_callback)
		m_callback->peerDisconnected(picopy);
}


void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	if (e.packet_length <= 0 || !e.packet_data)
	{
		LogO("== Netw receive event BAD");
		return;
	}
	switch (e.packet_data[0])
	{
		case protocol::HANDSHAKE:
		{
			if (m_state != LOBBY)
			{	LogO("== Netw HANDSHAKE  WRONG !lobby");
				break;
			}
			protocol::HandshakePackage hp = *reinterpret_cast<protocol::HandshakePackage const*>(e.packet_data);
			if (hp.game_protocol_version != protocol::GAME_PROTOCOL_VERSION)
			{
				m_client.disconnect(e.peer_id, false, protocol::INCOMPATIBLE_GAME_PROTOCOL);
				if (m_callback)
					m_callback->error(TR("#{NetGameProtocolError}"));
				LogO("== Netw HANDSHAKE  WRONG protocol ver");
				break;
			}
			boost::mutex::scoped_lock lock(m_mutex);
			if (std::string(m_playerInfo.password) != std::string(hp.password))
			{
				m_client.disconnect(e.peer_id, false, protocol::WRONG_PASSWORD);
				LogO("== Netw HANDSHAKE  WRONG passwod");
				break;
			}
			PeerInfo& pi = m_peers[e.peer_address.str()];
			pi.authenticated = true;
			pi.ping = e.ping;
			LogO("== Netw HANDSHAKE  ok");
		}	break;

		case protocol::PEER_ADDRESS:
		{
			if (m_state != LOBBY)
			{	LogO("== Netw PEER_ADDRESS  WRONG !lobby");
				break;
			}
			protocol::PeerAddressPacket pap = *reinterpret_cast<protocol::PeerAddressPacket const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);
			m_peers[pap.address.str()].address = pap.address;
			m_peers[e.peer_address.str()].ping = e.ping;
		}	break;

		case protocol::TEXT_MESSAGE:
		{
			std::string msg((const char*)e.packet_data, e.packet_length);
			msg = msg.substr(1);  // Remove the packet_type char
			LogO("== Netw TEXT_MESSAGE received: "+msg);
			if (m_callback)
			{
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address.str()];
				pi.ping = e.ping;

				PeerInfo picopy = pi;
				lock.unlock();  // Mutex unlocked in callback to avoid dead-locks
				m_callback->peerMessage(picopy, msg);
			}else
				LogO("== Netw TEXT_MESSAGE  !callback");
		}	break;

		case protocol::PLAYER_INFO:
		{
			LogO("== Netw PLAYER_INFO  start");
			protocol::PlayerInfoPacket pip = *reinterpret_cast<protocol::PlayerInfoPacket const*>(e.packet_data);

			if (std::string(pip.name) == m_playerInfo.name)
			{
				LogO("== Netw PLAYER_INFO  reject, same !!");
				break;
			}
			
			boost::mutex::scoped_lock lock(m_mutex);

			PeerInfo& pi = m_peers[e.peer_address.str()];
			bool isNew = pi.name.empty();

			//LogO("== Netw PI0  adr: "+pi.address.str()+"  name: "+pi.name+"  id: "+toStr(pi.id)+"  rid: "+toStr(pi.random_id)+"  peers: "+toStr(pi.peers)+"  rdy: "+toStr(pi.ready?1:0)+"  ld: "+toStr(pi.loaded?1:0)+"  ping: "+toStr(pi.ping)+"  aut: "+toStr(pi.authenticated));
			pi = pip;
			pi.ping = e.ping;
			//LogO("== Netw PI1  adr: "+pi.address.str()+"  name: "+pi.name+"  id: "+toStr(pi.id)+"  rid: "+toStr(pi.random_id)+"  peers: "+toStr(pi.peers)+"  rdy: "+toStr(pi.ready?1:0)+"  ld: "+toStr(pi.loaded?1:0)+"  ping: "+toStr(pi.ping)+"  aut: "+toStr(pi.authenticated));

			if (m_callback)  // Callback
			{
				LogO(std::string("== Netw PI  callb  ")+(isNew?"new":""));
				PeerInfo picopy = pi;
				if (isNew)
					recountPeersAndAssignIds();

				lock.unlock();  // Mutex unlocked in callback to avoid dead-locks

				if (isNew)	// First info means completed connection
					m_callback->peerConnected(picopy);
				else	// Callback regardless if the info changed in order to give ping updates
					m_callback->peerInfo(picopy);
			}else
				LogO("== Netw START_GAME  !callback");
		}	break;

		case protocol::START_GAME:
		{
			LogO("== Netw START_GAME  start");
			startGame(false);
			if (m_callback)
			{
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address.str()];
				pi.ping = e.ping;

				PeerInfo picopy = pi;
				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				m_callback->peerState(picopy, e.packet_data[0]);
				LogO("== Netw START_GAME  end");
			}else
				LogO("== Netw START_GAME  WRONG !callback");
		}	break;

		case protocol::START_COUNTDOWN:
		{
			LogO("== Netw START_COUNTDOWN  start");
			if (m_callback)
			{
				boost::mutex::scoped_lock lock(m_mutex);
				PeerInfo& pi = m_peers[e.peer_address.str()];
				pi.ping = e.ping;
				pi.loaded = true;
				if (!m_playerInfo.loaded)
				{
					LogO("== Netw START_COUNTDOWN  WRONG !m_playerInfo.loaded");
					break;
				}

				bool start = true;
				for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
					if (!it->second.loaded)
					{	start = false;	break;	}

				lock.unlock(); // Mutex unlocked in callback to avoid dead-locks
				if (start)
				{
					LogO("== Netw START_COUNTDOWN  startRace");
					m_callback->startRace();
				}
				LogO("== Netw START_COUNTDOWN  end");
			}else
				LogO("== Netw START_COUNTDOWN  WRONG !callback");
		}	break;

		case protocol::CAR_UPDATE:
		{
			if (m_state != GAME)
			{	LogO("== Netw CAR_UPDATE  WRONG !game");
				break;
			}
			protocol::CarStatePackage csp = *reinterpret_cast<protocol::CarStatePackage const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);

			PeerInfo& pi = m_peers[e.peer_address.str()];
			pi.ping = e.ping;
			if (pi.id < 0)
			{	LogO("== Netw CAR_UPDATE  WRONG pi.id < 0");
				break;
			}
			m_receivedCarStates[pi.id] = csp;
		}	break;

		case protocol::GAME_STATUS:
		{
			if (m_state != LOBBY || !m_callback)
			{	LogO("== Netw GAME_STATUS  WRONG !lobby or !callback");
				break;
			}
			protocol::GameInfo game = *reinterpret_cast<protocol::GameInfo const*>(e.packet_data);
			m_callback->gameInfo(game);
		}	break;

		case protocol::TIME_INFO:
		{
			if (m_state != GAME || !m_callback)
			{	LogO("== Netw TIME_INFO  WRONG !game or !callback");
				break;
			}
			protocol::TimeInfoPackage time = *reinterpret_cast<protocol::TimeInfoPackage const*>(e.packet_data);
			boost::mutex::scoped_lock lock(m_mutex);

			ClientID id = m_peers[e.peer_address.str()].id;
			lock.unlock();  // Mutex unlocked in callback to avoid dead-locks
			m_callback->timeInfo(id, time.lap, time.time);
		}	break;

		case protocol::RETURN_LOBBY:
		{
			returnToLobby(false);
			m_callback->returnToLobby();
		}	break;

		default:
			LogO("== Netw  Received unknown packet type: "+toStr((int)e.packet_data[0]));
			break;
	}
}
