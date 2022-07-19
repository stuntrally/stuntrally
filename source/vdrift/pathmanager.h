#pragma once
#ifdef unix
#include <dirent.h>
#endif
#include <string>
#include <sstream>
#include <list>

typedef std::list <std::string> strlist;

class PATHMANAGER
{
private:
	static std::string ogre_plugin, home_dir;
	static std::string user_config, game_config;
	static std::string user_data, game_data, cache_dir;

public:
	static void Init(bool log_paths=true);
	static std::stringstream info;

	static std::string OgrePluginDir() {   return ogre_plugin; }
	static std::string HomeDir() {         return home_dir; }

	static std::string UserConfigDir() {   return user_config;  }
	static std::string GameConfigDir() {   return game_config;  }

	static std::string CacheDir() {        return cache_dir; }
	static std::string ShaderDir() {       return cache_dir + "/_shaders"; }

	static std::string SettingsFile() {    return user_config + "/game.cfg"; }
	static std::string EditorSetFile() {   return user_config + "/editor.cfg"; }

	static std::string Data() {            return game_data; }
	static std::string DataUser() {        return user_data + "/data"; }
	static std::string Tracks() {          return game_data + "/tracks"; }
	static std::string TracksUser() {      return user_data + "/tracks"; }

	static std::string CarSim() {          return game_data + "/carsim"; }
	static std::string CarSimU() {         return user_data + "/data/carsim"; }
	static std::string Cars() {            return game_data + "/cars"; }

	static std::string Replays() {         return user_data + "/replays"; }
	static std::string Ghosts() {          return user_data + "/ghosts"; }
	static std::string TrkGhosts() {       return game_data + "/ghosts"; }
	static std::string Lessons() {         return game_data + "/lessons"; }

	static std::string Records() {         return user_data + "/records";  }
	static std::string Sounds() {          return game_data + "/sounds"; }
	static std::string Screenshots() {     return user_data + "/screenshots";  }

	//  list files
	static bool DirList(std::string dirpath, strlist& dirlist, std::string extension="");

	static bool FileExists(const std::string & filename);

	static bool CreateDir(const std::string& path);

	static void OpenUrl(const std::string& url);

};
