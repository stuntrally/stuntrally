#include "Factory.hpp"

#include "Platform.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);
	}

	Factory::~Factory ()
	{
	}

	void Factory::requestMaterial (const std::string& name)
	{
		/// \todo implement me
	}
}
