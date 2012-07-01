#include "ShaderInstance.hpp"

#include "Preprocessor.hpp"

#include <iostream> //temp

namespace sh
{
	ShaderInstance::ShaderInstance (const std::string& name, std::string source, const std::string& basePath, PropertySetGet* properties)
		: mName(name)
	{
		Preprocessor p;
		std::string newSource = p.preprocess(source, basePath, std::vector<std::string>());

	}

	std::string ShaderInstance::getName ()
	{
		return mName;
	}
}
