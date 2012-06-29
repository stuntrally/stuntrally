#ifndef SH_OGREGEOMETRYPROGRAM_H
#define SH_OGREGEOMETRYPROGRAM_H

#include <string>

#include <OgreHighLevelGpuProgram.h>

#include "../../Main/Platform.hpp"

namespace sh
{
	class OgreGeometryProgram : public GeometryProgram
	{
	public:
		OgreGeometryProgram  (
			const std::string& compileArguments,
			const std::string& name, const std::string& entryPoint,
			const std::string& source, const std::string& lang,
			const std::string& resourceGroup);

	private:
		Ogre::HighLevelGpuProgramPtr mProgram;
	};
}

#endif
