#ifndef _FluidsXml_h_
#define _FluidsXml_h_

#include <string>
#include <vector>
#include <map>


//  Fluid parameters  (water, mud, grease types)

class FluidParams
{
public:
	std::string name, material;

	//  car buoyancy
	float density, angularDrag, linearDrag, heightVelRes;  // damping, motion resistance

	//  wheel
	bool bWhForce;
	float whMaxAngVel, whSpinDamp, whForceLong, whForceUp, whSteerMul;
	float bumpFqX, bumpFqY, bumpAmp, bumpAng;
	
	FluidParams();
};


class FluidsXml
{
public:
	//  all fluids
	std::vector<FluidParams> fls;

	//  maps name to fls index, at track load
	std::map<std::string, int> flMap;  // 0 if not found
	
	//  methods
	//FluidsXml();  void Default();
	bool LoadXml(std::string file);
};

#endif
