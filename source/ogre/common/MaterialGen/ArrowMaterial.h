#ifndef ARROWMATERIAL_H
#define ARROWMATERIAL_H

// Checkpoint arrow material

#include "MaterialGenerator.h"

class ArrowMaterialGenerator : public MaterialGenerator
{
public:
	ArrowMaterialGenerator();

	virtual void generate();
	
protected:
	virtual Ogre::HighLevelGpuProgramPtr createVertexProgram();
	virtual Ogre::HighLevelGpuProgramPtr createFragmentProgram();
};
 
#endif
