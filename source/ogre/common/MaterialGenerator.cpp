#include "pch.h"
#include "MaterialGenerator.h"
#include "MaterialFactory.h"

using namespace Ogre;

MaterialGenerator* MaterialGenerator::setParent(MaterialFactory* parent)
{
	mParent = parent;
	return this;
}

MaterialPtr MaterialGenerator::prepareMaterial(const std::string& name)
{
}
