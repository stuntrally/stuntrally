#ifndef PIPEGLASSMATERIAL_H
#define PIPEGLASSMATERIAL_H

// Pipe glass material

#include "MaterialGenerator.h"

class PipeGlassMaterialGenerator : public MaterialGenerator
{
public:
	PipeGlassMaterialGenerator();

	virtual void generate(bool fixedFunction=false);
	
protected:
	virtual Ogre::HighLevelGpuProgramPtr createPipeFragmentProgram(); // ambient pass fragment program
	virtual Ogre::HighLevelGpuProgramPtr createPipeVertexProgram(); // ambient pass vertex program
};
 
#endif
