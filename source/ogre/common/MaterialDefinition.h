#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

class MaterialFactory;  struct MaterialProperties;

class MaterialDefinition
{
public:
	MaterialDefinition(MaterialFactory* parent, MaterialProperties* props);
	~MaterialDefinition();
	
	/// get/set
	const std::string& getName() { return mName; };
	MaterialProperties* getProps();
	void setName(const std::string& name) { mName = name; };
	
	MaterialProperties* mProps;

private:
	MaterialFactory* mParent;
		
	std::string mName; // name of generated material
};

#endif
