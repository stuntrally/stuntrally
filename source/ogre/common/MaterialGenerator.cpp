#include "MaterialGenerator.h"
#include "MaterialFactory.h"

MaterialGenerator* MaterialGenerator::setParent(MaterialFactory* parent)
{
	mParent = parent;
	return this;
}
