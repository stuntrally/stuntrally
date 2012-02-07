#include <OgrePlatform.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX

#include "HWMouse.h"

#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreResourceGroupManager.h>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

using namespace Ogre;

HWMouse::HWMouse(size_t windowID, const int xhot, const int yhot, const std::string& filename)
{
	Window x11Window;
	Display* x11Display;
	Cursor x11Cursor;
	
	x11Window = windowID;
	x11Display = XOpenDisplay(0);
	
	Image im;
	im.load(filename, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	
    XcursorImage* cursorImage = XcursorImageCreate(im.getWidth(), im.getHeight());
    cursorImage->xhot = xhot;
    cursorImage->yhot = yhot;
    
	// fill pixels
	int c = 0;
	for (int y=0; y<im.getHeight(); ++y)
	{
		for (int x=0; x<im.getWidth(); ++x)
		{
			const ColourValue& cv = im.getColourAt(x, y, 0);
			unsigned char r,g,b,a;
			r = cv.r*255; g = cv.g*255; b = cv.b*255; a = cv.a*255;
			
			cursorImage->pixels[c++] = (a<<24) | (r<<16) | (g<<8) | (b);
		}
	}
	
	x11Cursor = XcursorImageLoadCursor(x11Display, cursorImage);
	XcursorImageDestroy(cursorImage);
	
	XDefineCursor(x11Display, x11Window, x11Cursor);
    
    XFlush(x11Display);
}

HWMouse::~HWMouse()
{
	//XUndefineCursor(x11Display, x11Cursor);
	//XFreeCursor(x11Display, x11Cursor);
	//XCloseDisplay(x11Display);
}

#endif
