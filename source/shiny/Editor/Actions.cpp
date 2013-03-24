#include "Actions.hpp"

#include "../Main/Factory.hpp"

namespace sh
{

	void ActionDeleteMaterial::execute()
	{
		sh::Factory::getInstance().destroyMaterialInstance(mName);
	}

	void ActionCloneMaterial::execute()
	{
		sh::MaterialInstance* sourceMaterial = sh::Factory::getInstance().getMaterialInstance(mSourceName);
		std::string sourceMaterialParent = static_cast<sh::MaterialInstance*>(sourceMaterial->getParent())->getName();
		sh::MaterialInstance* material = sh::Factory::getInstance().createMaterialInstance(
					mDestName, sourceMaterialParent);
		sourceMaterial->copyAll(material, sourceMaterial, false);
	}

	void ActionSaveAll::execute()
	{
		sh::Factory::getInstance().saveAll();
	}

	void ActionChangeGlobalSetting::execute()
	{
		sh::Factory::getInstance().setGlobalSetting(mName, mNewValue);
	}

}
