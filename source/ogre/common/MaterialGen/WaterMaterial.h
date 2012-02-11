#ifndef WATERMATERIAL_H
#define WATERMATERIAL_H

// Water material

#include "MaterialGenerator.h"

class WaterMaterialGenerator : public MaterialGenerator
{
public:
	WaterMaterialGenerator();

	virtual void generate();
	
protected:
	// vertex program
	virtual Ogre::HighLevelGpuProgramPtr createVertexProgram();
	virtual void generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void vertexProgramParams(Ogre::HighLevelGpuProgramPtr program);
	virtual void individualVertexProgramParams(Ogre::GpuProgramParametersSharedPtr params);
	virtual void individualParamsAlways(Ogre::GpuProgramParametersSharedPtr params);
			
	// fragment program
	virtual Ogre::HighLevelGpuProgramPtr createFragmentProgram();
	virtual void generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void fragmentProgramParams(Ogre::HighLevelGpuProgramPtr program);
	virtual void individualFragmentProgramParams(Ogre::GpuProgramParametersSharedPtr params);

};
 
#endif

