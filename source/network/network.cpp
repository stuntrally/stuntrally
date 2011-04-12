#include "enet-wrapper.hpp"
#include "protocol.hpp"

using namespace net;

class P2PGameServer: public NetworkListener {
public:
	P2PGameServer(): m_server(*this, protocol::DEFAULT_PORT)
	{
		//TODO
	}

	void connection(NetworkTraffic const& e)
	{
		//TODO
	}

	void disconnect(NetworkTraffic const& e)
	{
		//TODO
	}

	void receive(NetworkTraffic const& e)
	{
		//TODO
	}

private:
	Server m_server;
};


class P2PGameClient: public NetworkListener {
public:
	P2PGameClient(): m_client(*this)
	{
		//TODO
	}

	void connection(NetworkTraffic const& e)
	{
		//TODO
	}

	void disconnect(NetworkTraffic const& e)
	{
		//TODO
	}

	void receive(NetworkTraffic const& e)
	{
		//TODO
	}

private:
	Client m_client;
};
