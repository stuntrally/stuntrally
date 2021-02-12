#include "pch.h"
#include "sdlcursormanager.hpp"

#include <OgreHardwarePixelBuffer.h>
#include <OgreRoot.h>

namespace SFO
{

    SDLCursorManager::SDLCursorManager() :
        mEnabled(false),
        mInitialized(false)
    {
    }

    SDLCursorManager::~SDLCursorManager()
    {
        CursorMap::const_iterator curs_iter = mCursorMap.begin();

        while(curs_iter != mCursorMap.end())
        {
            SDL_FreeCursor(curs_iter->second);
            ++curs_iter;
        }

        mCursorMap.clear();
    }

    void SDLCursorManager::setEnabled(bool enabled)
    {
        if(mInitialized && enabled == mEnabled)
            return;

        mInitialized = true;
        mEnabled = enabled;

        //turn on hardware cursors
        if(enabled)
        {
            if (!mCurrentCursor.empty())
                _setGUICursor(mCurrentCursor);
        }
        //turn off hardware cursors
        else
        {
            SDL_ShowCursor(SDL_FALSE);
        }
    }

    bool SDLCursorManager::cursorChanged(const std::string &name)
    {
        mCurrentCursor = name;

        CursorMap::const_iterator curs_iter = mCursorMap.find(name);

        //we have this cursor
        if(curs_iter != mCursorMap.end())
        {
            _setGUICursor(name);

            return false;
        }
        else
        {
            //they should get back to us with more info
            return true;
        }
    }

    void SDLCursorManager::_setGUICursor(const std::string &name)
    {
        CursorMap::const_iterator curs_iter = mCursorMap.find(name);
        assert (curs_iter != mCursorMap.end());

        SDL_SetCursor(curs_iter->second);
    }

    void SDLCursorManager::receiveCursorInfo(const std::string& name, Ogre::TexturePtr tex, Uint8 left, Uint8 top, Uint8 size_x, Uint8 size_y, Uint8 hotspot_x, Uint8 hotspot_y)
    {
        _createCursorFromResource(name, tex, left, top, size_x, size_y, hotspot_x, hotspot_y);
    }

    /// \brief creates an SDL cursor from an Ogre texture
    void SDLCursorManager::_createCursorFromResource(const std::string& name, Ogre::TexturePtr tex, Uint8 left, Uint8 top, Uint8 size_x, Uint8 size_y, Uint8 hotspot_x, Uint8 hotspot_y)
    {
        if (mCursorMap.find(name) != mCursorMap.end())
            return;

        // blit to memory
        std::vector<Ogre::uint32> data;
        data.resize(size_x*size_y);
        Ogre::PixelBox destImage(size_x, size_y, 1, tex->getFormat(), &data[0]);
        Ogre::Box srcBox(left, top, left+size_x, top+size_y);
        tex->getBuffer()->blitToMemory(srcBox, destImage);

        SDL_Surface* surf = SDL_CreateRGBSurface(0,size_x,size_y,32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);


        //copy the Ogre texture to an SDL surface
        for(size_t x = 0; x < size_x; ++x)
        {
            for(size_t y = 0; y < size_y; ++y)
            {
                Ogre::ColourValue clr = destImage.getColourAt(x, y, 0);

                //set the pixel on the SDL surface to the same value as the Ogre texture's
                _putPixel(surf, x, y, SDL_MapRGBA(surf->format, clr.r*255, clr.g*255, clr.b*255, clr.a*255));
            }
        }

        //set the cursor and store it for later
        SDL_Cursor* curs = SDL_CreateColorCursor(surf, hotspot_x, hotspot_y);
        mCursorMap.insert(CursorMap::value_type(std::string(name), curs));

        //clean up
        SDL_FreeSurface(surf);

        _setGUICursor(name);
    }

    void SDLCursorManager::_putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
    {
        int bpp = surface->format->BytesPerPixel;
        /* Here p is the address to the pixel we want to set */
        Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

        switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
        }
    }
}
