#include "Factory.hpp"

#include "Platform.hpp"
#include "DefinitionLoader.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);

		mDefinitionLoader = new DefinitionLoader(mPlatform->getBasePath());

		std::map <std::string, ConfigNode*> nodes = mDefinitionLoader->getAllConfigScripts();
		for (std::map <std::string, ConfigNode*>::const_iterator it = nodes.begin();
			it != nodes.end(); ++it)
		{
			if (!(it->second->getName() == "material_definition"))
				break;

			MaterialDefinition newDef;
			std::string name = it->first;

			std::vector<ConfigNode*> passes = it->second->getChildren();
			for (std::vector<ConfigNode*>::const_iterator passIt = passes.begin(); passIt != passes.end(); ++passIt)
			{
				PassDefinition* newPassDef = newDef.createPassDefinition();
				std::vector<ConfigNode*> props = (*passIt)->getChildren();
				for (std::vector<ConfigNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt )
				{
					std::string val = (*propIt)->getValue();
					newPassDef->setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
				}
			}

			mDefinitions[name] = newDef;
		}
	}

	Factory::~Factory ()
	{
		delete mDefinitionLoader;
		delete mPlatform;
	}

	void Factory::requestMaterial (const std::string& name)
	{
		/// \todo implement me
	}
}
