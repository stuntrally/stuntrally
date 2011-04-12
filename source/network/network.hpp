#pragma once

/**
 * Networking library wrapping ENet.
 * 
 * Requires linking agains ENet and boost_thread libraries.
 * 
 * QUICKGUIDE:
 *   1. Create ENetContainer library and keep it in scope
 *   2. Create a class inheriting NetworkListener and implement the functions
 *   3. Create a Server or Client object, giving it an instance of your NetworkListener
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
		virtual void connection(NetworkTraffic const& e) {}
		virtual void disconnect(NetworkTraffic const& e) {}
		virtual void receive(NetworkTraffic const& e) {}
	};


	/// Common networking stuff for both server and client
	class NetworkObject: public boost::noncopyable {
	  public:
		NetworkObject(NetworkListener& listener, void* data = NULL): m_quit(false), m_id(), m_host(NULL), m_peer(NULL), m_listener(listener), m_data(data) { }

		~NetworkObject() {
			terminate();
			if (m_host) enet_host_destroy(m_host);
			m_host = NULL;
		}

		virtual void listen() {
			ENetEvent e;
			while (!m_quit) {
				{	// Lock mutex
					boost::mutex::scoped_lock lock(m_mutex);
					enet_host_service(m_host, &e, 5);
				}
				switch (e.type) {
					case ENET_EVENT_TYPE_NONE: break;
					case ENET_EVENT_TYPE_CONNECT: {
						std::cout << "Connected " << IPv4(e.peer->address.host) << ":" << e.peer->address.port << std::endl;
						m_listener.connection(NetworkTraffic(e.peer->incomingPeerID, e.peer->data));
						break;
					} case ENET_EVENT_TYPE_DISCONNECT: {
						std::cout << "Disconnected " << IPv4(e.peer->address.host) << ":" << e.peer->address.port << std::endl;
						m_listener.disconnect(NetworkTraffic(e.peer->incomingPeerID, e.peer->data));
						e.peer->data = NULL;
						break;
					} case ENET_EVENT_TYPE_RECEIVE: {
						m_listener.receive(NetworkTraffic(e.peer->incomingPeerID, e.peer->data, e.packet->data, e.packet->dataLength));
						enet_packet_destroy (e.packet); // Clean-up
						break;
					}
				}
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		}

		/// Send a string
		//  TODO: Use separate channel for unordered/ordered packets
		void send(const std::string& msg, int flag = 0) {
			ENetPacket* packet = enet_packet_create(msg.c_str(), msg.length(), flag);
			boost::mutex::scoped_lock lock(m_mutex);
			if (m_peer) enet_peer_send(m_peer, 0, packet); // Send to peer through channel 0
			else enet_host_broadcast(m_host, 0, packet); // Send through channel 0 to all peers
		}

		/// Send a char
		void send(const char& msg, int flag = 0) {
			send(std::string(1, msg), flag);
		}

		void terminate() { m_quit = true; m_thread.join(); }

		unsigned getId() const { return m_id; }

		friend class Server;
		friend class Client;

	  protected:
		bool m_quit;
		unsigned m_id;
		ENetAddress m_address;
		ENetHost* m_host;
		ENetPeer* m_peer;
		mutable boost::mutex m_mutex;
		boost::thread m_thread;
		NetworkListener& m_listener;
		void* m_data;
	};


	class Server: public NetworkObject {
	  public:
		Server(NetworkListener& listener, int port = -1, void* data = NULL, unsigned max_connections = 16): NetworkObject(listener, data) {
			m_address.host = ENET_HOST_ANY;
			m_address.port = port < 0 ? ENET_PORT_ANY : port;
			// Create host at address, max_conns, unlimited up/down bandwith
			m_host = enet_host_create(&m_address, max_connections, 0, 0);
			if (m_host == NULL)
				throw std::runtime_error("An error occurred while trying to create an ENet host.");
			// Start listener thread
			m_thread = boost::thread(boost::bind(&NetworkObject::listen, boost::ref(*this)));
		}
	};


	class Client: public NetworkObject {
	  public:
		/// Construct new
		Client(NetworkListener& listener, void* data = NULL): NetworkObject(listener, data) { }

		/// Connect to the server
		void connect(std::string host, int port) {
			// Create an endpoint
			m_host = enet_host_create(NULL, 2, 0, 0);
			if (m_host == NULL)
				throw std::runtime_error("An error occurred while trying to create an ENet host.");
			// Set properties
			enet_address_set_host(&m_address, host.c_str());
			m_address.port = port;
			// Initiate the connection, allocating the two channels 0 and 1.
			m_peer = enet_host_connect(m_host, &m_address, 2);
			if (m_peer == NULL)
				throw std::runtime_error("No available peers for initiating an ENet connection.");
			// Wait up to 5 seconds for the connection attempt to succeed.
			ENetEvent event;
			if (enet_host_service (m_host, &event, 5000) > 0 &&
			  event.type == ENET_EVENT_TYPE_CONNECT) {
				m_id = event.peer->outgoingPeerID; // Setup id
				// Start listener thread
				m_thread = boost::thread(boost::bind(&NetworkObject::listen, boost::ref(*this)));
			} else { // Failure
				enet_peer_reset(m_peer);
				throw std::runtime_error(std::string("Connection to ") + host + " failed!");
			}
		}
	};

}
