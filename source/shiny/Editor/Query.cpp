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


}

}
