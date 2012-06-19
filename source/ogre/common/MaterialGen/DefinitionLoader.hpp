#ifndef SH_DEFINITIONLOADER_H
#define SH_DEFINITIONLOADER_H

#include "ScriptLoader.hpp"

namespace sh
{
	/**
	 * @brief
	 * loads .definition files that contain a serialized version of \a MaterialDefinition objects
	 */
	class DefinitionLoader : public ScriptLoader
	{
	public:
		DefinitionLoader(const std::string& path);
		///< constructor that loads all files from \a path (works recursively)

		virtual ~DefinitionLoader();
	};
}

#endif
