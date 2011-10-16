#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

class MaterialFactory;

// some useful defines that will save code
#define MATGEN_CTOR_H(s) s(MaterialFactory* parent);
#define MATGEN_CTOR(s) s::s(MaterialFactory* parent) { MaterialGenerator(parent); }

class MaterialGenerator
{
public:
	MaterialGenerator* setParent(MaterialFactory* parent);
	
	virtual void generate() = 0;
	
protected:
	MaterialFactory* mParent;
};

#endif
