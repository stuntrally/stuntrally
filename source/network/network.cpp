#include "network.hpp"


P2PGameClient::P2PGameClient(int port): m_client(*this, port)
{
	//TODO
}

void P2PGameClient::connect(std::string address, int port)
{
	m_client.connect(address, port);
}

void P2PGameClient::broadcast(const std::string& msg)
{
	//protocol::Packet packet(protocol::TEXT_MESSAGE, msg.length(), msg.c_str());
	m_client.broadcast(msg);
}

void P2PGameClient::connectionEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Connection id=" << e.peer_id << std::endl;
}

void P2PGameClient::disconnectEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Disconnected id=" << e.peer_id << std::endl;
}

void P2PGameClient::receiveEvent(net::NetworkTraffic const& e)
{
	//TODO
	std::cout << "Traffic id=" << e.peer_id << ", content: " << std::string((char*)e.packet_data, e.packet_length) << std::endl;
}
