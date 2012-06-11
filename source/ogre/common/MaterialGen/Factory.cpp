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
			MaterialDefinition newDef;
			std::cout << "node here " << std::endl;

			std::vector<ConfigNode*> passes = it->second->getChildren();
			std::cout << "passes size " << passes.size() << std::endl;
			for (std::vector<ConfigNode*>::const_iterator passIt = passes.begin(); passIt != passes.end(); ++passIt)
			{
				PassDefinition* newPassDef = newDef.createPassDefinition();
				std::cout << "created pass " << std::endl;
			}
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
