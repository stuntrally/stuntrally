#include "OgreVertexProgram.hpp"

#include <OgreHighLevelGpuProgramManager.h>

namespace sh
{
	OgreVertexProgram::OgreVertexProgram(
		const std::string& name, const std::string& entryPoint,
		const std::string& source, const std::string& lang,
		const std::string& resourceGroup)
		: VertexProgram()
	{
		Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();
		assert (mgr.getByName(name).isNull() && "Vertex program already exists");

		Ogre::HighLevelGpuProgramPtr program;
		program = mgr.createProgram(name, resourceGroup, lang, Ogre::GPT_VERTEX_PROGRAM);
		program->setParameter("entry_point", entryPoint);
		program->setSource(source);
		program->load();
	}
}
