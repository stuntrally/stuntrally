#include "MaterialInstancePass.hpp"

namespace sh
{

	MaterialInstanceTextureUnit* MaterialInstancePass::createTextureUnit (const std::string& name)
	{
		mTexUnits.insert(std::make_pair(name, MaterialInstanceTextureUnit(name)));
		return &mTexUnits.find(name)->second;
	}

	std::map <std::string, MaterialInstanceTextureUnit> MaterialInstancePass::getTexUnits ()
	{
		return mTexUnits;
	}
}
