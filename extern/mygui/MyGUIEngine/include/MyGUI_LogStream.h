/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_LOG_STREAM_H_
#define MYGUI_LOG_STREAM_H_

#include "MyGUI_Prerequest.h"
#include <cstring>
#include <sstream>

namespace MyGUI
{

	class MYGUI_EXPORT LogStream
	{
	public:
		struct End { };

	public:
		std::string operator << (const End& /*_endl*/)
		{
			return mStream.str();
		}

		template <typename T>
		LogStream& operator << (const T& _value)
		{
			mStream << _value;
			return *this;
		}

	private:
		std::ostringstream mStream;
	};

} // namespace MyGUI

#endif // MYGUI_LOG_STREAM_H_
