#include "Query.hpp"

#include "../Main/Factory.hpp"

namespace sh
{

void Query::execute()
{
	executeImpl();
	mDone = true;
}

ConfigurationQuery::ConfigurationQuery(const std::string &name)
	: mName(name)
{
}

void ConfigurationQuery::executeImpl()
{
	sh::Factory::getInstance().listConfigurationSettings(mName, mProperties);
}

}
