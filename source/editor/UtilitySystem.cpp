#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include <boost/filesystem.hpp>
#include <Ogre.h>
#ifndef _WIN32
#include <dirent.h>
#endif
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
			std::stringstream msg;
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
			std::stringstream msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			LogO(msg.str());
	}	}
	else
		LogO("GetMat, can't open: " + filename);
	stream.close();
}


///  system file, dir
//-----------------------------------------------------------------------------------------------------------
namespace bfs = boost::filesystem;


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
