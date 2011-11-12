#ifndef PIPEGLASSMATERIAL_H
#define PIPEGLASSMATERIAL_H

// Pipe glass material

#include "MaterialGenerator.h"
#include <OgreGpuProgramParams.h>

class PipeGlassMaterialGenerator : public MaterialGenerator
{
public:
	PipeGlassMaterialGenerator();

	virtual void generate(bool fixedFunction=false);
	
protected:
	virtual Ogre::HighLevelGpuProgramPtr createPipeFragmentProgram();
	virtual Ogre::HighLevelGpuProgramPtr createPipeVertexProgram();
	
	virtual void individualFragmentProgramParams(Ogre::GpuProgramParametersSharedPtr params);
};
 
#endif
