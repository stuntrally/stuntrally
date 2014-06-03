#pragma once
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
	int idParticles;  // index for wheel particles (0water, 1mud soft, 2mud hard, -1 none)
	float fDamage;  // damage per sec, per wheel (div by 4)
	
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
