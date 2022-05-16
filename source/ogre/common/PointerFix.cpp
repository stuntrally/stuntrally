#include "pch.h"
#include "PointerFix.h"

#include <MyGUI_PointerManager.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_RotatingSkin.h>
#include <MyGUI_Gui.h>


ResourceImageSetPointerFix::ResourceImageSetPointerFix()
	: mImageSet(NULL)
{
}

ResourceImageSetPointerFix::~ResourceImageSetPointerFix()
{
}

void ResourceImageSetPointerFix::deserialization(MyGUI::xml::ElementPtr _node, MyGUI::Version _version)
{
	Base::deserialization(_node, _version);

	MyGUI::xml::ElementEnumerator info = _node->getElementEnumerator();
	while (info.next("Property"))
	{
		const std::string& key = info->findAttribute("key");
		const std::string& value = info->findAttribute("value");

		if (key == "Point")
			mPoint = MyGUI::IntPoint::parse(value);
		else if (key == "Size")
			mSize = MyGUI::IntSize::parse(value);
		else if (key == "Resource")
			mImageSet = MyGUI::ResourceManager::getInstance().getByName(value)->castType<MyGUI::ResourceImageSet>();
	}
}

void ResourceImageSetPointerFix::setImage(MyGUI::ImageBox* _image)
{
	if (mImageSet != NULL)
		_image->setItemResourceInfo(mImageSet->getIndexInfo(0, 0));
}

void ResourceImageSetPointerFix::setPosition(MyGUI::ImageBox* _image, const MyGUI::IntPoint& _point)
{
	_image->setCoord(_point.left - mPoint.left, _point.top - mPoint.top, mSize.width, mSize.height);
}

MyGUI::ResourceImageSetPtr ResourceImageSetPointerFix:: getImageSet()
{
	return mImageSet;
}

MyGUI::IntPoint ResourceImageSetPointerFix::getHotSpot()
{
	return mPoint;
}

MyGUI::IntPoint ResourceImageSetPointerFix::getTexturePosition()
{
	MyGUI::EnumeratorGroupImage groupImages = mImageSet->getEnumerator();
	while (groupImages.next())
	{
		const MyGUI::GroupImage& image = groupImages.current();
		for (auto it = image.indexes.begin(); it != image.indexes.end(); ++it)
		{
			const MyGUI::IndexImage& index = *it;
			for (auto fIt = index.frames.begin(); fIt != index.frames.end(); ++fIt)
				return *fIt;
		}
	}
	throw std::runtime_error("Unable to find an image in the cursor");
}

MyGUI::IntSize ResourceImageSetPointerFix::getSize()
{
	return mSize;
}
