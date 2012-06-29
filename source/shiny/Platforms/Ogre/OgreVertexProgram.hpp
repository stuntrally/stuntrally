#ifndef SH_OGREVERTEXPROGRAM_H
#define SH_OGREVERTEXPROGRAM_H

#include <string>

#include <OgreHighLevelGpuProgram.h>

#include "../../Main/Platform.hpp"

namespace sh
{
	class OgreVertexProgram : public VertexProgram
	{
	public:
		OgreVertexProgram (
			const std::string& compileArguments,
			const std::string& name, const std::string& entryPoint,
			const std::string& source, const std::string& lang,
			const std::string& resourceGroup);

	private:
		Ogre::HighLevelGpuProgramPtr mProgram;
	};
}

#endif
