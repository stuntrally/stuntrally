/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_FILE_LOG_LISTENER_H_
#define MYGUI_FILE_LOG_LISTENER_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_ILogListener.h"
#include <fstream>
#include <string>

namespace MyGUI
{

	class MYGUI_EXPORT FileLogListener :
		public ILogListener
	{
	public:
		//! @copydoc ILogListener::open()
		void open() override;
		//! @copydoc ILogListener::close()
		void close() override;
		//! @copydoc ILogListener::flush()
		void flush() override;
		//! @copydoc ILogListener::log(const std::string& _section, LogLevel _level, const struct tm* _time, const std::string& _message, const char* _file, int _line)
		void log(const std::string& _section, LogLevel _level, const struct tm* _time, const std::string& _message, const char* _file, int _line) override;

		/** Set listener's file name where it wirte log. */
		void setFileName(const std::string& _value);
		/** Get listener's file name where it wirte log. */
		const std::string& getFileName() const;

	private:
		std::ofstream mStream;
		std::string mFileName;
	};

} // namespace MyGUI

#endif // MYGUI_FILE_LOG_LISTENER_H_
