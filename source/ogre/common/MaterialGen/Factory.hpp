#ifndef SH_FACTORY_H
#define SH_FACTORY_H

#include <map>
#include <string>

#include "MaterialDefinition.hpp"
#include "MaterialInstance.hpp"

namespace sh
{
	class Platform;
	class ConfigLoader;

	typedef std::map<std::string, MaterialDefinition> DefinitionMap;
	typedef std::map<std::string, MaterialInstance> InstanceMap;
	typedef std::map<std::string, InstanceMap> GroupMap;

	class Factory
	{
	public:
		Factory(Platform* platform);
		~Factory();

		/**
		 * create a MaterialInstance based upon \a definition
		 */
		void createMaterialInstance (MaterialDefinition* definition, const std::string group = "");

		/**
		 * create a MaterialInstance, copying all properties from \a instance
		 * and using the same MaterialDefinition that \a instance uses
		 */
		void createMaterialInstance (MaterialInstance* instance, const std::string group = "");

		friend class Platform;

	private:
		DefinitionMap mDefinitions;
		GroupMap mGroups;

		Platform* mPlatform;

	private:
		void requestMaterial (const std::string& name);
	};
};

#endif
