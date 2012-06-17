#include "Factory.hpp"

#include "Platform.hpp"
#include "DefinitionLoader.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);


		// load definitions
		mDefinitionLoader = new DefinitionLoader(mPlatform->getBasePath());

		std::map <std::string, ScriptNode*> nodes = mDefinitionLoader->getAllConfigScripts();
		for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
			it != nodes.end(); ++it)
		{
			if (!(it->second->getName() == "material_definition"))
				break;

			MaterialDefinition newDef;
			std::string name = it->first;

			std::vector<ScriptNode*> passes = it->second->getChildren();
			for (std::vector<ScriptNode*>::const_iterator passIt = passes.begin(); passIt != passes.end(); ++passIt)
			{
				PassDefinition* newPassDef = newDef.createPassDefinition();
				std::vector<ScriptNode*> props = (*passIt)->getChildren();
				for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
				{
					std::string name = (*propIt)->getName();
					std::string val = (*propIt)->getValue();

					if (name == "texture_unit")
					{
						TextureUnitStateDefinition* newTexDef = newPassDef->createTextureUnitStateDefinition();
						newTexDef->setName(val);
						std::vector<ScriptNode*> texProps = (*propIt)->getChildren();
						for (std::vector<ScriptNode*>::const_iterator texPropIt = texProps.begin(); texPropIt != texProps.end(); ++texPropIt)
						{
							std::string val = (*texPropIt)->getValue();
							newTexDef->setProperty((*texPropIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
						}
					}
					else
					{
						newPassDef->setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
					}
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
