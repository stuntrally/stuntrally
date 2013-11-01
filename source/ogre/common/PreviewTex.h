#pragma once
#include <OgreString.h>
#include <OgreTexture.h>


//  for loading images directly from disk path to memory and into texture

class PreviewTex
{
public:
	PreviewTex();
	//~PreviewTex();
	//bool Destroy();

	//  call first with size of texture (x,y) and name
	bool Create(int x, int y, Ogre::String texName);
	void Clear();

	//  load image from global path
	bool Load(Ogre::String path, bool force=false);

protected:
	int xSize, ySize;
	Ogre::String sName, curPath;
	Ogre::TexturePtr prvTex;
};
