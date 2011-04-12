#include "network.hpp"
#include "protocol.hpp"


class P2PGameServer: public net::Server {
public:
	P2PGameServer(): net::Server(*this)
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

};


class P2PGameClient: public net::Client {
public:
	P2PGameClient(): net::Client(*this)
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

};
