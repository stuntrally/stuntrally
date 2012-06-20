#ifndef SH_FACTORY_H
#define SH_FACTORY_H

#include <map>
#include <string>

#include "MaterialDefinition.hpp"
#include "MaterialInstance.hpp"
#include "Configuration.hpp"
#include "Group.hpp"

namespace sh
{
	class Platform;

	typedef std::map<std::string, MaterialDefinition> DefinitionMap;
	typedef std::map<std::string, Group> GroupMap;

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
		 * create a MaterialInstance based upon \a definition
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (const std::string& definition, const std::string& group = "");

		/**
		 * create a MaterialInstance, copying all properties from \a instance
		 * and using the same MaterialDefinition that \a instance uses
		 * @return newly created instance
		 */
		MaterialInstance* createMaterialInstance (MaterialInstance* instance, const std::string& group = "");

		/**
		 * destroys all materials that belong to \a group \n
		 * also destroys the group itself
		 */
		void destroyGroup (const std::string& group);

		/**
		 * get a MaterialInstance by name
		 */
		MaterialInstance* getInstance (const std::string& name);

		/**
		 * switch the active \a Configuration of either a specific group, or all groups if the \a group parameter
		 * is left empty
		 * @param configuration name of the configuration to switch to
		 * @param group to apply this configuration to
		 */
		void setActiveConfiguration (const std::string& configuration, const std::string& group = "");

		/**
		 * register a \a Configuration, which can then be used for setActiveConfiguration method
		 */
		void registerConfiguration (const std::string& name, Configuration configuration);

		friend class Platform;

	private:
		DefinitionMap mDefinitions;
		GroupMap mGroups;

		Platform* mPlatform;

		void requestMaterial (const std::string& name);
	};
};

#endif
