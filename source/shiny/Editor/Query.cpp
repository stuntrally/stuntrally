#include "Query.hpp"

#include "../Main/Factory.hpp"

namespace sh
{

void Query::execute()
{
	executeImpl();
	mDone = true;
}

ConfigurationQuery::ConfigurationQuery(const std::string &name)
	: mName(name)
{
}

void ConfigurationQuery::executeImpl()
{
	sh::Factory::getInstance().listConfigurationSettings(mName, mProperties);
}

void MaterialQuery::executeImpl()
{
	sh::MaterialInstance* instance = sh::Factory::getInstance().getMaterialInstance(mName);

	if (instance->getParent())
		mParent = static_cast<sh::MaterialInstance*>(instance->getParent())->getName();

	const sh::PropertyMap& ourProperties = instance->listProperties();

	for (PropertyMap::const_iterator it = ourProperties.begin(); it != ourProperties.end(); ++it)
	{
		mProperties[it->first] = MaterialProperty (
					retrieveValue<sh::StringValue>(instance->getProperty(it->first), NULL).get(),
					MaterialProperty::Misc, false);
	}

	sh::PropertySetGet* parent = instance;
	while (parent = parent->getParent())
	{
		const sh::PropertyMap& parentProperties = parent->listProperties();

		for (PropertyMap::const_iterator it = parentProperties.begin(); it != parentProperties.end(); ++it)
		{
			if (ourProperties.find(it->first) == ourProperties.end())
				mProperties[it->first] = MaterialProperty (
							retrieveValue<sh::StringValue>(parent->getProperty(it->first), NULL).get(),
							MaterialProperty::Misc, true);
		}
	}

	std::vector<MaterialInstancePass>* passes = instance->getPasses();
	for (std::vector<MaterialInstancePass>::iterator it = passes->begin(); it != passes->end(); ++it)
	{
		mPasses.push_back(PassInfo());

		const sh::PropertyMap& passProperties = it->listProperties();
		for (PropertyMap::const_iterator pit = passProperties.begin(); pit != passProperties.end(); ++pit)
		{
			PropertyValuePtr property = it->getProperty(pit->first);
			if (typeid(*property).name() == typeid(sh::LinkedValue).name())
				mPasses.back().mProperties[pit->first] = "$" + property->_getStringValue();
			else
				mPasses.back().mProperties[pit->first] =
						retrieveValue<sh::StringValue>(property, NULL).get();
		}

		const sh::PropertyMap& shaderProperties = it->mShaderProperties.listProperties();
		for (PropertyMap::const_iterator pit = shaderProperties.begin(); pit != shaderProperties.end(); ++pit)
		{
			PropertyValuePtr property = it->mShaderProperties.getProperty(pit->first);
			if (typeid(*property).name() == typeid(sh::LinkedValue).name())
				mPasses.back().mShaderProperties[pit->first] = "$" + property->_getStringValue();
			else
				mPasses.back().mShaderProperties[pit->first] =
						retrieveValue<sh::StringValue>(property, NULL).get();
		}

		std::vector<MaterialInstanceTextureUnit>* texUnits = &it->mTexUnits;
		for (std::vector<MaterialInstanceTextureUnit>::iterator tIt = texUnits->begin(); tIt != texUnits->end(); ++tIt)
		{
			mPasses.back().mTextureUnits.push_back(TextureUnitInfo());
			mPasses.back().mTextureUnits.back().mName = tIt->getName();
			const sh::PropertyMap& unitProperties = tIt->listProperties();
			for (PropertyMap::const_iterator pit = unitProperties.begin(); pit != unitProperties.end(); ++pit)
			{
				PropertyValuePtr property = tIt->getProperty(pit->first);
				if (typeid(*property).name() == typeid(sh::LinkedValue).name())
					mPasses.back().mTextureUnits.back().mProperties[pit->first] = "$" + property->_getStringValue();
				else
					mPasses.back().mTextureUnits.back().mProperties[pit->first] =
							retrieveValue<sh::StringValue>(property, NULL).get();
			}
		}
	}

}

}
