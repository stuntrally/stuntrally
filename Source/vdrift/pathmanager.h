#ifndef _PATHMANAGER_H
#define _PATHMANAGER_H

class PATHMANAGER
{
	private:
		std::string home_directory;
		std::string settings_path;
		std::string data_directory;
		std::string profile_suffix;
		
		#ifndef _WIN32
		bool DirectoryExists(std::string filename) const
		{
			DIR *dp;
			dp = opendir(filename.c_str());
			if (dp != NULL) {
				closedir(dp);
				return true;
			} else {
				return false;
			}
		}
		#endif
		
		void MakeDir(const std::string & dir)
		{
			#ifndef _WIN32
			mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			#else
			//mkdir(dir.c_str());
			#endif
		}
	
	public:
		void Init(std::ostream & info_output, std::ostream & error_output);
		bool GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension="") const; ///<optionally filter for the given extension

		std::string GetDataPath() const {				return data_directory;  }
		//std::string GetTrackRecordsPath() const {		return settings_path  + "/records" + profile_suffix;  }
		std::string GetSettingsFile() const {			return "_game.cfg";  }  //settings_path + "/VDrift.config"+profile_suffix;  }
		std::string GetLogFile() const {				return "_log.txt";  }  //settings_path + "/log.txt";  }
		std::string GetTrackPath() const {				return data_directory + "/tracks";  }
		std::string GetCarPath() const {				return data_directory + "/cars";  }
		std::string GetCarControlsFile() const {		return "_controls.cfg";  }  //settings_path+"/controls.config"+profile_suffix;
		std::string GetDefaultCarControlsFile() const {	return "_controls.cfg";  }//-
		std::string GetGenericSoundPath() const {		return data_directory + "/sounds";  }
		std::string GetDriverPath() const {				return data_directory + "/drivers";  }
		std::string GetReplayPath() const {				return data_directory + "/replays";  }
		
		bool FileExists(const std::string & filename) const
		{
			std::ifstream test;
			test.open(filename.c_str());
			if (test)
			{
				test.close();
				return true;
			}
			else
				return false;
		}

		///only call this before Init()
		void SetProfile ( const std::string& value )
		{
			assert(data_directory.empty()); //assert that Init() hasn't been called yet
			profile_suffix = "."+value;
		}
	
};

#endif
