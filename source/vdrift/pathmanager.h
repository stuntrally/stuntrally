#ifndef _PATHMANAGER_H
#define _PATHMANAGER_H

#ifdef unix
#include <dirent.h>
#endif

class PATHMANAGER
{
private:
	static std::string home_dir;
	static std::string user_config_dir;
	static std::string game_config_dir;
	static std::string user_data_dir;
	static std::string game_data_dir;
	static std::string profile_suffix;

public:
	static void Init(std::ostream & info_output, std::ostream & error_output);
	static bool GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension=""); ///<optionally filter for the given extension

	static std::string GetDataPath() {               return game_data_dir; }
	static std::string GetUserConfigDir() {          return user_config_dir;  }
	static std::string GetGameConfigDir() {          return game_config_dir;  }
	//static std::string GetTrackRecordsPath() const { return user_config_dir  + "/records" + profile_suffix;  }
	static std::string GetSettingsFile() {           return user_config_dir + "/game.cfg"; }
	static std::string GetLogFile() {                return user_config_dir + "/log.txt"; }
	static std::string GetTrackPath() {              return game_data_dir + "/tracks"; }
	static std::string GetCarPath() {                return game_data_dir + "/cars"; }
	static std::string GetCarControlsFile() {        return user_config_dir + "/controls.cfg"; }
	static std::string GetDefaultCarControlsFile() { return game_config_dir + "/controls.cfg"; }
	static std::string GetGenericSoundPath() {       return game_data_dir + "/sounds"; }
	static std::string GetDriverPath() {             return game_data_dir + "/drivers"; }
	static std::string GetReplayPath() {             return game_data_dir + "/replays"; }

	static bool FileExists(const std::string & filename)
	{
		std::ifstream test(filename.c_str());
		if (test) return true;
		else return false;
	}


	// FIXME: Do we intend to use profiles?
	/// Only call this before Init()
	static void SetProfile(const std::string& value)
	{
		assert(game_data_dir.empty()); // Assert that Init() hasn't been called yet
		profile_suffix = "." + value;
	}

};

#endif
