#ifndef SH_PREPROCESSOR_H
#define SH_PREPROCESSOR_H

#include <string>

namespace sh
{
	/**
	 * @brief A simple interface for the boost::wave preprocessor
	 */
	class Preprocessor
	{
	public:
		static std::string preprocess (std::string source, const std::string& includePath);
	};
}

#endif
