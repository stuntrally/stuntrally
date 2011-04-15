#include "stdafx.h"
#include "CarModel.h"

CarModel::CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set) : 
	hue(0), sat(0), val(0), fCam(0), pMainNode(0), pCar(0)
{
	iIndex = index;
	sDirname = name;
	pSceneMgr = sceneMgr;
	pSet = set;
	
	pReflect = new CarReflection(pSet, iIndex);
}
CarModel::~CarModel(void)
{
	delete pReflect;
	///TODO
}
void CarModel::Update(void)
{
	pReflect->Update();
}
