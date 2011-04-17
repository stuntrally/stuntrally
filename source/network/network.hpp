#include "enet-wrapper.hpp"
#include "protocol.hpp"

class P2PGameClient: public net::NetworkListener {
public:
	P2PGameClient(int port = protocol::DEFAULT_PORT);

	void connect(std::string address, int port = protocol::DEFAULT_PORT);

	void broadcast(const std::string& msg);

	void connectionEvent(net::NetworkTraffic const& e);

	void disconnectEvent(net::NetworkTraffic const& e);

	void receiveEvent(net::NetworkTraffic const& e);

private:
	net::NetworkObject m_client;
};
