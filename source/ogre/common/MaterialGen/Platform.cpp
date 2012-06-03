#include "Platform.hpp"

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

	void Platform::fireMaterialRequested (const std::string& name)
	{
		mFactory->requestMaterial (name);
	}
}
