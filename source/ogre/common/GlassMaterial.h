#ifndef GLASSMATERIAL_H
#define GLASSMATERIAL_H

// Car glass material

#include "MaterialGenerator.h"

class GlassMaterialGenerator : public MaterialGenerator
{
public:
	GlassMaterialGenerator();

	std::string mName;

	//virtual void generate(bool fixedFunction=false);
};
 
#endif
