#include "DefinitionLoader.hpp"

namespace sh
{
	DefinitionLoader::DefinitionLoader (const std::string& path)
		: ConfigLoader(".definition")
	{
		ConfigLoader::loadAllFiles(this, path);
	}

	DefinitionLoader::~DefinitionLoader ()
	{
	}
}
