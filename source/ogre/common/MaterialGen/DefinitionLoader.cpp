#include "DefinitionLoader.hpp"

namespace sh
{
	DefinitionLoader::DefinitionLoader (const std::string& path)
		: ScriptLoader(".definition")
	{
		ScriptLoader::loadAllFiles(this, path);
	}

	DefinitionLoader::~DefinitionLoader ()
	{
	}
}
