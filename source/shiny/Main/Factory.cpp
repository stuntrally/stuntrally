#include "Factory.hpp"

#include <stdexcept>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "Platform.hpp"
#include "InstanceLoader.hpp"
#include "ShaderSetLoader.hpp"
#include "ShaderSet.hpp"
#include "MaterialInstanceTextureUnit.hpp"

namespace sh
{
	Factory* Factory::sThis = 0;

	Factory& Factory::getInstance()
	{
		assert (sThis);
		return *sThis;
	}

	Factory::Factory (Platform* platform)
		: mPlatform(platform)
		, mShadersEnabled(true)
		, mCurrentLanguage(Language_CG)
	{
		assert (!sThis);
		sThis = this;

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

				if (!it->second->findChild("profiles_cg"))
					throw std::runtime_error ("missing \"profiles_cg\" field for \"" + it->first + "\"");
				if (!it->second->findChild("profiles_hlsl"))
					throw std::runtime_error ("missing \"profiles_hlsl\" field for \"" + it->first + "\"");
				if (!it->second->findChild("source"))
					throw std::runtime_error ("missing \"source\" field for \"" + it->first + "\"");
				if (!it->second->findChild("type"))
					throw std::runtime_error ("missing \"type\" field for \"" + it->first + "\"");

				std::vector<std::string> profiles_cg;
				boost::split (profiles_cg, it->second->findChild("profiles_cg")->getValue(), boost::is_any_of(" "));
				std::string cg_profile;
				for (std::vector<std::string>::iterator it2 = profiles_cg.begin(); it2 != profiles_cg.end(); ++it2)
				{
					if (mPlatform->isProfileSupported(*it2))
					{
						cg_profile = *it2;
						break;
					}
				}

				std::vector<std::string> profiles_hlsl;
				boost::split (profiles_hlsl, it->second->findChild("profiles_hlsl")->getValue(), boost::is_any_of(" "));
				std::string hlsl_profile;
				for (std::vector<std::string>::iterator it2 = profiles_hlsl.begin(); it2 != profiles_hlsl.end(); ++it2)
				{
					if (mPlatform->isProfileSupported(*it2))
					{
						hlsl_profile = *it2;
						break;
					}
				}

				ShaderSet newSet (it->second->findChild("type")->getValue(), cg_profile, hlsl_profile,
								  mPlatform->getBasePath() + "/" + it->second->findChild("source")->getValue(),
								  mPlatform->getBasePath(),
								  it->first,
								  &mGlobalSettings);

				mShaderSets.insert(std::make_pair(it->first, newSet));
			}
		}

		// load instances
		{
			InstanceLoader instanceLoader(mPlatform->getBasePath());

			std::map <std::string, ScriptNode*> nodes = instanceLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "material"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .mat" << std::endl;
					break;
				}

				MaterialInstance newInstance(it->first, this);
				newInstance.create(mPlatform);

				std::vector<ScriptNode*> props = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
				{
					std::string name = (*propIt)->getName();

					std::string val = (*propIt)->getValue();

					if (name == "pass")
					{
						MaterialInstancePass* newPass = newInstance.createPass();
						std::vector<ScriptNode*> props2 = (*propIt)->getChildren();
						for (std::vector<ScriptNode*>::const_iterator propIt2 = props2.begin(); propIt2 != props2.end(); ++propIt2)
						{
							std::string name2 = (*propIt2)->getName();
							std::string val2 = (*propIt2)->getValue();

							if (name2 == "texture_unit")
							{
								MaterialInstanceTextureUnit* newTex = newPass->createTextureUnit(val2);
								std::vector<ScriptNode*> texProps = (*propIt2)->getChildren();
								for (std::vector<ScriptNode*>::const_iterator texPropIt = texProps.begin(); texPropIt != texProps.end(); ++texPropIt)
								{
									std::string val = (*texPropIt)->getValue();
									newTex->setProperty((*texPropIt)->getName(), makeProperty(val));
								}
							}
							else
								newPass->setProperty((*propIt2)->getName(), makeProperty(val2));
						}
					}
					else if (name == "parent")
						newInstance.setParentInstance(val);
					else
						newInstance.setProperty((*propIt)->getName(), makeProperty(val));
				}

				mMaterials.insert (std::make_pair(it->first, newInstance));
			}

			// now that all instances are loaded, replace the parent names with the actual pointers to parent
			for (MaterialMap::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
			{
				std::string parent = it->second.getParentInstance();
				if (parent != "")
				{
					if (mMaterials.find (it->second.getParentInstance()) == mMaterials.end())
						throw std::runtime_error ("Unable to find parent for material instance \"" + it->first + "\"");
					it->second.setParent(&mMaterials.find(parent)->second);
				}
			}
		}
	}

	Factory::~Factory ()
	{
		delete mPlatform;
		sThis = 0;
	}

	MaterialInstance* Factory::searchInstance (const std::string& name)
	{
		if (mMaterials.find(name) != mMaterials.end())
				return &mMaterials.find(name)->second;

		return NULL;
	}

	MaterialInstance* Factory::findInstance (const std::string& name)
	{
		assert (mMaterials.find(name) != mMaterials.end());
		return &mMaterials.find(name)->second;
	}

	MaterialInstance* Factory::requestMaterial (const std::string& name, const std::string& configuration)
	{
		MaterialInstance* m = searchInstance (name);
		if (m)
			m->createForConfiguration (mPlatform, configuration);
		return m;
	}

	void Factory::notifyFrameEntered ()
	{
		mPlatform->notifyFrameEntered();
	}

	MaterialInstance* Factory::createMaterialInstance (const std::string& name, const std::string& instance, bool createImmediately)
	{
		if (mMaterials.find(instance) == mMaterials.end())
			throw std::runtime_error ("trying to clone material that does not exist");
		MaterialInstance newInstance(name, this);
		if (!mShadersEnabled)
			newInstance.setShadersEnabled(false);
		newInstance.setParent (&mMaterials.find(instance)->second);
		newInstance.create(mPlatform);
		if (createImmediately)
			newInstance.createForConfiguration(mPlatform, "Default"); /// \todo create all configurations
		mMaterials.insert (std::make_pair(name, newInstance));
		return &mMaterials.find(name)->second;
	}

	void Factory::destroyMaterialInstance (const std::string& name)
	{
		if (mMaterials.find(name) != mMaterials.end())
			mMaterials.erase(name);
	}

	void Factory::setShadersEnabled (bool enabled)
	{
		mShadersEnabled = enabled;
		for (MaterialMap::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
		{
			it->second.setShadersEnabled(enabled);
		}
	}

	void Factory::setGlobalSetting (const std::string& name, const std::string& value)
	{
		/*
		bool changed = true;
		if (mGlobalSettings.find(name) != mGlobalSettings.end())
			changed = (mGlobalSettings[name] == value);
		*/
		mGlobalSettings[name] = value;
	}

	void Factory::setSharedParameter (const std::string& name, PropertyValuePtr value)
	{
		mPlatform->setSharedParameter(name, value);
	}

	ShaderSet* Factory::getShaderSet (const std::string& name)
	{
		return &mShaderSets.find(name)->second;
	}

	Platform* Factory::getPlatform ()
	{
		return mPlatform;
	}

	Language Factory::getCurrentLanguage ()
	{
		return mCurrentLanguage;
	}

	void Factory::setCurrentLanguage (Language lang)
	{
		mCurrentLanguage = lang;
		/// \todo
	}
}
