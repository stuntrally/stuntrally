#include "ShaderSet.hpp"

namespace sh
{
	ShaderSet::ShaderSet(Type type, const std::string& sourceFile)
		: mType(type)
		, mSourceFile(sourceFile)
	{
	}

	ShaderSet::ShaderSet()
	{
	}

	ShaderSet::ShaderSet (const std::string& type, const std::string& sourceFile)
		: mSourceFile(sourceFile)
	{
		if (type == "vertex")
			mType = Type_Vertex;
		else // if (type == "fragment")
			mType = Type_Fragment;
	}
}
