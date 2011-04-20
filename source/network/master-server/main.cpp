#include <iostream>
#include <string>
#include <map>
#include "../enet-wrapper.hpp"
#include "../protocol.hpp"

#define VERSIONSTRING "0.1"

//TODO: Demonization

class GameListManager {
public:

	/// Constructor.
	GameListManager(): m_next_id(1) {}

	/// Update or announce a game.
	/// @param game the game's information
	void updateGame(protocol::GameInfo& game) {
		if (game.id == 0) {
			game.id = m_next_id;
			++m_next_id;
		}
		m_games[game.id] = game;
	}

	/// Gets the games.
	/// @return a packet containing the games in a serialized form
	const protocol::GameList& getGames() const {
		return m_games;
	}

	/// Removes outdated games from the list.
	void purgeGames() {
		for (protocol::GameList::iterator it = m_games.begin(); it != m_games.end(); ++it) {
			// TODO: Implement
		}
	}

private:
	protocol::GameList m_games;
	unsigned m_next_id;
};


class Server: public net::NetworkListener {
public:
	Server(GameListManager& glm, int port = protocol::DEFAULT_PORT): m_client(*this, port), m_glm(glm)
	{
		std::cout << "Listening on port " << port << "..." << std::endl;
	}

	void connectionEvent(net::NetworkTraffic const& e)
	{
		std::cout << "Connection id=" << e.peer_id << std::endl;
	}

	void disconnectEvent(net::NetworkTraffic const& e)
	{
		std::cout << "Disconnected id=" << e.peer_id << std::endl;
	}

	void receiveEvent(net::NetworkTraffic const& e)
	{
		if (e.packet_length <= 0 || !e.packet_data) return;
		switch (e.packet_data[0]) {
			case protocol::GAME_LIST: {
				std::cout << "Game list request received." << std::endl;
				protocol::GameList games = m_glm.getGames();
				for (protocol::GameList::const_iterator it = games.begin(); it != games.end(); ++it) {
					m_client.send(e.peer_id, it->second, net::PACKET_RELIABLE);
				}
				break;
			}
			case protocol::GAME_STATUS: {
				// Get peer struct
				ENetPeer* peer = m_client.getPeerPtr(e.peer_id);
				if (!peer) return;
				std::cout << "Game update received." << std::endl;
				// Unserialize
				protocol::GameInfo game = *reinterpret_cast<const protocol::GameInfo*>(e.packet_data);
				// Fill in peer info
				game.address = peer->address.host;
				game.port = peer->address.port;
				// Update game status
				m_glm.updateGame(game);
				// Send confirmation (and id in case of new game)
				game.packet_type = protocol::GAME_ACCEPTED;
				m_client.send(e.peer_id, game, net::PACKET_RELIABLE);
				break;
			}
			default: {
				std::cout << "Unknown packet type received." << std::endl;
			}
		}
	}

private:
	net::NetworkObject m_client;
	GameListManager m_glm;
};


int main(int argc, char** argv) {
	std::cout << "Stunt Rally Master Server - version " << VERSIONSTRING << std::endl;
	int port = protocol::DEFAULT_PORT;

	// Command line handling
	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "--version" || arg == "-v") {
			return 0; // Already printed version
		} else if (arg == "--help" || arg == "-h") {
			std::cout << std::endl << "Usage: " << argv[0] << " [PARAMETERS]" << std::endl
				<< std::endl
				<< "Available parameters:" << std::endl
				<< "  -v, --version               print version number and exit" << std::endl
				<< "  -h, --help                  this help" << std::endl
				<< "  -p, --port <portnumber>     listen given port for connections" << std::endl
				<< "                              default: " << protocol::DEFAULT_PORT << std::endl
				;
			return 0;
		} else if ((arg == "--port" || arg == "-p") && i < argc-1) {
			port = atoi(argv[i+1]);
			++i;
		} else {
			std::cout << "Invalid argument " << arg << std::endl;
			return -1;
		}
	}

	GameListManager games;
	Server server(games, port);

	while (true) {
		// Periodically remove zombie games
		boost::this_thread::sleep(boost::posix_time::milliseconds(7000));
		games.purgeGames();
	}

	return 0;
}
