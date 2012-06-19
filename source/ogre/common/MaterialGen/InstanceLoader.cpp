#include "InstanceLoader.hpp"

namespace sh
{
	InstanceLoader::InstanceLoader (const std::string& path)
		: ScriptLoader(".instance")
	{
		ScriptLoader::loadAllFiles(this, path);
	}

	InstanceLoader::~InstanceLoader ()
	{
	}
}
