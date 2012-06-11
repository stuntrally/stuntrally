#include "Platform.hpp"

#include <stdexcept>

#include "Factory.hpp"

namespace sh
{
	Platform::Platform (const std::string& basePath)
		: mBasePath(basePath)
	{
	}

	Platform::~Platform ()
	{
	}

	void Platform::setFactory (Factory* factory)
	{
		mFactory = factory;
	}

	std::string Platform::getBasePath ()
	{
		return mBasePath;
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
