#ifndef SH_SHADERSETLOADER_H
#define SH_SHADERSETLOADER_H

#include "ScriptLoader.hpp"

namespace sh
{
	/**
	 * @brief
	 * loads .shaderset files that contain a serialized version of \a ShaderSet objects
	 */
	class ShaderSetLoader : public ScriptLoader
	{
	public:
		ShaderSetLoader(const std::string& path);
		///< constructor that loads all files from \a path (works recursively)

		virtual ~ShaderSetLoader();
	};
}

#endif
