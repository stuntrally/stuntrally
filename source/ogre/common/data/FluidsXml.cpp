#include "pch.h"
#include "../Def_Str.h"
#include "FluidsXml.h"
#include "tinyxml2.h"
using namespace tinyxml2;


FluidParams::FluidParams()
	:density(200.f)  //, name(""), material("water_none")
	,angularDrag(0.7f), linearDrag(0.2f), heightVelRes(0.f)
	,bWhForce(false), whMaxAngVel(100.f), whSpinDamp(10)
	,whForceLong(30.f), whForceUp(50.f), whSteerMul(1.f)
	,bumpFqX(20.f), bumpFqY(30.f),bumpAmp(0.2f), bumpAng(0.5f)
	,idParticles(0)
{	}


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------

bool FluidsXml::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	//Default();
	fls.clear();  flMap.clear();

	///  fluids
	const char* a;  int i=1;  //0 = none
	XMLElement* eFl = root->FirstChildElement("fluid");
	while (eFl)
	{
		FluidParams fp;
		a = eFl->Attribute("name");			if (a)  fp.name = std::string(a);
		a = eFl->Attribute("material");		if (a)  fp.material = std::string(a);
		//  car buoyancy
		a = eFl->Attribute("density");		if (a)  fp.density = s2r(a);
		a = eFl->Attribute("angDamp");		if (a)  fp.angularDrag = s2r(a);
		a = eFl->Attribute("linDamp");		if (a)  fp.linearDrag = s2r(a);
		a = eFl->Attribute("heightDamp");	if (a)  fp.heightVelRes = s2r(a);

		//  wheel
		a = eFl->Attribute("bWhForce");		if (a)  fp.bWhForce = s2i(a) > 0;
		a = eFl->Attribute("whMaxAngVel");	if (a)  fp.whMaxAngVel = s2r(a);

		a = eFl->Attribute("whSpinDamp");	if (a)  fp.whSpinDamp = s2r(a);
		a = eFl->Attribute("whForceLong");	if (a)  fp.whForceLong = s2r(a);
		a = eFl->Attribute("whForceUp");	if (a)  fp.whForceUp = s2r(a);
		a = eFl->Attribute("whSteerMul");	if (a)  fp.whSteerMul = s2r(a);

		a = eFl->Attribute("bumpFqX");		if (a)  fp.bumpFqX = s2r(a);
		a = eFl->Attribute("bumpFqY");		if (a)  fp.bumpFqY = s2r(a);
		a = eFl->Attribute("bumpAmp");		if (a)  fp.bumpAmp = s2r(a);
		a = eFl->Attribute("bumpAngle");	if (a)  fp.bumpAng = s2r(a);
		
		a = eFl->Attribute("idParticles");	if (a)  fp.idParticles = s2i(a);
		//

		fls.push_back(fp);
		flMap[fp.name] = i++;
		eFl = eFl->NextSiblingElement("fluid");
	}
	return true;
}
