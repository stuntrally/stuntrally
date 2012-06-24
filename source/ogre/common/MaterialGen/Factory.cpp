#include "Factory.hpp"

#include <stdexcept>

#include "Platform.hpp"
#include "DefinitionLoader.hpp"
#include "InstanceLoader.hpp"
#include "ShaderSetLoader.hpp"
#include "ShaderSet.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);

		// load shader sets
		{
			ShaderSetLoader shaderSetLoader(mPlatform->getBasePath());
			std::map <std::string, ScriptNode*> nodes = shaderSetLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "shader_set"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .shaderset" << std::endl;
					break;
				}

				ShaderSet newSet (it->second->findChild("type")->getValue(), it->second->findChild("source")->getValue());

				mShaderSets[it->first] = newSet;
			}
		}

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

				MaterialInstance newInstance(it->first);
				newInstance._create(mPlatform);

				std::string group;

				std::vector<ScriptNode*> props = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
				{
					std::string name = (*propIt)->getName();
					std::string val = (*propIt)->getValue();

					if (name == "parent")
						newInstance._setParentInstance(val);
					else if (name == "group")
						group = val;
					else
						newInstance.setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
				}

				if (mGroups.find(group) == mGroups.end())
				{
					Group newGroup;
					newGroup.mInstances[it->first] = newInstance;
					mGroups[group] = newGroup;
				}
				else
					mGroups[group].mInstances[it->first] = newInstance;
			}

			// now that all instances are loaded, replace the parent names with the actual pointers to parent
			for (GroupMap::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
			{
				for (std::map<std::string, MaterialInstance>::iterator instanceIt = it->second.mInstances.begin(); instanceIt != it->second.mInstances.end(); ++instanceIt)
				{
					if (instanceIt->second._getParentInstance() != "")
					{
						std::string p = instanceIt->second._getParentInstance();
						if (it->second.mInstances.find (p) == it->second.mInstances.end())
							throw std::runtime_error ("Unable to find parent for material instance \"" + instanceIt->first + "\". Make sure it belongs to the same group.");
						else
							instanceIt->second.setParent(&it->second.mInstances[p]);
					}

					std::string definition = PropertyValue::retrieve<StringValue>(instanceIt->second.getProperty("definition"))->get();
					instanceIt->second._setDefinition (&mDefinitions[definition]);
				}
			}
		}
	}

	Factory::~Factory ()
	{
		delete mPlatform;
	}

	MaterialInstance* Factory::searchInstance (const std::string& name)
	{
		for (GroupMap::iterator it = mGroups.begin(); it != mGroups.end(); ++it)
		{
			if (it->second.mInstances.find (name) != it->second.mInstances.end())
			{
				return &it->second.mInstances[name];
			}
		}
		return NULL;
	}

	MaterialInstance* Factory::findInstance (const std::string& name)
	{
		MaterialInstance* m = searchInstance (name);
		if (m)
			return m;
		else
			throw std::runtime_error ("MaterialInstance with name \"" + name + "\" not found");
	}

	MaterialInstance* Factory::requestMaterial (const std::string& name, const std::string& configuration)
	{
		MaterialInstance* m = searchInstance (name);
		if (m)
			m->_createForConfiguration (mPlatform, configuration);
		return m;
	}

	void Factory::notifyFrameEntered ()
	{
		mPlatform->notifyFrameEntered();
	}
}
