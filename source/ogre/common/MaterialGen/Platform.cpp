#include "Platform.hpp"

#include <stdexcept>

#include "Factory.hpp"

namespace sh
{
	Platform::Platform ()
	{
	}

	Platform::~Platform ()
	{
	}

	void Platform::setFactory (Factory* factory)
	{
		mFactory = factory;
	}

	bool Platform::supportsMaterialQueuedListener ()
	{
		return false;
	}

	bool Platform::supportsShaderSerialization ()
	{
		return false;
	}

	void Platform::fireMaterialRequested (const std::string& name)
	{
		mFactory->requestMaterial (name);
	}

	void Platform::serializeShaders (const std::string& file)
	{
		throw std::runtime_error ("Shader serialization not supported by this platform");
	}

	void Platform::deserializeShaders (const std::string& file)
	{
		throw std::runtime_error ("Shader serialization not supported by this platform");
	}
}
