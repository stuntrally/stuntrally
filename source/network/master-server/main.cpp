#include <iostream>
#include <string>
#include <map>
#include "../enet-wrapper.hpp"
#include "../protocol.hpp"

#define VERSIONSTRING "0.1"

typedef std::map<uint32_t, protocol::GameInfo> GameList;

class GameListManager {
public:

	/// Update or announce a game
	/// @param game the game's information
	void updateGame(const protocol::GameInfo& game) {
		m_games[game.id] = game;
	}

	/// Gets the games
	/// @return a packet containing the games in a serialized form
	net::NetworkTraffic getGames() const {
		for (GameList::const_iterator it = m_games.begin(); it != m_games.end(); ++it) {
			// TODO: Implement
		}
		return net::NetworkTraffic();
	}

	/// Removes outdated games from the list
	void purgeGames() {
		for (GameList::iterator it = m_games.begin(); it != m_games.end(); ++it) {
			// TODO: Implement
		}
	}

private:
	GameList m_games;
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
			case protocol::REQUEST_GAME_LIST: {
				m_client.send(e.peer_id, m_glm.getGames(), net::PACKET_RELIABLE);
				break;
			}
			case protocol::UPDATE_GAME_STATUS: {
				// FIXME
				const protocol::GameInfo* game = reinterpret_cast<const protocol::GameInfo*>(e.packet_data);
				m_glm.updateGame(*game);
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
			std::cout << "Usage: " << argv[0] << " "
			  << "[--version | -v] [--help | -h] [--port <portnumber>]"
			  << std::endl;
			return 0;
		} else if (arg == "--port" && i < argc-1) {
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
