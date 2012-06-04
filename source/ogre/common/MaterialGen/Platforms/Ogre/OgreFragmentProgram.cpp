#include "OgreFragmentProgram.hpp"

#include <OgreHighLevelGpuProgramManager.h>

namespace sh
{
	OgreFragmentProgram::OgreFragmentProgram(
		const std::string& name, const std::string& entryPoint,
		const std::string& source, const std::string& lang,
		const std::string& resourceGroup)
		: FragmentProgram()
	{
		Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();
		assert (mgr.getByName(name).isNull() && "Fragment program already exists");

		Ogre::HighLevelGpuProgramPtr program;
		program = mgr.createProgram(name, resourceGroup, lang, Ogre::GPT_FRAGMENT_PROGRAM);
		program->setParameter("entry_point", entryPoint);
		program->setSource(source);
		program->load();
	}
}
