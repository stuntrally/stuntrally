#include "pch.h"
#include "PreviewTex.h"
// #include <OgreResourceGroupManager.h>
// #include <OgreTextureManager.h>
// #include <OgreImage.h>
// #include <OgreDataStream.h>
// #include <fstream>
using namespace Ogre;
using Ogre::uint8;


PreviewTex::PreviewTex()
	:xSize(0), ySize(0)
{
}

bool PreviewTex::Create(int x, int y, String texName)
{
	xSize = x;  ySize = y;
	sName = texName;
	prvTex = TextureManager::getSingleton().createManual(
		sName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		TEX_TYPE_2D, x, y, 0,
		PF_BYTE_BGRA, TU_DEFAULT);  //TU_DYNAMIC_WRITE_ONLY_DISCARDABLE
		
	//Clear();
	return !prvTex.isNull();
}

bool PreviewTex::Load(String path, bool force)
{
	if (curPath == path && !force)  // check if same
		return false;
	curPath = path;
	
	bool loaded = false;
	std::ifstream ifs(path.c_str(), std::ios::binary|std::ios::in);
	if (ifs.is_open())
	{
		String ext;
		String::size_type id = path.find_last_of('.');
		if (id != String::npos)
		{
			ext = path.substr(id+1);
			DataStreamPtr data(new FileStreamDataStream(path, &ifs, false));
			Image img;
			img.load(data, ext);

			prvTex->getBuffer(0,0)->blitFromMemory(img.getPixelBox(0,0));
			//prvTex->unload();  prvTex->loadImage(img);  //same

			//prvTex->loadRawData
			//TextureManager::getSingleton().loadImage("PrvView",
			//	ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, img, TEX_TYPE_2D, 0, 1.0f);
			loaded = true;
		}
		ifs.close();
	}
	return loaded;
}


void PreviewTex::Clear()
{
	//  fill texture
	HardwarePixelBufferSharedPtr pixelBuffer = prvTex->getBuffer();
	pixelBuffer->lock(HardwareBuffer::HBL_DISCARD);
	const PixelBox& pixelBox = pixelBuffer->getCurrentLock();
	 
	uint8* pDest = static_cast<Ogre::uint8*>(pixelBox.data);
	 
	size_t j,i;
	for (j = 0; j < 1024; ++j)
	{
		for (i = 0; i < 1024; ++i)
		{
			*pDest++ = 255; // B
			*pDest++ = rand()%255; // G
			*pDest++ =   0; // R
			*pDest++ = 127; // A
		}
		pDest += pixelBox.getRowSkip() * PixelUtil::getNumElemBytes(pixelBox.format);
	}
	pixelBuffer->unlock();
}
