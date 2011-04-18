#pragma once

/**
 * Networking library wrapping ENet.
 * 
 * Requires linking agains ENet and boost_thread libraries.
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
#include <enet/enet.h>

namespace net {

	const unsigned ENetChannels = 3; // 0 = Sequenced, 1 = Reliable, 2 = Unsequenced
	const unsigned ENetMaxConnections = 16;

	typedef enet_uint16 peer_id_t;

	enum PacketFlags {
		PACKET_SEQUENCED = 0,
		PACKET_RELIABLE = 1,
		PACKET_UNSEQUENCED = 2
	};

	/// Convert integer IPv4 address to dot-notation
	inline std::string IPv4(unsigned i) {
		std::ostringstream oss;
		oss << (i & 0xFF) << "." << ((i >> 8) & 0xFF) << "." << ((i >> 16) & 0xFF) << "." << ((i >> 24) & 0xFF);
		return oss.str();
	}

	/// RAII Wrapper for the library
	struct ENetContainer {
		ENetContainer() { enet_initialize(); }
		~ENetContainer() { enet_deinitialize(); }
	};


	/// Network traffic container
	struct NetworkTraffic {
		NetworkTraffic(const enet_uint8* pckd = NULL, size_t pckl = 0):
			peer_id(0), peer_data(NULL), packet_data(pckd), packet_length(pckl) {}
		NetworkTraffic(peer_id_t id, void* dptr, const enet_uint8* pckd = NULL, size_t pckl = 0):
			peer_id(id), peer_data(dptr), packet_data(pckd), packet_length(pckl) {}
		peer_id_t peer_id;
		void* peer_data;
		const enet_uint8* packet_data;
		size_t packet_length;
	};

	/// Callback class prototype
	class NetworkListener {
	  public:
		virtual void connectionEvent(NetworkTraffic const& e) {}
		virtual void disconnectEvent(NetworkTraffic const& e) {}
		virtual void receiveEvent(NetworkTraffic const& e) {}
	};


	/// Common networking stuff for both server and client
	class NetworkObject: public boost::noncopyable {
	public:

		/**
		 * Constructor.
		 * @param listener class to receive network events
		 * @param port the port for listening for new connections, defaults to any port
		 * @param data optional application specific data to associate with the NetworkObject instance
		 */
		NetworkObject(NetworkListener& listener, int port = -1, void* data = NULL): m_quit(false), m_host(NULL), m_listener(listener), m_data(data)
		{
			m_address.host = ENET_HOST_ANY;
			m_address.port = port < 0 ? ENET_PORT_ANY : port;
			// Create host at address, max_conns, unlimited up/down bandwith
			m_host = enet_host_create(&m_address, ENetMaxConnections, ENetChannels, 0, 0);
			if (!m_host) throw std::runtime_error("An error occurred while trying to create an ENet host.");
			// Start listener thread
			m_thread = boost::thread(boost::bind(&NetworkObject::listen, boost::ref(*this)));
		}

		/// Destructor.
		~NetworkObject() {
			terminate();
			if (m_host) enet_host_destroy(m_host);
			m_host = NULL;
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
					case ENET_EVENT_TYPE_NONE:
						break;
					case ENET_EVENT_TYPE_CONNECT: {
						std::cout << "Connected " << IPv4(e.peer->address.host) << ":" << e.peer->address.port << std::endl;
						m_listener.connectionEvent(NetworkTraffic(e.peer->incomingPeerID, e.peer->data));
						break;
					} case ENET_EVENT_TYPE_DISCONNECT: {
						std::cout << "Disconnected " << IPv4(e.peer->address.host) << ":" << e.peer->address.port << std::endl;
						m_listener.disconnectEvent(NetworkTraffic(e.peer->incomingPeerID, e.peer->data));
						e.peer->data = NULL;
						break;
					} case ENET_EVENT_TYPE_RECEIVE: {
						m_listener.receiveEvent(NetworkTraffic(e.peer->incomingPeerID, e.peer->data, e.packet->data, e.packet->dataLength));
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
		void connect(std::string host, int port, void* data = NULL) {
			// Set properties
			ENetAddress address;
			enet_address_set_host(&address, host.c_str());
			address.port = port;
			// Initiate the connection
			ENetPeer* peer = NULL;
			boost::mutex::scoped_lock lock(m_mutex);
			peer = enet_host_connect(m_host, &address, ENetChannels, 0);
			if (!peer) throw std::runtime_error("No available peers for initiating an ENet connection.");
			peer->data = data;
			// TODO: Handle peers in listen()
			m_peers[peer->incomingPeerID] = peer;
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

		ENetPeer* getPeerPtr(peer_id_t peer_id) {
			Peers::iterator it = m_peers.find(peer_id);
			if (it != m_peers.end()) return it->second;
			return NULL;
		}

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
