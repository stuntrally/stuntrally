#include "pch.h"
/// Big portions of this file are borrowed and adapted from Performous under GPL (http://performous.org)


#include "pathmanager.h"
#include <boost/filesystem.hpp>
#include <string>
#include <fstream>
#include <list>
#include <cassert>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

// Should come from CMake
#ifndef SHARED_DATA_DIR
#define SHARED_DATA_DIR "data"
#endif
// Optionally comes from CMake
#ifndef OGRE_PLUGIN_DIR
#define OGRE_PLUGIN_DIR ""
#endif


// TODO: Create a PORTABLE_INSTALL flag that allows disabling the usage of system dirs


// Define this useful alias for the overlong namespace name
namespace fs = boost::filesystem;

namespace {
	fs::path execname();
}


//  static vars
std::string PATHMANAGER::ogre_plugin_dir, PATHMANAGER::home_dir,
	PATHMANAGER::user_config_dir, PATHMANAGER::game_config_dir,
	PATHMANAGER::user_data_dir, PATHMANAGER::game_data_dir,
	PATHMANAGER::cache_dir, PATHMANAGER::profile_suffix;


void PATHMANAGER::Init(std::ostream & info_output, std::ostream & error_output)
{
	typedef std::vector<fs::path> Paths;

	// Set Ogre plugins dir
	{
		ogre_plugin_dir = "";
		char *plugindir = getenv("OGRE_PLUGIN_DIR");
		if (plugindir) {
			ogre_plugin_dir = plugindir;
		#ifndef _WIN32
		} else if (fs::exists(fs::path(OGRE_PLUGIN_DIR) / "RenderSystem_GL.so")) {
			ogre_plugin_dir = OGRE_PLUGIN_DIR;
		#endif
		} else {
			#ifdef _WIN32
			ogre_plugin_dir = ".";
			#else
			Paths dirs;
			#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(_M_X64)
			dirs.push_back("/usr/local/lib64");
			dirs.push_back("/usr/lib64");
			#else
			dirs.push_back("/usr/local/lib32");
			dirs.push_back("/usr/lib32");
			#endif
			dirs.push_back("/usr/local");
			dirs.push_back("/usr/lib");
			// Loop through the paths and pick the first one that contain a plugin
			for (Paths::const_iterator p = dirs.begin(); p != dirs.end(); ++p) {
				if (fs::exists(*p / "OGRE/RenderSystem_GL.so")) {
					ogre_plugin_dir = (*p / "OGRE").string();
					break;
				} else if (fs::exists(*p / "ogre/RenderSystem_GL.so")) {
					ogre_plugin_dir = (*p / "ogre").string();
					break;
				}
			}
			#endif
		}
	}

	fs::path shortDir = "stuntrally";
	// Figure out the user's home directory
	{
		home_dir = "";
		#ifndef _WIN32 // POSIX
			char *homedir = getenv("HOME");
			if (homedir == NULL)
			{
				home_dir = "/home/";
				homedir = getenv("USER");
				if (homedir == NULL) {
					homedir = getenv("USERNAME");
					if (homedir == NULL) {
						error_output << "Could not find user's home directory!" << std::endl;
						home_dir = "/tmp/";
					}
				}
			}
		#else // Windows
			char *homedir = getenv("USERPROFILE");
			if (homedir == NULL) homedir = "data"; // WIN 9x/Me
		#endif
		home_dir += homedir;
	}

	// Find user's config dir
	#ifndef _WIN32 // POSIX
	{
		char const* conf = getenv("XDG_CONFIG_HOME");
		if (conf) user_config_dir = (fs::path(conf) / "stuntrally").string();
		else user_config_dir = (fs::path(home_dir) / ".config" / "stuntrally").string();
	}
	#else // Windows
	{
		// Open AppData directory
		std::string str;
		ITEMIDLIST* pidl;
		char AppDir[MAX_PATH];
		HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE , &pidl);
		if (hRes == NOERROR)
		{
			SHGetPathFromIDList(pidl, AppDir);
			int i;
			for (i = 0; AppDir[i] != '\0'; i++) {
				if (AppDir[i] == '\\') str += '/';
				else str += AppDir[i];
			}
			user_config_dir = (fs::path(str) / "stuntrally").string();
		}
	}
	#endif
	// Create user's config dir
	CreateDir(user_config_dir, error_output);

	// Find user's data dir (for additional data)
	#ifdef _WIN32
	user_data_dir = user_config_dir;  // APPDATA/stuntrally
	#else
	{
		fs::path shareDir = SHARED_DATA_DIR;
		char const* xdg_data_home = getenv("XDG_DATA_HOME");
		user_data_dir = (xdg_data_home ? xdg_data_home / shortDir : fs::path(home_dir) / ".local" / shareDir).string();
	}
	#endif
	// Create user's data dir and its children
	CreateDir(user_data_dir, error_output);
	CreateDir(GetTrackRecordsPath(), error_output);
	CreateDir(GetScreenShotDir(), error_output);
	CreateDir(GetTrackPathUser(), error_output);  // user tracks
	CreateDir(GetTrackPathUser()+"/_previews", error_output);
	CreateDir(GetReplayPath(), error_output);
	CreateDir(GetGhostsPath(), error_output);

	// Find game data dir and defaults config dir
	char *datadir = getenv("STUNTRALLY_DATA_ROOT");
	if (datadir)
		game_data_dir = std::string(datadir);
	else
	{	fs::path shareDir = SHARED_DATA_DIR;
		Paths dirs;

		// Adding users data dir
		// TODO: Disabled for now until this is handled properly
		//dirs.push_back(user_data_dir);

		// Adding relative path from installed executable
		dirs.push_back(execname().parent_path().parent_path() / shareDir);
		// Adding relative path for running from sources
		dirs.push_back(execname().parent_path().parent_path() / "data");
		dirs.push_back(execname().parent_path().parent_path());
		dirs.push_back(execname().parent_path() / "data");
		dirs.push_back(execname().parent_path());
		#ifndef _WIN32
		// Adding XDG_DATA_DIRS
		{
			char const* xdg_data_dirs = getenv("XDG_DATA_DIRS");
			std::istringstream iss(xdg_data_dirs ? xdg_data_dirs : "/usr/local/share/:/usr/share/");
			for (std::string p; std::getline(iss, p, ':'); dirs.push_back(p / shortDir)) {}
		}
		#endif
		// TODO: Adding path from config file

		// Loop through the paths and pick the first one that contain some data
		for (Paths::const_iterator p = dirs.begin(); p != dirs.end(); ++p) {
			// Data dir
			if (fs::exists(*p / "hud")) game_data_dir = p->string();
			// Config dir
			if (fs::exists(*p / "config"))
				game_config_dir = (*p / "config").string();
			// Check if both are found
			if (!game_data_dir.empty() && !game_config_dir.empty()) break;
		}
	}

	// Find cache dir
	#ifdef _WIN32
	cache_dir = user_config_dir + "/cache";  // APPDATA/stuntrally/cache
	#else
	char const* xdg_cache_home = getenv("XDG_CACHE_HOME");
	cache_dir = (xdg_cache_home ? xdg_cache_home / shortDir : fs::path(home_dir) / ".cache" / shortDir).string();
	#endif
	// Create cache dir
	CreateDir(cache_dir, error_output);

	// Print diagnostic info
	info_output << "Ogre plugin directory: " << ogre_plugin_dir << std::endl;
	info_output << "Home directory: " << home_dir << std::endl;
	info_output << "Config defaults directory: " << GetGameConfigDir() << std::endl;
	info_output << "User config directory: " << GetUserConfigDir() << std::endl;
	info_output << "Data directory: " << GetDataPath() << std::endl;
	info_output << "User data directory: " << GetUserDataDir() << std::endl;
	info_output << "Cache directory: " << GetCacheDir() << std::endl;
	info_output << "Log directory: " << GetLogDir() << std::endl;
}

bool PATHMANAGER::FileExists(const std::string & filename)
{
	std::ifstream test(filename.c_str());
	if (test)  return true;
	else  return false;
}

bool PATHMANAGER::CreateDir(const std::string& path, std::ostream & error_output)
{
	try	{	fs::create_directories(path);	}
	catch (...)
	{
		error_output << "Could not create directory " << path << std::endl;
		return false;
	}
	return true;
}

void PATHMANAGER::SetProfile(const std::string& value)
{
	assert(game_data_dir.empty()); // Assert that Init() hasn't been called yet
	profile_suffix = "." + value;
}


// TODO: This is probably far easier and more elegant to implement with boost::filesystem
bool PATHMANAGER::GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension)
{
//------Folder listing code for POSIX
#ifndef _WIN32
	DIR *dp;
	struct dirent *ep;
	dp = opendir (folderpath.c_str());
	if (dp != NULL)
	{
		while ( ( ep = readdir( dp ) ) )
		{
			//puts (ep->d_name);
			std::string newname = ep->d_name;
			if (newname[0] != '.')
			{
				outputfolderlist.push_back(newname);
			}
		}
		(void) closedir (dp);
	}
	else
		return false;
#else 	//------End POSIX-specific folder listing code ---- Start WIN32 Specific code
	HANDLE          hList;
	TCHAR           szDir[MAX_PATH+1];
	WIN32_FIND_DATA FileData;

	// Get the proper directory path
	sprintf(szDir, "%s\\*", folderpath.c_str ());

	// Get the first file
	hList = FindFirstFile(szDir, &FileData);
	if (hList == INVALID_HANDLE_VALUE)
	{ 
		//no files found.  that's OK
	}
	else
	{
		// Traverse through the directory structure
		while (FindNextFile(hList, &FileData))
		{
			// Check the object is a directory or not
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{} else
			{
				if (FileData.cFileName[0] != '.')
				{
					outputfolderlist.push_back (FileData.cFileName);
				}
			}
		}
	}

	FindClose(hList);
#endif //------End WIN32 specific folder listing code
	
	//remove non-matcthing extensions
	if (!extension.empty())
	{
		std::list <std::list <std::string>::iterator> todel;
		for (std::list <std::string>::iterator i = outputfolderlist.begin(); i != outputfolderlist.end(); ++i)
		{
			if (i->find(extension) != i->length()-extension.length())
				todel.push_back(i);
		}
		
		for (std::list <std::list <std::string>::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			outputfolderlist.erase(*i);
	}
	
	outputfolderlist.sort();
	return true;
}

namespace {
	/// Get the current executable name with path. Returns empty path if the name
	/// cannot be found. May return absolute or relative paths.
	#if defined(_WIN32)
	#include <windows.h>
	fs::path execname() {
		char buf[1024];
		DWORD ret = GetModuleFileName(NULL, buf, sizeof(buf));
		if (ret == 0 || ret == sizeof(buf)) return fs::path();
		return buf;
	}
	#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
	fs::path execname() {
		char buf[1024];
		uint32_t size = sizeof(buf);
		int ret = _NSGetExecutablePath(buf, &size);
		if (ret != 0) return fs::path();
		return buf;
	}
	#elif defined(sun) || defined(__sun)
	#include <stdlib.h>
	fs::path execname() {
		return getexecname();
	}
	#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>
	fs::path execname() {
		int mib[4];
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PATHNAME;
		mib[3] = -1;
		char buf[1024];
		size_t maxchars = sizeof(buf) - 1;
		size_t size = maxchars;
		sysctl(mib, 4, buf, &size, NULL, 0);
		if (size == 0 || size >= maxchars) return fs::path();
		buf[size] = '\0';
		return buf;
	}
	#elif defined(__linux__)
	#include <unistd.h>
	fs::path execname() {
		char buf[1024];
		ssize_t maxchars = sizeof(buf) - 1;
		ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
		if (size <= 0 || size >= maxchars) return fs::path();
		buf[size] = '\0';
		return buf;
	}
	#else
	fs::path execname() {
		return fs::path();
	}
	#endif
}
