#ifndef SH_PREPROCESSOR_H
#define SH_PREPROCESSOR_H

#include <string>
#include <vector>

namespace sh
{
	/**
	 * @brief A simple interface for the boost::wave preprocessor
	 */
	class Preprocessor
	{
	public:
		/**
		 * @brief preprocess a shader source string
		 * @param source source string
		 * @param includePath path to search for includes (that are included with #include)
		 * @param definitions macros to predefine (vector of strings of the format MACRO=value, or just MACRO to define it as 1)
		 * @param name name to use for error message
		 * @return processed string
		 */
		static std::string preprocess (std::string source, const std::string& includePath, std::vector<std::string> definitions, const std::string& name);
	};
}

#endif
