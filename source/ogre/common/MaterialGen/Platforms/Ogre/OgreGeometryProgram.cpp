#include "OgreGeometryProgram.hpp"

#include <OgreHighLevelGpuProgramManager.h>

namespace sh
{
	OgreGeometryProgram::OgreGeometryProgram(
		const std::string& compileArguments,
		const std::string& name, const std::string& entryPoint,
		const std::string& source, const std::string& lang,
		const std::string& resourceGroup)
		: GeometryProgram()
	{
		Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();
		assert (mgr.getByName(name).isNull() && "Geometry program already exists");

		Ogre::HighLevelGpuProgramPtr program;
		program = mgr.createProgram(name, resourceGroup, lang, Ogre::GPT_GEOMETRY_PROGRAM);
		program->setParameter("entry_point", entryPoint);
		program->setSource(source);
		program->load();
	}
}
