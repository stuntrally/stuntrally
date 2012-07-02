#include "OgreFragmentProgram.hpp"

#include <OgreHighLevelGpuProgramManager.h>

namespace sh
{
	OgreFragmentProgram::OgreFragmentProgram(
		const std::string& compileArguments,
		const std::string& name, const std::string& profile,
		const std::string& source, const std::string& lang,
		const std::string& resourceGroup)
		: FragmentProgram()
	{
		Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();
		assert (mgr.getByName(name).isNull() && "Fragment program already exists");

		mProgram = mgr.createProgram(name, resourceGroup, lang, Ogre::GPT_FRAGMENT_PROGRAM);
		if (lang != "glsl")
			mProgram->setParameter("entry_point", "main");

		if (lang == "hlsl")
			mProgram->setParameter("target", profile);
		else if (lang == "cg")
			mProgram->setParameter("profiles", profile);

		mProgram->setSource(source);
		mProgram->load();

		if (mProgram.isNull() || !mProgram->isSupported())
			std::cerr << "Failed to compile fragment shader \"" << name << "\". Consider the OGRE log for more information." << std::endl;
	}

	bool OgreFragmentProgram::getSupported()
	{
		return (!mProgram.isNull() && mProgram->isSupported());
	}
}
