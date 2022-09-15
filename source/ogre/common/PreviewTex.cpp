#include "pch.h"
#include "RenderConst.h"
#include "PreviewTex.h"
#include "Def_Str.h"
#include <OgreException.h>
#include <fstream>
#include <OgreHardwarePixelBuffer.h>
#include <OgreResourceGroupManager.h>
#include <OgreTextureManager.h>
#include <OgreImage.h>
#include <OgreDataStream.h>
using namespace Ogre;
using Ogre::uint8;


PreviewTex::PreviewTex()
	:xSize(0), ySize(0)
{	}


//  1 set name if size unknown
void PreviewTex::SetName(String texName)
{
	sName = texName;
}

//  1 create (if known size)
bool PreviewTex::Create(int x, int y, String texName)
{
	xSize = x;  ySize = y;
	sName = texName;
	prvTex = TextureManager::getSingleton().createManual(
		sName, rgDef, TEX_TYPE_2D,
		x, y, 5, //par mipmaps
		PF_BYTE_BGRA, TU_DEFAULT);  //TU_DYNAMIC_WRITE_ONLY_DISCARDABLE
		
	//Clear();
	return (bool)prvTex; //.isNull();
}

//  3 destroy
void PreviewTex::Destroy()
{
	if (!TextureManager::getSingleton().resourceExists(sName))
		return;
	try
	{
		TextureManager::getSingleton().remove(sName);
	}
	catch (Exception ex)
	{
		LogO(std::string("! PRV TEX dest:") + sName +" exc:" + ex.what());
	}
}


//  2 load image from path
bool PreviewTex::Load(String path, bool force,  uint8 b, uint8 g, uint8 r, uint8 a)
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

			if (!prvTex)
				Create(img.getWidth(), img.getHeight(), sName);
				
			//LogO(path+" "+toStr(img.getWidth())+" "+toStr(img.getHeight()));

			if (img.getWidth() == prvTex->getWidth() &&
				img.getHeight() == prvTex->getHeight())  // same dim
				prvTex->getBuffer()->blitFromMemory(img.getPixelBox());
			else
				Clear(b,g,r,a);

			//prvTex->setNumMipmaps(5);
			//prvTex->unload();  prvTex->loadImage(img);  //same

			//prvTex->loadRawData
			//TextureManager::getSingleton().loadImage("PrvView",
			//	rgDef, img, TEX_TYPE_2D, 0, 1.0f);
			loaded = true;
		}
		ifs.close();
	}else
		Clear(b,g,r,a);

	return loaded;
}

bool PreviewTex::Load(String path, bool force)
{
	return Load(path, force, 100, 90, 80, 120);
}


void PreviewTex::Clear(const uint8 b, const uint8 g, const uint8 r, const uint8 a)
{
	if (!prvTex)  return;
	//  fill texture
	HardwarePixelBufferSharedPtr pb = prvTex->getBuffer();
	pb->lock(HardwareBuffer::HBL_DISCARD);

	const PixelBox& pixelBox = pb->getCurrentLock();
	uint8* pDest = static_cast<uint8*>(pixelBox.data);
	 
	size_t j,i;
	for (j = 0; j < ySize; ++j)
	{
		for (i = 0; i < xSize; ++i)   // B,G,R,A
		{	*pDest++ = b;  *pDest++ = g;  *pDest++ = r;  *pDest++ = a;  }

		pDest += pixelBox.getRowSkip() * PixelUtil::getNumElemBytes(pixelBox.format);
	}
	pb->unlock();
}


//  utility for terrain textures
//  copies other texture's r channel to this texture's alpha
//  note: both must be same size
bool PreviewTex::LoadTer(String sRgb, String sAa, float defA)
{
	curPath = sRgb;
	bool loaded = false;
	std::ifstream ifR(sRgb.c_str(), std::ios::binary|std::ios::in);
	std::ifstream ifA(sAa.c_str(),  std::ios::binary|std::ios::in);
	String exR;  String::size_type idR = sRgb.find_last_of('.');
	String exA;  String::size_type idA = sAa.find_last_of('.');

	//  no alpha, use default const value
	if (ifR.is_open() && !ifA.is_open() &&
		idR != String::npos)
	{
		exR = sRgb.substr(idR+1);  exA = sAa.substr(idA+1);
		DataStreamPtr dataR(new FileStreamDataStream(sRgb, &ifR, false));
		Image imR; 	imR.load(dataR, exR);
	
		PixelBox pbR = imR.getPixelBox();
		//uchar* pR = static_cast<uchar*>(pbR.data);
		//int aR = pbR.getRowSkip() * PixelUtil::getNumElemBytes(pbR.format);

		xSize = imR.getWidth();  ySize = imR.getHeight();
		prvTex = TextureManager::getSingleton().createManual(
			sName, rgDef, TEX_TYPE_2D,
			xSize, ySize, 5,
			PF_BYTE_BGRA, TU_DEFAULT);

		//  fill texture  rgb,a
		HardwarePixelBufferSharedPtr pt = prvTex->getBuffer();
		pt->lock(HardwareBuffer::HBL_DISCARD);

		const PixelBox& pb = pt->getCurrentLock();
		uint8* pD = static_cast<uint8*>(pb.data);
		int aD = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
		 
		float fA = defA * 255.f;
		size_t j,i;
		for (j = 0; j < ySize; ++j)
		{
			for (i = 0; i < xSize; ++i)   // B,G,R,A
			{	
				ColourValue cR = pbR.getColourAt(i,j,0);
				*pD++ = cR.b * 255.f;
				*pD++ = cR.g * 255.f;
				*pD++ = cR.r * 255.f;
				*pD++ = fA;
			}
			pD += aD;
		}
		pt->unlock();
	
		loaded = true;
	}
	else
	if (ifR.is_open() && ifA.is_open() &&
		idR != String::npos && idA != String::npos)
	{
		exR = sRgb.substr(idR+1);  exA = sAa.substr(idA+1);
		DataStreamPtr dataR(new FileStreamDataStream(sRgb, &ifR, false));
		DataStreamPtr dataA(new FileStreamDataStream(sAa,  &ifA, false));
		Image imR; 	imR.load(dataR, exR);
		Image imA; 	imA.load(dataA, exA);
	
		PixelBox pbR = imR.getPixelBox();
		PixelBox pbA = imA.getPixelBox();
		//uchar* pR = static_cast<uchar*>(pbR.data);
		//uchar* pA = static_cast<uchar*>(pbA.data);
		//int aR = pbR.getRowSkip() * PixelUtil::getNumElemBytes(pbR.format);
		//int aA = pbA.getRowSkip() * PixelUtil::getNumElemBytes(pbA.format);

		xSize = imR.getWidth();  ySize = imR.getHeight();
		prvTex = TextureManager::getSingleton().createManual(
			sName, rgDef, TEX_TYPE_2D,
			xSize, ySize, 5,
			PF_BYTE_BGRA, TU_DEFAULT);

		//  fill texture  rgb,a
		HardwarePixelBufferSharedPtr pt = prvTex->getBuffer();
		pt->lock(HardwareBuffer::HBL_DISCARD);

		const PixelBox& pb = pt->getCurrentLock();
		uint8* pD = static_cast<uint8*>(pb.data);
		int aD = pb.getRowSkip() * PixelUtil::getNumElemBytes(pb.format);
		 
		size_t j,i;
		for (j = 0; j < ySize; ++j)
		{
			for (i = 0; i < xSize; ++i)   // B,G,R,A
			{	
				ColourValue cR = pbR.getColourAt(i,j,0);
				ColourValue cA = pbA.getColourAt(i,j,0);
				*pD++ = cR.b * 255.f;
				*pD++ = cR.g * 255.f;
				*pD++ = cR.r * 255.f;
				*pD++ = cA.r * 255.f;
			}
			pD += aD;
		}
		pt->unlock();
	
		loaded = true;
	}else
		Clear();

	return loaded;
}
