#include "OgreVertexProgram.hpp"

#include <OgreHighLevelGpuProgramManager.h>

namespace sh
{
	OgreVertexProgram::OgreVertexProgram(
		const std::string& compileArguments,
		const std::string& name, const std::string& profile,
		const std::string& source, const std::string& lang,
		const std::string& resourceGroup)
		: VertexProgram()
	{
		Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();
		assert (mgr.getByName(name).isNull() && "Vertex program already exists");

		mProgram = mgr.createProgram(name, resourceGroup, lang, Ogre::GPT_VERTEX_PROGRAM);
		if (lang != "glsl")
			mProgram->setParameter("entry_point", "main");

		if (lang == "hlsl")
			mProgram->setParameter("target", profile);
		else if (lang == "cg")
			mProgram->setParameter("profiles", profile);

		mProgram->setSource(source);
		mProgram->load();

		if (mProgram.isNull() || !mProgram->isSupported())
			std::cerr << "Failed to compile vertex shader \"" << name << "\". Consider the OGRE log for more information." << std::endl;
	}

	bool OgreVertexProgram::getSupported()
	{
		return (!mProgram.isNull() && mProgram->isSupported());
	}

}
