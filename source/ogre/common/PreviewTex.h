#pragma once
#include <OgreString.h>
#include <OgreSharedPtr.h>
#include <OgreTexture.h>


//  for loading images directly from disk path to memory and into texture

class PreviewTex
{
public:
	PreviewTex();
	//~PreviewTex();

	//  call first with size of texture (x,y) and name
	void SetName(Ogre::String texName);
	bool Create(int x, int y, Ogre::String texName);
	void Clear(const Ogre::uint8 b=100, const Ogre::uint8 g=90, const Ogre::uint8 r=80, const Ogre::uint8 a=120);
	void Destroy();

	//  load image from global path
	bool Load(Ogre::String path, bool force=false);
	bool Load(Ogre::String path, bool force,  Ogre::uint8 b, Ogre::uint8 g, Ogre::uint8 r, Ogre::uint8 a);
	//  for terrain textures
	bool LoadTer(Ogre::String rgb, Ogre::String a, float defA = 0.f);

protected:
	int xSize, ySize;
	Ogre::String sName, curPath;
	Ogre::TexturePtr prvTex;
};
