#ifndef GLASSMATERIAL_H
#define GLASSMATERIAL_H

// Car glass material

#include "MaterialGenerator.h"

class GlassMaterialGenerator : public MaterialGenerator
{
public:
	GlassMaterialGenerator();

	virtual void generate();
	
protected:
	virtual Ogre::HighLevelGpuProgramPtr createAmbientFragmentProgram(); // ambient pass fragment program
	virtual Ogre::HighLevelGpuProgramPtr createAmbientVertexProgram(); // ambient pass vertex program
};
 
#endif
