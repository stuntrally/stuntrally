#ifndef SH_QUERY_H
#define SH_QUERY_H

#include <string>
#include <map>
#include <vector>

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



struct TextureUnitInfo
{
	std::map<std::string, std::string> mProperties;
};

struct PassInfo
{
	std::map<std::string, std::string> mShaderProperties;

	std::map<std::string, std::string> mProperties;
	std::vector<TextureUnitInfo> mTextureUnits;
};

struct MaterialProperty
{

	enum Type
	{
		Texture,
		Boolean,
		Misc
	};

	MaterialProperty() {}
	MaterialProperty (const std::string& value, Type type, bool inherited)
		: mValue(value), mType(type), mInherited(inherited) {}

	std::string mValue;
	Type mType;
	bool mInherited;
};

class MaterialQuery : public Query
{
public:
	MaterialQuery(const std::string& name)
		: mName(name) {}

	std::string mParent;
	std::vector<PassInfo> mPasses;
	std::map<std::string, MaterialProperty> mProperties;

protected:
	std::string mName;
	virtual void executeImpl();
};

}

#endif
