#include "Factory.hpp"

#include "Platform.hpp"
#include "DefinitionLoader.hpp"
#include "InstanceLoader.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);

		// load definitions
		{
			DefinitionLoader definitionLoader(mPlatform->getBasePath());

			std::map <std::string, ScriptNode*> nodes = definitionLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "material_definition"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .definition" << std::endl;
					break;
				}

				MaterialDefinition newDef;

				std::vector<ScriptNode*> passes = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator passIt = passes.begin(); passIt != passes.end(); ++passIt)
				{
					if ((*passIt)->getName() != "pass")
					{
						std::cerr << "sh::Factory: Warning: Unsupported node type \"" << (*passIt)->getName() << "\" for parent node of type \"material_definition\"" << std::endl;
						break;
					}
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
							newPassDef->setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
					}
				}

				mDefinitions[it->first] = newDef;
			}
		}

		// load instances
		{
			InstanceLoader instanceLoader(mPlatform->getBasePath());

			std::map <std::string, ScriptNode*> nodes = instanceLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "material_instance"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .instance" << std::endl;
					break;
				}

				MaterialInstance newInstance;

				std::string group;

				std::vector<ScriptNode*> props = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
				{
					std::string name = (*propIt)->getName();
					std::string val = (*propIt)->getValue();

					if (name == "parent")
						newInstance._setParentInstance(val);
					else
						newInstance.setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
				}
			}

			// now that all instances are loaded, replace the parent name with the actual pointer to this parent
		}
	}

	Factory::~Factory ()
	{
		delete mPlatform;
	}

	void Factory::requestMaterial (const std::string& name)
	{
		/// \todo implement me
	}
}
