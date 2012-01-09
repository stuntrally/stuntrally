#ifndef PARTICLEMATERIAL_H
#define PARTICLEMATERIAL_H

// particle material

#include "MaterialGenerator.h"

class ParticleMaterialGenerator : public MaterialGenerator
{
public:
	ParticleMaterialGenerator();

	virtual void generate(bool fixedFunction=false);
	
protected:
	virtual Ogre::HighLevelGpuProgramPtr createSoftParticleFragmentProgram(); // ambient pass fragment program
	virtual Ogre::HighLevelGpuProgramPtr createSoftParticleVertexProgram(); // ambient pass vertex program
};
 
#endif
