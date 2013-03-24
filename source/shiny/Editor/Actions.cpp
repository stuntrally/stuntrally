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

	void ActionCreateConfiguration::execute()
	{
		sh::Configuration newConfiguration;
		sh::Factory::getInstance().registerConfiguration(mName, newConfiguration);
	}

	void ActionDeleteConfiguration::execute()
	{
		sh::Factory::getInstance().destroyConfiguration(mName);
	}

	void ActionChangeConfiguration::execute()
	{
		sh::Configuration* c = sh::Factory::getInstance().getConfiguration(mName);
		c->setProperty(mKey, sh::makeProperty(new sh::StringValue(mValue)));
		sh::Factory::getInstance().notifyConfigurationChanged();
	}

	void ActionDeleteConfigurationProperty::execute()
	{
		sh::Configuration* c = sh::Factory::getInstance().getConfiguration(mName);
		c->deleteProperty(mKey);
		sh::Factory::getInstance().notifyConfigurationChanged();
	}
}
