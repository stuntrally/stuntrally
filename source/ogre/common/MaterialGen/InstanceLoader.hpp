#ifndef SH_INSTANCELOADER_H
#define SH_INSTANCELOADER_H

#include "ScriptLoader.hpp"

namespace sh
{
	/**
	 * @brief
	 * loads .mat files that contain a serialized version of \a MaterialInstance objects
	 */
	class InstanceLoader : public ScriptLoader
	{
	public:
		InstanceLoader(const std::string& path);
		///< constructor that loads all files from \a path (works recursively)

		virtual ~InstanceLoader();
	};
}

#endif
