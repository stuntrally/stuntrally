#ifndef RESOURCEIMAGESETPOINTERFIX_H
#define RESOURCEIMAGESETPOINTERFIX_H

#include <MyGUI_IPointer.h>
#include <MyGUI_ResourceImageSet.h>

/// \brief Allows us to get the members of
///        ResourceImageSetPointer that we need.
/// \example MyGUI::FactoryManager::getInstance().registerFactory<ResourceImageSetPointerFix>("Resource", "ResourceImageSetPointer");
///          MyGUI::ResourceManager::getInstance().load("core.xml");
class ResourceImageSetPointerFix :
	public MyGUI::IPointer
{
	MYGUI_RTTI_DERIVED( ResourceImageSetPointerFix )

public:
	ResourceImageSetPointerFix();
	virtual ~ResourceImageSetPointerFix();

	virtual void deserialization(MyGUI::xml::ElementPtr _node, MyGUI::Version _version);

	virtual void setImage(MyGUI::ImageBox* _image);
	virtual void setPosition(MyGUI::ImageBox* _image, const MyGUI::IntPoint& _point);

	//and now for the whole point of this class, allow us to get
	//the hot spot, the image and the size of the cursor.
	virtual MyGUI::ResourceImageSetPtr getImageSet();
	virtual MyGUI::IntPoint getHotSpot();
	virtual MyGUI::IntSize getSize();
	virtual MyGUI::IntPoint getTexturePosition(); // position of the image on the texture

private:
	MyGUI::IntPoint mPoint;
	MyGUI::IntSize mSize;
	MyGUI::ResourceImageSetPtr mImageSet;
};

#endif
