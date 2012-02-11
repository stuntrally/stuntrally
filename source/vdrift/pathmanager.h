#ifndef _PATHMANAGER_H
#define _PATHMANAGER_H

#ifdef unix
#include <dirent.h>
#endif
#include <string>
#include <list>


class PATHMANAGER
{
private:
	static std::string ogre_plugin_dir;
	static std::string home_dir;
	static std::string user_config_dir;
	static std::string game_config_dir;
	static std::string user_data_dir;
	static std::string game_data_dir;
	static std::string cache_dir;
	static std::string profile_suffix;

public:
	static void Init(std::ostream & info_output, std::ostream & error_output, bool log_paths=true);
	static bool GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension=""); ///<optionally filter for the given extension

	static std::string GetOgrePluginDir() {          return ogre_plugin_dir; }
	static std::string GetHomeDir() {                return home_dir; }
	static std::string GetDataPath() {               return game_data_dir; }
	static std::string GetUserDataDir() {            return user_data_dir; }
	static std::string GetUserConfigDir() {          return user_config_dir;  }
	static std::string GetGameConfigDir() {          return game_config_dir;  }

	static std::string GetCacheDir() {               return cache_dir; }
	static std::string GetShaderCacheDir() {         return cache_dir + "/shaders"; }
	static std::string GetSettingsFile() {           return user_config_dir + "/game.cfg"; }
	static std::string GetEditorSetFile() {          return user_config_dir + "/editor.cfg"; }

	static std::string GetTrackRecordsPath() {       return user_data_dir + "/records";  }
	static std::string GetScreenShotDir() {          return user_data_dir + "/screenshots";  }

	static std::string GetTreesPath() {              return game_data_dir + "/trees"; }
	static std::string GetTrackPath() {              return game_data_dir + "/tracks"; }
	static std::string GetTrackPathUser() {          return user_data_dir + "/tracks"; }
	static std::string GetCarPath() {                return game_data_dir + "/cars"; }

	static std::string GetGenericSoundPath() {       return game_data_dir + "/sounds"; }
	static std::string GetDriverPath() {             return game_data_dir + "/drivers"; }
	static std::string GetReplayPath() {             return user_data_dir + "/replays"; }
	static std::string GetGhostsPath() {             return user_data_dir + "/ghosts"; }

	static bool FileExists(const std::string & filename);

	static bool CreateDir(const std::string& path, std::ostream & error_output);

	// FIXME: Do we intend to use profiles?
	/// Only call this before Init()
	static void SetProfile(const std::string& value);
};

#endif
