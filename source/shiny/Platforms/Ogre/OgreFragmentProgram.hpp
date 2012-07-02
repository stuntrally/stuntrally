#ifndef SH_OGREFRAGMENTPROGRAM_H
#define SH_OGREFRAGMENTPROGRAM_H

#include <string>

#include <OgreHighLevelGpuProgram.h>

#include "../../Main/Platform.hpp"

namespace sh
{
	class OgreFragmentProgram : public FragmentProgram
	{
	public:
		OgreFragmentProgram (
			const std::string& compileArguments,
			const std::string& name, const std::string& profile,
			const std::string& source, const std::string& lang,
			const std::string& resourceGroup);

		virtual bool getSupported();

	private:
		Ogre::HighLevelGpuProgramPtr mProgram;
	};
}

#endif
