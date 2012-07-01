#ifndef SH_FACTORY_H
#define SH_FACTORY_H

#include <map>
#include <string>

#include "MaterialInstance.hpp"
#include "Configuration.hpp"
#include "ShaderSet.hpp"
#include "Language.hpp"

namespace sh
{
	class Platform;

	typedef std::map<std::string, MaterialInstance> MaterialMap;
	typedef std::map<std::string, ShaderSet> ShaderSetMap;
	typedef std::map<std::string, std::string> SettingsMap;

	/**
	 * @brief
	 * the main interface class
	 */
	class Factory
	{
	public:
		Factory(Platform* platform);
		///< @note ownership of \a platform is transferred to this class, so you don't have to delete it

		~Factory();

		/**
		 * create a MaterialInstance, copying all properties from \a instance
		 * @param name name of the new instance
		 * @param instance name of the parent
		 * @param createImmediately if true, the material is created immediately, otherwise it is created as soon as the renderer requests it
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (const std::string& name, const std::string& instance, bool createImmediately = false);

		/// safe to call if instance does not exist
		void destroyMaterialInstance (const std::string& name);

		/// use this to enable or disable shaders on-the-fly
		void setShadersEnabled (bool enabled);

		/// use this to manage user settings (for example this can be used to disable normal maps in your shaders) \n
		/// global settings can be retrieved in a shader through a macro \n
		/// when a global setting is changed, the shaders that depend on them are recompiled automatically
		void setGlobalSetting (const std::string& name, const std::string& value);

		/// adjusts the given shared parameter \n
		/// internally, this will change all uniform parameters of this name marked with @shSharedParameter \n
		/// @note they also apply to shaders that are created after the parameter was set, so the order does not matter
		void setSharedParameter (const std::string& name, PropertyValuePtr value);

		Language getCurrentLanguage ();

		/// switch between different shader languages (cg, glsl, hlsl)
		void setCurrentLanguage (Language lang);

		/// get a MaterialInstance by name
		//MaterialInstance* getInstance (const std::string& name);

		/// switch the active \a Configuration
		/// @param configuration name of the configuration to switch to
		void setActiveConfiguration (const std::string& configuration);

		/// register a \a Configuration, which can then be used for setActiveConfiguration method
		void registerConfiguration (const std::string& name, Configuration configuration);

		void notifyFrameEntered ();

		static Factory& getInstance();
		///< Return instance of this class.

		friend class Platform;
		friend class MaterialInstance;
		friend class ShaderInstance;

	private:
		static Factory* sThis;

		bool mShadersEnabled;

		MaterialMap mMaterials;
		ShaderSetMap mShaderSets;
		SettingsMap mGlobalSettings;

		Language mCurrentLanguage;

		Platform* mPlatform;

		Platform* getPlatform ();

		ShaderSet* getShaderSet (const std::string& name);

		MaterialInstance* requestMaterial (const std::string& name, const std::string& configuration);

		MaterialInstance* findInstance (const std::string& name);
		MaterialInstance* searchInstance (const std::string& name);
	};
};

#endif
