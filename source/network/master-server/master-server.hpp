#pragma once
/**
 * The master server interface.
 * In other words, what information clients can receive and how.
 */

#include <string>
#include <stdint.h>

struct GameInfo {
	uint32_t id;
	std::string name;
	std::string address;
	uint8_t players;
	std::string track;
};

typedef std::vector<GameInfo> GameList;

class MasterInterface {
public:

	/**
	 * Announces new game to the master server.
	 * @param game the game to announce
	 */
	void announceGame(const GameInfo& game);

	/**
	 * Gets the list of games.
	 * @return the list of games
	 */
	GameList getGameList() const;
};
