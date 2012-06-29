#include "InstanceLoader.hpp"

namespace sh
{
	InstanceLoader::InstanceLoader (const std::string& path)
		: ScriptLoader(".mat")
	{
		ScriptLoader::loadAllFiles(this, path);
	}

	InstanceLoader::~InstanceLoader ()
	{
	}
}
