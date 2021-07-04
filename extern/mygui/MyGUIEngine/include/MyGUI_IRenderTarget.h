/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_I_RENDER_TARGET_H_
#define MYGUI_I_RENDER_TARGET_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_RenderTargetInfo.h"
#include <stddef.h>

namespace MyGUI
{

	class ITexture;
	class IVertexBuffer;

	class MYGUI_EXPORT IRenderTarget
	{
	public:
		virtual ~IRenderTarget() { }

		virtual void begin() = 0;
		virtual void end() = 0;

		virtual void doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count) = 0;

		virtual const RenderTargetInfo& getInfo() const = 0;
	};

} // namespace MyGUI

#endif // MYGUI_I_RENDER_TARGET_H_
