#pragma once
#include <string>
#include <iostream>
#include "texture.h"
#include "reseatable_reference.h"

class TEXTURELIBRARY
{
public:
	TEXTURELIBRARY() : repeatu(true), repeatv(true) {}
	
	///set the path to the files, minus the trailing /
	void SetLibraryPath(const std::string & newpath)
	{
		librarypath = newpath;
	}

	///filename is the path to the resource minus the path prefix
	///returns true if the file was loaded successfully or is already loaded
	bool Load(const std::string & filename, std::ostream & error_output, bool mipmap=true, bool allownpot=true)
	{
		resourcemap_iterator existing = resourcemap.find(filename);
		if (existing != resourcemap.end()) //already loaded
			return true;

		std::string filepath = librarypath+"/"+filename;
		TEXTUREINFO texinfo(filepath);
		texinfo.SetMipMap(mipmap);
		texinfo.SetAllowNonPowerOfTwo(allownpot);
		texinfo.SetRepeat(repeatu, repeatv);
		return resourcemap[filename].Load(texinfo, error_output, "large");
	}

	reseatable_reference <TEXTURE_GL> GetResource(const std::string & filename)
	{
		resourcemap_iterator resource = resourcemap.find(filename);
		if (resource != resourcemap.end())
			return reseatable_reference <TEXTURE_GL> (resource->second);
		else
			return reseatable_reference <TEXTURE_GL> ();
	}
	
	void SetRepeat(bool u, bool v)
	{
		repeatu = u;
		repeatv = v;
	}
	
	void Clear()
	{
		resourcemap.clear();
	}

private:
	std::string librarypath;
	std::map <std::string, TEXTURE_GL> resourcemap;
	typedef std::map <std::string, TEXTURE_GL>::iterator resourcemap_iterator;
	bool repeatu, repeatv;
	
	bool FileExists(const std::string & filename)
	{
		std::ifstream f(filename.c_str());
		return f;
	}
};
