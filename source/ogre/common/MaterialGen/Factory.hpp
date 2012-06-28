#ifndef SH_FACTORY_H
#define SH_FACTORY_H

#include <map>
#include <string>

#include "MaterialInstance.hpp"
#include "Configuration.hpp"
#include "ShaderSet.hpp"

namespace sh
{
	class Platform;

	typedef std::map<std::string, MaterialInstance> MaterialMap;
	typedef std::map<std::string, ShaderSet> ShaderSetMap;

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
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (const std::string& name, const std::string& instance);

		void destroyMaterialInstance (const std::string& name);

		/**
		 * get a MaterialInstance by name
		 */
		MaterialInstance* getInstance (const std::string& name);

		/**
		 * switch the active \a Configuration
		 * @param configuration name of the configuration to switch to
		 */
		void setActiveConfiguration (const std::string& configuration);

		/**
		 * register a \a Configuration, which can then be used for setActiveConfiguration method
		 */
		void registerConfiguration (const std::string& name, Configuration configuration);

		void notifyFrameEntered ();

		friend class Platform;

	private:
		MaterialMap mMaterials;
		ShaderSetMap mShaderSets;

		Platform* mPlatform;

		MaterialInstance* requestMaterial (const std::string& name, const std::string& configuration);

		MaterialInstance* findInstance (const std::string& name);
		MaterialInstance* searchInstance (const std::string& name);
	};
};

#endif
