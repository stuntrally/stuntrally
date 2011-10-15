#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

class MaterialFactory;

class MaterialGenerator
{
public:
	MaterialGenerator();
	~MaterialGenerator();
	
	void setParent(MaterialFactory* parent);
	
protected:
	MaterialFactory* mParent;
};

#endif
