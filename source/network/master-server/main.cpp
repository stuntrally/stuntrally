#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include "../enet-wrapper.hpp"
#include "../protocol.hpp"
#ifdef __linux
#include <unistd.h> // for daemon()
#endif

#define VERSIONSTRING "0.2"

#define ZOMBIE_TIMEOUT 5  // How many seconds without update until the game becomes zombie


enum LogLevel {
	ERROR   = 0,
	NORMAL  = 1,
	VERBOSE = 2
} g_loglevel = NORMAL;

/// Use this function as std::cout, giving it the message's log level as parameter
std::ostream& out(LogLevel level) {
	if (g_loglevel == ERROR) return std::cerr;
	if (g_loglevel >= level) return std::cout;
	static std::ostringstream oss;
	oss.clear();
	return oss;
}


/// Class for managing the available games
class GameListManager {
public:

	/// Constructor.
	GameListManager(): m_mutex(), m_next_id(1) {}

	/// Update or announce a game.
	/// @param game the game's information
	void updateGame(protocol::GameInfo& game) {
		if (game.id == 0) {
			// Assign ID
			game.id = m_next_id;
			++m_next_id;
		}
		// Update timestamp
		game.timestamp = (uint32_t)std::time(NULL);
		// Save
		boost::mutex::scoped_lock lock(m_mutex);
		m_games[game.id] = game;
	}

	/// Gets the games.
	/// @return a packet containing the games in a serialized form
	const protocol::GameList getGames() const {
		return m_games;
	}

	/// Removes outdated games from the list.
	void purgeGames() {
		int removecount = 0;
		{
			boost::mutex::scoped_lock lock(m_mutex);
			protocol::GameList::iterator it = m_games.begin();
			// Loop through the games
			while (it != m_games.end()) {
				// Check condition
				if ((uint32_t)std::time(NULL) > it->second.timestamp + ZOMBIE_TIMEOUT) {
					out(VERBOSE) << "Zombie game: \"" << it->second.name << "\"" << std::endl;
					m_games.erase(it++);
					++removecount;
				} else ++it;
			}
		}
		if (removecount > 0)
			out(VERBOSE) << "Removed " << removecount << " game(s) due to time out." << std::endl;
	}

private:
	mutable boost::mutex m_mutex;
	protocol::GameList m_games;
	unsigned m_next_id;
};


class Server: public net::NetworkListener {
public:
	Server(GameListManager& glm, int port = protocol::DEFAULT_PORT)
		: m_client(*this, port), m_glm(glm)
	{
		out(NORMAL) << "Listening on port " << port << "..." << std::endl;
	}

	void connectionEvent(net::NetworkTraffic const& e)
	{
		out(VERBOSE) << "Connection id=" << e.peer_id << std::endl;
	}

	void disconnectEvent(net::NetworkTraffic const& e)
	{
		out(VERBOSE) << "Disconnected id=" << e.peer_id << std::endl;
	}

	void receiveEvent(net::NetworkTraffic const& e)
	{
		if (e.packet_length <= 0 || !e.packet_data) return;
		switch (e.packet_data[0]) {
			case protocol::GAME_LIST: {
				out(VERBOSE) << "Game list request received" << std::endl;
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
				// Unserialize
				protocol::GameInfo game = *reinterpret_cast<const protocol::GameInfo*>(e.packet_data);
				out(VERBOSE) << "Game update received for \"" << game.name << "\"" << std::endl;
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
				out(VERBOSE) << "Unknown packet type " << int(e.packet_data[0]) << " received" << std::endl;
			}
		}
	}

private:
	net::NetworkObject m_client;
	GameListManager& m_glm;
};


int main(int argc, char** argv) {
	std::cout << "Stunt Rally Master Server - version " << VERSIONSTRING << std::endl;
	int port = protocol::DEFAULT_PORT;
	bool daemonize = false;

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
				<< "  -V, --verbose               output more information, useful for testing" << std::endl
#ifdef __linux
				<< "  -d, --daemon                run in backround (i.e. daemonize)" << std::endl
#endif
				<< "  -p, --port <portnumber>     listen given port for connections" << std::endl
				<< "                              default: " << protocol::DEFAULT_PORT << std::endl
				;
			return 0;
		} else if (arg == "--verbose" || arg == "-V") {
			g_loglevel = VERBOSE;
#ifdef __linux
		} else if (arg == "--daemon" || arg == "-d") {
			daemonize = true;
#endif
		} else if ((arg == "--port" || arg == "-p") && i < argc-1) {
			port = atoi(argv[i+1]);
			++i;
		} else {
			out(ERROR) << "Invalid argument " << arg << std::endl;
			return -1;
		}
	}

#ifdef __linux
	// Daemonization
	if (daemonize) {
		if (daemon(1, 0)) { // keep working dir, close streams
			out(ERROR) << "Daemonization failed" << std::endl;
			return EXIT_FAILURE;
		}
	}
#endif

	out(VERBOSE) << "Verbose mode" << std::endl;

	try {
		GameListManager games;
		Server server(games, port); // Launches a thread for listening the traffic

		while (true) {
			// Periodically remove zombie games
			boost::this_thread::sleep(boost::posix_time::milliseconds(ZOMBIE_TIMEOUT * 1000));
			games.purgeGames();
		}
	} catch (const std::exception& e) {
		out(ERROR) << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
