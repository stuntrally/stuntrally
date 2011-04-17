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

	const unsigned ENetChannels = 4;
	const unsigned ENetMaxConnections = 16;

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
		NetworkTraffic(unsigned id = 0, void* dptr = NULL, enet_uint8* pckd = NULL, size_t pckl = 0):
			peer_id(id), peer_data(dptr), packet_data(pckd), packet_length(pckl) {}
		unsigned peer_id;
		void* peer_data;
		enet_uint8* packet_data;
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
		NetworkObject(NetworkListener& listener, int port = -1, void* data = NULL): m_quit(false), m_host(NULL), m_listener(listener), m_data(data)
		{
			m_address.host = ENET_HOST_ANY;
			m_address.port = port < 0 ? ENET_PORT_ANY : port;
			// Create host at address, max_conns, unlimited up/down bandwith
			m_host = enet_host_create(&m_address, ENetMaxConnections, ENetChannels, 0, 0);
			if (m_host == NULL)
				throw std::runtime_error("An error occurred while trying to create an ENet host.");
			// Start listener thread
			m_thread = boost::thread(boost::bind(&NetworkObject::listen, boost::ref(*this)));
		}

		~NetworkObject() {
			terminate();
			if (m_host) enet_host_destroy(m_host);
			m_host = NULL;
		}

		void listen() {
			ENetEvent e;
			while (!m_quit) {
				{	// Lock mutex
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

		/// Connect to a peer
		void connect(std::string host, int port) {
			// Set properties
			ENetAddress address;
			enet_address_set_host(&address, host.c_str());
			address.port = port;
			// Lock
			boost::mutex::scoped_lock lock(m_mutex);
			// Initiate the connection
			ENetPeer* peer = NULL;
			peer = enet_host_connect(m_host, &address, ENetChannels, 0);
			if (peer == NULL)
				throw std::runtime_error("No available peers for initiating an ENet connection.");
			// TODO: Handle peers in listen()
			m_peers.push_back(peer);
		}

		/// Send a string to everyone
		//  TODO: Use separate channel for unordered/ordered packets
		void broadcast(const std::string& msg, int flag = 0) {
			ENetPacket* packet = enet_packet_create(msg.c_str(), msg.length(), flag);
			boost::mutex::scoped_lock lock(m_mutex);
			//if (m_peer) enet_peer_send(m_peer, 0, packet); // Send to peer through channel 0
			enet_host_broadcast(m_host, 0, packet); // Send through channel 0 to all peers
		}

		/// Send a char to everyone
		void broadcast(const char& msg, int flag = 0) {
			broadcast(std::string(1, msg), flag);
		}

		void terminate() { m_quit = true; m_thread.join(); }

	  protected:
		bool m_quit;
		ENetAddress m_address;
		ENetHost* m_host;
		typedef std::list<ENetPeer*> Peers;
		Peers m_peers;
		mutable boost::mutex m_mutex;
		boost::thread m_thread;
		NetworkListener& m_listener;
		void* m_data;
	};
}
