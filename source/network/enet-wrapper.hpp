#pragma once

/**
 * @file
 * Networking library wrapping ENet.
 *
 * The aim is to hide ENet behind an object-oriented, threaded API
 * that could theoretically be implemented with another networking
 * library without changes to the user's code.
 * 
 * Requires linking agains ENet 1.3.x and boost_thread libraries.
 * 
 * QUICKGUIDE:
 *   1. Create ENetContainer library and keep it in scope
 *   2. Create a class inheriting NetworkListener and implement the functions
 *   3. Create a NetworkObject, giving it an instance of your NetworkListener
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <enet/enet.h>

#include "address.hpp"
#include "types.hpp"

// Version check
#if ENET_VERSION < ENET_VERSION_CREATE(1,3,0)
	#error ENet versions below 1.3.0 not supported
#endif

namespace net {

	const unsigned ENetChannels = 3; // 0 = Sequenced, 1 = Reliable, 2 = Unsequenced

	typedef enet_uint16 peer_id_t;

	enum PacketFlags {
		PACKET_SEQUENCED = 0,
		PACKET_RELIABLE = 1,
		PACKET_UNSEQUENCED = 2
	};

	/**
	 * RAII Wrapper for the library
	 *
	 * Create an instance when you want to initialize networking and
	 * keep it in scope during the time you need it. Deinit is automatic
	 * upon destruction.
	 */
	struct ENetContainer {
		ENetContainer() { enet_initialize(); }
		~ENetContainer() { enet_deinitialize(); }
	};

	/// Convert ENetPeer to net::Address
	inline Address convert(const ENetPeer* peer) {
		if (peer) return Address(peer->address.host, peer->address.port);
		else return Address();
	}

	/// Convert ENetAddress to net::Address
	inline Address convert(const ENetAddress addr) {
		return Address(addr.host, addr.port);
	}

	/// Network traffic container
	struct NetworkTraffic {
		NetworkTraffic(const enet_uint8* pckd = NULL, size_t pckl = 0, enet_uint32 evdata = 0):
			peer_id(0), peer_address(), peer_data(NULL), packet_data(pckd),
			packet_length(pckl), ping(0), event_data(evdata) {}
		NetworkTraffic(ENetPeer* peer, void* dptr, const enet_uint8* pckd = NULL, size_t pckl = 0, enet_uint32 evdata = 0):
			peer_id(peer->incomingPeerID), peer_address(convert(peer)), peer_data(dptr), packet_data(pckd),
			packet_length(pckl), ping(peer->roundTripTime), event_data(evdata) {}
		peer_id_t peer_id; ///< Peer ID assigned by the library
		Address peer_address; ///< Address from which the peer connected
		void* peer_data; ///< User data associated with the peer that sent the traffic
		const enet_uint8* packet_data; ///< The actual packet data (empty for connect/disconnect events)
		size_t packet_length; ///< Length of the packet data in bytes
		unsigned ping; ///< Average round-trip time to the peer
		enet_uint32 event_data; ///< Data associated with the event
	};

	// Serialization functions

	/*template <typename T>
	enet_uint8* convert(T& obj) {
		return reinterpret_cast<enet_uint8*>(&obj);
	}*/

	/*template <typename T>
	enet_uint8 const* convert(const T& obj) {
		return reinterpret_cast<enet_uint8 const*>(&obj);
	}*/

	template <typename T>
	NetworkTraffic convert(T& obj) {
		return NetworkTraffic(reinterpret_cast<enet_uint8*>(&obj), sizeof(T));
	}

	/*template <typename T>
	const NetworkTraffic convert(const T& obj) {
		return NetworkTraffic(reinterpret_cast<enet_uint8 const*>(&obj), sizeof(T));
	}*/

	/**
	 * @brief Callback class prototype
	 *
	 * Inherit this class and implement the functions to get network events.
	 * The events are sent from another thread, but there will not be two
	 * simultaneous events.
	 */
	class NetworkListener {
	public:
		/// Someone connected
		/// @param e the traffic associated with the event
		virtual void connectionEvent(NetworkTraffic const& e) {}
		/// Someone disconnected
		/// @param e the traffic associated with the event
		virtual void disconnectEvent(NetworkTraffic const& e) {}
		/// Someone sent some data
		/// @param e the traffic associated with the event
		virtual void receiveEvent(NetworkTraffic const& e) {}
	};


	/**
	 * Networking class capable of acting as client or server
	 *
	 * This is the main object that provides an object-oriented API
	 * for the underlying low-level networking implementation.
	 *
	 * A thread is used so that the networking events are asynchronous.
	 * There is no need for user to poll - use NetworkListener class instead.
	 *
	 * Connections are automatically terminated upon destruction of the instance.
	 *
	 * This class is thread-safe.
	 */
	class NetworkObject: public boost::noncopyable {
	public:

		/**
		 * Constructor.
		 * @param listener class to receive network events
		 * @param port the port for listening for new connections, defaults to any port
		 * @param data optional application specific data to associate with the NetworkObject instance
		 * @param max_connections maximum number of peers
		 */
		NetworkObject(NetworkListener& listener, int port = -1, void* data = NULL, int max_connections = 16): m_quit(false), m_host(NULL), m_listener(listener), m_data(data)
		{
			m_address.host = ENET_HOST_ANY;
			m_address.port = port < 0 ? ENET_PORT_ANY : port;
			// Create host at address, max_conns, unlimited up/down bandwith
			m_host = enet_host_create(&m_address, max_connections, ENetChannels, 0, 0);
			if (!m_host) throw std::runtime_error("An error occurred while trying to create an ENet host.");
			// Start listener thread
			m_thread = boost::thread(boost::bind(&NetworkObject::listen, boost::ref(*this)));
		}

		/// Destructor.
		~NetworkObject() {
			terminate();
			// Gracefully disconnect all peers
			boost::mutex::scoped_lock lock(m_mutex);
			if (m_host) {
				ENetEvent e;
				enet_host_service(m_host, &e, 5); // Dummy service to allow for sending pending messages
				for (size_t i = 0; i < m_host->peerCount; ++i)
					enet_peer_disconnect(&m_host->peers[i], 0);
				enet_host_destroy(m_host);
				m_host = NULL;
			}
		}

		/// Server thread, do not call directly
		void listen() {
			ENetEvent e;
			while (!m_quit) {
				{
					boost::mutex::scoped_lock lock(m_mutex);
					enet_host_service(m_host, &e, 5);
				}
				switch (e.type) {
					case ENET_EVENT_TYPE_NONE: {
						break;
					} case ENET_EVENT_TYPE_CONNECT: {
						m_peers[e.peer->incomingPeerID] = e.peer;
						m_listener.connectionEvent(NetworkTraffic(e.peer, e.peer->data, NULL, 0, e.data));
						break;
					} case ENET_EVENT_TYPE_DISCONNECT: {
						m_listener.disconnectEvent(NetworkTraffic(e.peer, e.peer->data, NULL, 0, e.data));
						e.peer->data = NULL;
						m_peers.erase(e.peer->incomingPeerID);
						break;
					} case ENET_EVENT_TYPE_RECEIVE: {
						m_listener.receiveEvent(NetworkTraffic(e.peer, e.peer->data, e.packet->data, e.packet->dataLength, e.data));
						enet_packet_destroy(e.packet); // Clean-up
						break;
					}
				}
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		/**
		 * Connect to a peer.
		 * @param host the address to connect to
		 * @param port the port to connect to
		 * @param data application specific data that can be retrieved in events
		 */
		void connect(const std::string& host, int port, void* data = NULL, uint32_t connection_data = 0) {
			// Set properties
			ENetAddress address;
			enet_address_set_host(&address, host.c_str());
			address.port = port;
			// Initiate the connection
			ENetPeer* peer = NULL;
			boost::mutex::scoped_lock lock(m_mutex);
			peer = enet_host_connect(m_host, &address, ENetChannels, connection_data);
			if (!peer) throw std::runtime_error("No available peers for initiating an ENet connection.");
			peer->data = data;
		}

		/**
		 * Connect to a peer.
		 * @param addr the address to connect to
		 * @param data application specific data that can be retrieved in events
		 */
		void connect(const Address& addr, void* data = NULL, uint32_t connection_data = 0) {
			// Set properties
			ENetAddress address;
			address.host = addr.host;
			address.port = addr.port;
			// Initiate the connection
			ENetPeer* peer = NULL;
			boost::mutex::scoped_lock lock(m_mutex);
			peer = enet_host_connect(m_host, &address, ENetChannels, connection_data);
			if (!peer) throw std::runtime_error("No available peers for initiating an ENet connection.");
			peer->data = data;
		}

		/// Send a packet to everyone
		void broadcast(const NetworkTraffic& msg, int flags = 0) {
			ENetPacket* packet = enet_packet_create(msg.packet_data, msg.packet_length, flags);
			// Pick channel for sending and broadcast to all peers
			int channel = 0;
			if (flags & PACKET_RELIABLE) channel = 1;
			else if (flags & PACKET_UNSEQUENCED) channel = 2;
			// Send
			boost::mutex::scoped_lock lock(m_mutex);
			enet_host_broadcast(m_host, channel, packet);
		}

		/// Send a string to everyone
		void broadcast(const std::string& msg, int flags = 0) {
			NetworkTraffic traffic(reinterpret_cast<const enet_uint8*>(msg.c_str()), msg.length());
			broadcast(traffic, flags);
		}

		/// Send a packet to a specific peer
		void send(peer_id_t peer_id, const NetworkTraffic& msg, int flags = 0) {
			ENetPeer* peer = getPeerPtr(peer_id);
			if (!peer) return;
			ENetPacket* packet = enet_packet_create(msg.packet_data, msg.packet_length, flags);
			// Pick channel for sending and broadcast to all peers
			int channel = 0;
			if (flags & PACKET_RELIABLE) channel = 1;
			else if (flags & PACKET_UNSEQUENCED) channel = 2;
			// Send
			boost::mutex::scoped_lock lock(m_mutex);
			enet_peer_send(peer, channel, packet);
		}

		/// Send a string to a specific peer
		void send(peer_id_t peer_id, const std::string& msg, int flags = 0) {
			NetworkTraffic traffic(reinterpret_cast<const enet_uint8*>(msg.c_str()), msg.length());
			send(peer_id, traffic, flags);
		}

		/// Disconnects the given peer
		void disconnect(peer_id_t peer_id, bool force = false, uint32_t disconnection_data = 0) {
			ENetPeer* peer = getPeerPtr(peer_id);
			if (!peer) return;
			if (force) enet_peer_disconnect_now(peer, disconnection_data);
			else enet_peer_disconnect(peer, disconnection_data);
		}

		/// Returns a raw ENet peer pointer
		ENetPeer* getPeerPtr(peer_id_t peer_id) {
			Peers::iterator it = m_peers.find(peer_id);
			if (it != m_peers.end()) return it->second;
			return NULL;
		}

		/// Get address of the local host
		Address getAddress() const {
			boost::mutex::scoped_lock lock(m_mutex);
			return convert(m_address);
		}

		/// Terminate the network thread. Automatically called upon destruction.
		void terminate() { m_quit = true; m_thread.join(); }

	private:
		bool m_quit;
		ENetAddress m_address;
		ENetHost* m_host;
		typedef std::map<peer_id_t, ENetPeer*> Peers;
		Peers m_peers;
		mutable boost::mutex m_mutex;
		boost::thread m_thread;
		NetworkListener& m_listener;
		void* m_data;
	};
}
