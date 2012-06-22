#include "MaterialInstance.hpp"

namespace sh
{
	MaterialInstance::MaterialInstance (const std::string& name)
		: mName(name)
	{
	}

	MaterialInstance::MaterialInstance ()
	{
	}

	void MaterialInstance::_setParentInstance (const std::string& name)
	{
		mParentInstance = name;
	}

	std::string MaterialInstance::_getParentInstance ()
	{
		return mParentInstance;
	}

	void MaterialInstance::_create (Platform* platform)
	{
		mMaterial = platform->createMaterial(mName);
	}

	void MaterialInstance::_createForConfiguration (Platform* platform, const std::string& configuration)
	{
		std::cout << "trying to create config " << configuration << " for material " << mName << std::endl;
	}

	Material* MaterialInstance::getMaterial ()
	{
		return mMaterial.get();
	}
}
