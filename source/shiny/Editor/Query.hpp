#ifndef SH_QUERY_H
#define SH_QUERY_H

#include <string>
#include <map>

namespace sh
{

class Query
{
public:
	Query()
		: mDone(false) {}

	void execute();

	bool mDone;

protected:
	virtual void executeImpl() = 0;
};

class ConfigurationQuery : public Query
{
public:
	ConfigurationQuery(const std::string& name);

	std::map<std::string, std::string> mProperties;
protected:
	std::string mName;
	virtual void executeImpl();
};

}

#endif
