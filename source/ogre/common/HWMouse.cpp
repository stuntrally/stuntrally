#include "pch.h"
#include "HWMouse.h"

#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreResourceGroupManager.h>

#include <OgrePlatform.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	#include <X11/Xlib.h>
	#include <X11/Xcursor/Xcursor.h>
#endif

using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	static Window x11Window;
	static Display* x11Display;
	static Cursor x11Cursor;
	static Cursor x11Cursor_hidden; // hidden, empty cursor
#endif
	
HWMouse::HWMouse(size_t windowID, const int xhot, const int yhot, const std::string& filename) :
	mVisible(0)
{
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
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
	
	// Create a blank cursor
	Pixmap bm_no;
	XColor black, dummy;
	Colormap colormap;
	static char no_data[] = { 0,0,0,0,0,0,0,0 };
	colormap = DefaultColormap( x11Display, DefaultScreen(x11Display) );
	XAllocNamedColor( x11Display, colormap, "black", &black, &dummy );
	bm_no = XCreateBitmapFromData( x11Display, x11Window, no_data, 8, 8 );
	x11Cursor_hidden = XCreatePixmapCursor( x11Display, bm_no, bm_no, &black, &black, 0, 0 );
	
	show();
#endif
}

HWMouse::~HWMouse()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	//if (mVisible)
		//XUndefineCursor(x11Display, x11Cursor);
	//else
		//XUndefineCursor(x11Display, x11Cursor_hidden);
	//XFreeCursor(x11Display, x11Cursor_hidden);
	//XFreeCursor(x11Display, x11Cursor);
	
	// this seems to interfere with OIS (causing a X11 BadWindow error)
	//XCloseDisplay(x11Display);
#endif
}

void HWMouse::show()
{
	if (mVisible) return;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	XDefineCursor(x11Display, x11Window, x11Cursor);
	XFlush(x11Display);
#endif
	mVisible = true;
}

void HWMouse::hide()
{
	if (!mVisible) return;
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	XDefineCursor(x11Display, x11Window, x11Cursor_hidden);
	XFlush(x11Display);
#endif
	mVisible = false;
}
