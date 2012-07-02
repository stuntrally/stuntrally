#include "Configuration.hpp"

namespace sh
{
	void Configuration::addOverride (const std::string& name, const std::string& value)
	{
		mOverrides[name] = value;
	}

	std::map<std::string, std::string>* Configuration::getOverrides ()
	{
		return &mOverrides;
	}
}
