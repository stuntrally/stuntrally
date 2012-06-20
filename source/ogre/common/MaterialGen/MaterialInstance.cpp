#include "MaterialInstance.hpp"

namespace sh
{
	void MaterialInstance::_setParentInstance (const std::string& name)
	{
		mParentInstance = name;
	}

	std::string MaterialInstance::_getParentInstance ()
	{
		return mParentInstance;
	}
}
