#include "ShaderSetLoader.hpp"

namespace sh
{
	ShaderSetLoader::ShaderSetLoader (const std::string& path)
		: ScriptLoader(".shaderset")
	{
		ScriptLoader::loadAllFiles(this, path);
	}

	ShaderSetLoader::~ShaderSetLoader ()
	{
	}
}
