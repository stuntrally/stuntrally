#include "enet-wrapper.hpp"
#include "protocol.hpp"


class P2PGameClient: public net::NetworkListener {
public:
	P2PGameClient(int port = protocol::DEFAULT_PORT);

	~P2PGameClient();

	/// Connects to a peer
	void connect(const std::string& address, int port = protocol::DEFAULT_PORT);

	/// Broadcasts all known peer infos to every one connected
	void sendPeerInfo();

	/// Broadcasts a chat message to all peers
	void sendMessage(const std::string& msg);

	/// Starts the network forming phase
	void startLobby();

	/// Shuts down the network forming phase
	void startGame();

	/// Thread that periodically proadcasts peer info, don't call directly
	void peerInfoSenderThread();

	/// Callback from networking
	void connectionEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void disconnectEvent(net::NetworkTraffic const& e);

	/// Callback from networking
	void receiveEvent(net::NetworkTraffic const& e);

private:
	net::NetworkObject m_client;
	protocol::PeerMap m_peers;
	enum State { DISCONNECTED, LOBBY, GAME } m_state;
	boost::thread m_peerInfoSenderThread;
	mutable boost::mutex m_mutex;
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
