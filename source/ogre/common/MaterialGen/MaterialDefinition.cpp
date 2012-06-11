#include "MaterialDefinition.hpp"

namespace sh
{
	PassDefinition* MaterialDefinition::createPassDefinition()
	{
		PassDefinition newDef;
		mPasses.push_back(newDef);
		return &mPasses.back();
	}
}
