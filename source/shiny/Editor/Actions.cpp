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

		material->setSourceFile(sourceMaterial->getSourceFile());
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
		sh::Factory::getInstance().createConfiguration(mName);
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

	void ActionSetMaterialProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		m->setProperty(mKey, sh::makeProperty(new sh::StringValue(mValue)));
	}

	void ActionDeleteMaterialProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		m->deleteProperty(mKey);
	}

	void ActionSetPassProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		m->getPasses()->at(mPassIndex).setProperty (mKey, sh::makeProperty(new sh::StringValue(mValue)));
	}

	void ActionDeletePassProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		m->getPasses()->at(mPassIndex).deleteProperty(mKey);
	}

	void ActionSetShaderProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		m->getPasses()->at(mPassIndex).mShaderProperties.setProperty (mKey, sh::makeProperty(new sh::StringValue(mValue)));
	}

	void ActionDeleteShaderProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		m->getPasses()->at(mPassIndex).mShaderProperties.deleteProperty (mKey);
	}

	void ActionSetTextureProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		assert (m->getPasses()->at(mPassIndex).getTexUnits()->size() > mTextureIndex);
		m->getPasses()->at(mPassIndex).getTexUnits()->at(mTextureIndex).setProperty(mKey, sh::makeProperty(new sh::StringValue(mValue)));
	}

	void ActionDeleteTextureProperty::execute()
	{
		sh::MaterialInstance* m = sh::Factory::getInstance().getMaterialInstance(mName);
		assert (m->getPasses()->size() > mPassIndex);
		assert (m->getPasses()->at(mPassIndex).getTexUnits()->size() > mTextureIndex);
		m->getPasses()->at(mPassIndex).getTexUnits()->at(mTextureIndex).deleteProperty(mKey);
	}
}
