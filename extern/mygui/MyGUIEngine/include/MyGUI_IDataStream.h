/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_I_DATA_STREAM_H_
#define MYGUI_I_DATA_STREAM_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_Types.h"

namespace MyGUI
{

	class MYGUI_EXPORT IDataStream
	{
	public:
		virtual ~IDataStream() = default;

		virtual bool eof() = 0;
		virtual size_t size() = 0;
		virtual void readline(std::string& _source, Char _delim = '\n') = 0;
		virtual size_t read(void* _buf, size_t _count) = 0;
	};

} // namespace MyGUI

#endif // MYGUI_I_DATA_STREAM_H_
