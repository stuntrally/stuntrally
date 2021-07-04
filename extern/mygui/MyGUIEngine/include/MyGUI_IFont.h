/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_I_FONT_H_
#define MYGUI_I_FONT_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_ISerializable.h"
#include "MyGUI_IResource.h"
#include "MyGUI_FontData.h"

namespace MyGUI
{

	class ITexture;

	class MYGUI_EXPORT IFont :
		public IResource
	{
		MYGUI_RTTI_DERIVED( IFont )

	public:
		IFont() = default;
		~IFont() override = default;

		virtual const GlyphInfo* getGlyphInfo(Char _id) const = 0;

		virtual ITexture* getTextureFont() const = 0;

		virtual int getDefaultHeight() const = 0;
	};

} // namespace MyGUI

#endif // MYGUI_I_FONT_H_
