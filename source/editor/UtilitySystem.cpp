#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include <boost/filesystem.hpp>
using namespace MyGUI;
using namespace Ogre;


///  Get Materials
//-----------------------------------------------------------------------------------------------------------

void CGui::GetMaterials(String filename, bool clear, String type)
{
	if (clear)
		vsMaterials.clear();
	
	DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(filename);
	if(!stream.isNull())
	{	try
		{	while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type/*"material"*/))
				{
					//LogO(line);
					Ogre::vector<String>::type vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (Ogre::vector<String>::type::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						if (skipFirst)
						{	skipFirst = false;
							continue;	}
						
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			LogO(msg.str());
	}	}
	stream->close();
}

void CGui::GetMaterialsMat(String filename, bool clear, String type)
{
	if (clear)
		vsMaterials.clear();
	
	std::ifstream stream(filename.c_str(), std::ifstream::in);
	if (!stream.fail())
	{	try
		{	while(!stream.eof())
			{
				char ch[256+2];
				stream.getline(ch,256);
				std::string line = ch;
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type/*"material"*/))
				{
					//LogO(line);
					Ogre::vector<String>::type vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (Ogre::vector<String>::type::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							if (skipFirst)
							{	skipFirst = false;  continue;	}

							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			LogO(msg.str());
	}	}
	stream.close();
}


///  system file, dir
//-----------------------------------------------------------------------------------------------------------
namespace bfs = boost::filesystem;

bool CGui::TrackExists(String name/*, bool user*/)
{	// ignore letters case..
	for (strlist::const_iterator it = liTracks.begin(); it != liTracks.end(); ++it)
		if (*it == name)  return true;
	for (strlist::const_iterator it = liTracksUser.begin(); it != liTracksUser.end(); ++it)
		if (*it == name)  return true;
	return false;
}

bool CGui::Rename(String from, String to)
{
	try
	{	if (bfs::exists(from.c_str()))
			bfs::rename(from.c_str(), to.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Renaming file " + from + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::Delete(String file)
{
	try
	{	bfs::remove(file.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Deleting file " + file + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::DeleteDir(String dir)
{
	try
	{	bfs::remove_all(dir.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Deleting directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::CreateDir(String dir)
{
	try
	{	bfs::create_directory(dir.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Creating directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool CGui::Copy(String file, String to)
{
	try
	{	if (bfs::exists(to.c_str()))
			bfs::remove(to.c_str());

		if (bfs::exists(file.c_str()))
			bfs::copy_file(file.c_str(), to.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Copying file " + file + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}


void App::UpdWndTitle()
{
	String s = String("SR Editor  track: ") + pSet->gui.track;
	if (pSet->gui.track_user)  s += "  *user*";

	SDL_SetWindowTitle(mSDLWindow, s.c_str());
}

String CGui::TrkDir() {
	int u = pSet->gui.track_user ? 1 : 0;		return pathTrk[u] + pSet->gui.track + "/";  }

String CGui::PathListTrk(int user) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + sListTrack;  }
	
String CGui::PathListTrkPrv(int user, String track){
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + track + "/preview/";  }
	
String CGui::PathCopyTrk(int user){
	int u = user == -1 ? bCopyTrackU : user;	return pathTrk[u] + sCopyTrack;  }



//  vdr util, get tracks
//----------------------------------------------------------------------------------------------------------------------
bool string_compare(const std::string& s1, const std::string& s2)
{
	return strcmp(s1.c_str(), s2.c_str()) != 0;
}

bool CGui::GetFolderIndex(std::string dirpath, std::list <std::string>& dirlist, std::string extension)
{
#ifndef _WIN32
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dirpath.c_str());
	if (dp != NULL)
	{
		while ( ( ep = readdir( dp ) ) )
		{
			//puts (ep->d_name);
			std::string newname = ep->d_name;
			if (newname[0] != '.')
			{
				dirlist.push_back(newname);
			}
		}
		(void) closedir(dp);
	}
	else
		return false;
#else
	HANDLE	  hList;
	TCHAR	  szDir[MAX_PATH+1];
	WIN32_FIND_DATA FileData;

	// Get the proper directory path
	sprintf(szDir, "%s\\*", dirpath.c_str ());

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
					dirlist.push_back (FileData.cFileName);
				}
			}
		}
	}

	FindClose(hList);
#endif
	
	//remove non-matcthing extensions
	if (!extension.empty())
	{
		std::list <std::list <std::string>::iterator> todel;
		for (std::list <std::string>::iterator i = dirlist.begin(); i != dirlist.end(); ++i)
		{
			if (i->find(extension) != i->length()-extension.length())
				todel.push_back(i);
		}
		
		for (std::list <std::list <std::string>::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			dirlist.erase(*i);
	}
	
	dirlist.sort();
	//dirlist.sort(dirlist.begin(), dirlist.end(), string_compare);  //?-
	return true;
}
