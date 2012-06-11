#ifndef SH_FACTORY_H
#define SH_FACTORY_H

#include <map>
#include <string>

#include "MaterialDefinition.hpp"
#include "MaterialInstance.hpp"

namespace sh
{
	class Platform;
	class DefinitionLoader;

	typedef std::map<std::string, MaterialDefinition> DefinitionMap;
	typedef std::map<std::string, MaterialInstance> InstanceMap;
	typedef std::map<std::string, InstanceMap> GroupMap;

	class Factory
	{
	public:
		Factory(Platform* platform);
		///< @note ownership of \a platform is transferred to this class, so you don't have to delete it

		~Factory();

		/**
		 * create a MaterialInstance based upon \a definition
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (MaterialDefinition* definition, const std::string& group = "");

		/**
		 * create a MaterialInstance, copying all properties from \a instance
		 * and using the same MaterialDefinition that \a instance uses
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (MaterialInstance* instance, const std::string& group = "");

		/**
		 * destroys all materials & their corresponding definitions that belong to \a group
		 * also destroys the group itself
		 */
		void destroyGroup (const std::string& group);

		/**
		 * get a MaterialInstance by name
		 */
		MaterialInstance* getInstance (const std::string& name);

		/**
		 * get a MaterialDefinition by name
		 */
		MaterialDefinition* getDefinition (const std::string& name);

		friend class Platform;

	private:
		DefinitionMap mDefinitions;
		GroupMap mGroups;

		Platform* mPlatform;

		DefinitionLoader* mDefinitionLoader;

	private:
		void requestMaterial (const std::string& name);
	};
};

#endif
