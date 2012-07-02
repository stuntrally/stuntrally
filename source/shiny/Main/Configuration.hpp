#ifndef SH_CONFIGURATION_H
#define SH_CONFIGURATION_H

#include <string>
#include <map>

namespace sh
{
	/**
	 * @brief
	 * a \a Configuration allows you to create a specialized set of shaders for specific purposes. \n
	 * for example, you might want to not include shadows in reflection targets. \n
	 * a \a Configuration can override any property in a pass (and thus, in the shader used by the pass)
	 */
	class Configuration
	{
	public:
		void addOverride (const std::string& name, const std::string& value);

		std::map<std::string, std::string>* getOverrides();

	private:
		std::map<std::string, std::string> mOverrides;
	};
}

#endif
