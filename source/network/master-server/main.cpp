// Copyright Tapio Vierros 2011-2014
// Licensed under GPLv3 or later.
// See License.txt for more info on licensing.

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include "../enet-wrapper.hpp"
#include "../protocol.hpp"
#ifdef __linux
#include <unistd.h>  // for daemon()
#endif

// Fix error define in windows header
#undef ERROR

#define VERSIONSTRING "0.7.1"

//  How many seconds without update until a game becomes zombie
#define DEFAULT_ZOMBIE_TIMEOUT 5
unsigned g_zombieTimeout = DEFAULT_ZOMBIE_TIMEOUT;


// TODO: Linux syslog support for daemon
enum LogLevel
{
	ERROR   = 0,
	NORMAL  = 1,
	VERBOSE = 2
}
g_loglevel = NORMAL;

struct Stats
{
	Stats()
	{
		memset(this, 0, sizeof(Stats));
	}
	time_t launchTime;
	int port;
	unsigned listRequests;
	unsigned gamesCreated;
	unsigned gamesStarted;
	unsigned gamesWaiting;
}
g_stats;


///  This prints the current time, but only if enough seconds has passed since last time
void handleTimePrinting(int silencetime = 60)
{
	static uint32_t logSilenceTimer = 0;
	if ((uint32_t)std::time(NULL) > logSilenceTimer + silencetime)
	{
		time_t t = std::time(NULL);
		std::cout << "Time: " << ctime(&t);  // endl comes from ctime()
	}
	logSilenceTimer = (uint32_t)std::time(NULL);
}


///  Use this function as std::cout, giving it the message's log level as parameter
std::ostream& out(LogLevel level)
{
	if (level == VERBOSE)
		handleTimePrinting();
	if (level == ERROR)
		return std::cerr;
	if (g_loglevel >= level)
		return std::cout;
	
	static std::ostringstream oss;  // Sink for discarded messages
	oss.clear();
	return oss;
}


///  Class for managing the available games
//--------------------------------------------------------------------------------
class GameListManager
{
public:

	///  Constructor
	GameListManager()
		: m_mutex(), m_next_id(1)
	{
	}

	///  Update or announce a game.
	///  @param game the game's information
	void updateGame(protocol::GameInfo& game)
	{
		if (game.id == 0)
		{
			//  Assign ID
			game.id = m_next_id;
			++m_next_id;
			g_stats.gamesCreated++;
		}
		//  Update timestamp
		game.timestamp = (uint32_t)std::time(NULL);
		//  Save
		boost::mutex::scoped_lock lock(m_mutex);
		m_games[game.id] = game;
		g_stats.gamesWaiting = m_games.size();
	}

	///  Gets the games.
	///  @return a packet containing the games in a serialized form
	const protocol::GameList getGames() const
	{
		return m_games;
	}

	///  Removes outdated games from the list.
	void purgeGames()
	{
		int removecount = 0;
		{
			boost::mutex::scoped_lock lock(m_mutex);
			protocol::GameList::iterator it = m_games.begin();
			//  Loop through the games
			while (it != m_games.end())
			{
				//  Check condition
				if ((uint32_t)std::time(NULL) > it->second.timestamp + g_zombieTimeout)
				{
					out(VERBOSE) << "Zombie game: \"" << it->second.name << "\"" << std::endl;
					m_games.erase(it++);
					++removecount;
				}else
					++it;
			}
			g_stats.gamesWaiting = m_games.size();
		}
		if (removecount > 0)
			out(VERBOSE) << "Removed " << removecount << " game(s) due to time out." << std::endl;
	}

private:
	mutable boost::mutex m_mutex;
	protocol::GameList m_games;
	unsigned m_next_id;
};


///  Network listener for handling the traffic
//--------------------------------------------------------------------------------
class Server: public net::NetworkListener
{
public:
	Server(GameListManager& glm, int port = protocol::DEFAULT_PORT)
		: m_client(*this, port, NULL, 100), m_glm(glm)
	{
		out(NORMAL) << "Listening on port " << port << "..." << std::endl;
	}

	void connectionEvent(net::NetworkTraffic const& e)
	{
		out(VERBOSE) << "Connection id=" << e.peer_id << " " << e.peer_address << std::endl;
		if (e.event_data != protocol::MASTER_PROTOCOL_VERSION)
		{
			out(VERBOSE) << "Incompatible protocol versions "
				<< "(my: " << protocol::MASTER_PROTOCOL_VERSION
				<< " hers: " << e.event_data << ")!" << std::endl;
			m_client.disconnect(e.peer_id, false, protocol::INCOMPATIBLE_MASTER_PROTOCOL);
			return;
		}
		protocol::HandshakePackage handshake = protocol::HandshakePackage();
		m_client.send(e.peer_id, net::convert(handshake), net::PACKET_RELIABLE);
	}

	void disconnectEvent(net::NetworkTraffic const& e)
	{
		out(VERBOSE) << "Disconnected id=" << e.peer_id << " " << e.peer_address << std::endl;
	}

	void receiveEvent(net::NetworkTraffic const& e)
	{
		if (e.packet_length <= 0 || !e.packet_data)
			return;
		
		switch (e.packet_data[0])
		{
			case protocol::GAME_LIST:
			{
				out(VERBOSE) << "Game list request received from " << e.peer_address << std::endl;
				g_stats.listRequests++;
				protocol::GameList games = m_glm.getGames();
				
				// Send an info packet for each game
				for (protocol::GameList::iterator it = games.begin(); it != games.end(); ++it)
				{
					m_client.send(e.peer_id, net::convert(it->second), net::PACKET_RELIABLE);
				}
			}	break;

			case protocol::GAME_STATUS:
			{
				// Get peer struct
				ENetPeer* peer = m_client.getPeerPtr(e.peer_id);
				if (!peer)
					return;
				
				// Unserialize
				protocol::GameInfo game = *reinterpret_cast<const protocol::GameInfo*>(e.packet_data);
				if (game.id == 0)
					out(VERBOSE) << "A game received from " << e.peer_address << std::endl;
				
				// Fill in peer address
				game.address = peer->address.host;
				// Update game status
				m_glm.updateGame(game);

				// Send confirmation (and id in case of new game)
				game.packet_type = protocol::GAME_ACCEPTED;
				m_client.send(e.peer_id, net::convert(game), net::PACKET_RELIABLE);

			}	break;

			case protocol::START_GAME:
			{	out(VERBOSE) << "A game started" << std::endl;
				g_stats.gamesStarted++;
			}	break;
			
			default:
				out(VERBOSE) << "Unknown packet type " << int(e.packet_data[0]) << " received from " << e.peer_address << std::endl;
		}
	}

private:
	net::NetworkObject m_client;
	GameListManager& m_glm;
};


//--------------------------------------------------------------------------------
class StatusPage
{
public:
	StatusPage(std::string file)
		: m_format(FORMAT_PLAIN), m_fileName(file)
	{
		if (m_fileName.find(".html") != std::string::npos)
			m_format = FORMAT_HTML;
	}

	void write(const Stats& stats)
	{
		if (m_fileName.empty())
			return;
		
		std::string text = m_templates[m_format];
		boost::replace_all(text, "%DATE%", boost::lexical_cast<std::string>(ctime(&stats.launchTime)));
		boost::replace_all(text, "%PORT%", boost::lexical_cast<std::string>(stats.port));
		boost::replace_all(text, "%GAMES%", boost::lexical_cast<std::string>(stats.gamesWaiting));
		boost::replace_all(text, "%CREATED%", boost::lexical_cast<std::string>(stats.gamesCreated));
		boost::replace_all(text, "%STARTED%", boost::lexical_cast<std::string>(stats.gamesStarted));
		boost::replace_all(text, "%LISTS%", boost::lexical_cast<std::string>(stats.listRequests));
		
		std::ofstream f(m_fileName.c_str());
		f << text << std::flush;
	}

private:
	enum
	{	FORMAT_PLAIN, FORMAT_HTML, NUM_FORMATS  }
	m_format;
	std::string m_fileName;
	static const std::string m_templates[];
};


//--------------------------------------------------------------------------------
const std::string StatusPage::m_templates[] =
{
	"Stunt Rally Master Server Status Page\n"
	"====================================\n"
	"\n"
	"Currently, there are %GAMES% available games.\n"
	"\n"
	"Started on %DATE%"
	"Since then, there has been %LISTS% game list requests,\n"
	"%CREATED% lobbies created and %STARTED% games started.\n"
	"\n"
	"Running on port %PORT%.\n"
	,

	"<!DOCTYPE html>\n"
	"<html>\n"
	"<head>\n"
	"	<meta http-equiv=\"refresh\" content=\"10\">\n"
	"	<link href=\"style.css\" rel=\"stylesheet\" type=\"text/css\">\n"
	"	<title>[%GAMES%] Stunt Rally Master Server</title>\n"
	"</head>\n"
	"<body>\n"
	"	<h1>Stunt Rally Master Server Status Page</h1>\n"
	"	<p>Currently, there are %GAMES% available games.</p>\n"
	"	<p>Since %DATE% there have been:<br>\n"
	"	%LISTS% game list requests,<br>\n"
	"	%CREATED% lobbies created and<br>\n"
	"	%STARTED% games started.</p>\n"
	"	<small>Running on port %PORT%.</small>\n"
	"</body>\n"
	"</html>\n"
};


///  Program entry point
//--------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	std::cout << "Stunt Rally Master Server - version " << VERSIONSTRING << std::endl;
	int port = protocol::DEFAULT_PORT;
	bool daemonize = false;
	std::string statusFile;

	//  Command line handling
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);
		if (arg == "--version" || arg == "-v")
		{
			return 0;  // Already printed version
		}
		else if (arg == "--help" || arg == "-h")
		{
			std::cout << std::endl << "Usage: " << argv[0] << " [PARAMETERS]" << std::endl
				<< std::endl
				<< "Available parameters:" << std::endl
				<< "  -v, --version               print version number and exit" << std::endl
				<< "  -h, --help                  this help" << std::endl
				<< "  -V, --verbose               output more information, useful for testing" << std::endl
				<< "  -q, --quiet                 output only errors" << std::endl
#ifdef __linux
				<< "  -d, --daemon                run in backround (i.e. daemonize)" << std::endl
#endif
				<< "  -t, --timeout <seconds>     seconds without update and the game becomes zombie" << std::endl
				<< "                              default: " << DEFAULT_ZOMBIE_TIMEOUT << std::endl
				<< "  -p, --port <portnumber>     listen given port for connections" << std::endl
				<< "                              default: " << protocol::DEFAULT_PORT << std::endl
				<< "  -s, --status <file>         periodically dump status to given file" << std::endl
				<< "                              uses HTML if file name has .html extension" << std::endl;
			return 0;
		}
		else if (arg == "--verbose" || arg == "-V")
		{
			g_loglevel = VERBOSE;
		}
		else if (arg == "--quiet" || arg == "-q")
		{
			g_loglevel = ERROR;
#ifdef __linux
		}
		else if (arg == "--daemon" || arg == "-d")
		{
			daemonize = true;
#endif
		}
		else if ((arg == "--port" || arg == "-p") && i < argc-1)
		{
			port = atoi(argv[i+1]);
			++i;
		}
		else if ((arg == "--timeout" || arg == "-t") && i < argc-1)
		{
			g_zombieTimeout = atoi(argv[i+1]);
			++i;
		}
		else if ((arg == "--status" || arg == "-s") && i < argc-1)
		{
			statusFile = argv[i+1];
			++i;
		}
		else
		{	out(ERROR) << "Invalid argument " << arg << std::endl;
			return -1;
		}
	}

#ifdef __linux
	//  Daemonization
	if (daemonize)
	{
		if (daemon(1, 0))
		{	//  keep working dir, close streams
			out(ERROR) << "Daemonization failed" << std::endl;
			return EXIT_FAILURE;
		}
	}
#endif

	g_stats.launchTime = std::time(NULL);
	g_stats.port = port;

	out(VERBOSE) << "Verbose mode enabled" << std::endl;

	StatusPage status(statusFile);
	out(VERBOSE) << (statusFile.empty() ? "Not creating status page"
										: ("Using status page " + statusFile)) << std::endl;
	try
	{
		GameListManager games;
		Server server(games, port);  // Launches a thread for listening the traffic

		while (true)
		{
			status.write(g_stats);

			//  Periodically remove zombie games
			boost::this_thread::sleep(boost::posix_time::milliseconds(g_zombieTimeout * 1000));
			games.purgeGames();
		}
	}
	catch (const std::exception& e)
	{
		out(ERROR) << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
