#include "enet-wrapper.hpp"
#include "protocol.hpp"


class P2PGameClient: public net::NetworkListener {
public:
	P2PGameClient(int port = protocol::DEFAULT_PORT);

	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	void sendPeerInfo();

	void sendMessage(const std::string& msg);

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	net::NetworkObject m_client;
	protocol::PeerMap m_peers;
};


class MasterClient: public net::NetworkListener {
public:
	MasterClient();

	/// Connects to the master server
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	/// Updates the hosted game state to master server
	void updateGame(const std::string& name, const std::string& track, int players);

	/// Requests new game listing
	void refreshList();

	/// Returns cached list of games
	protocol::GameList getList() const { return m_games; };

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	net::NetworkObject m_client;
	protocol::GameList m_games;
	int m_gameId;
};
