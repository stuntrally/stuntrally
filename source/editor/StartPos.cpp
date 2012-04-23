#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../ogre/common/RenderConst.h"
using namespace Ogre;


bool App::LoadStartPos()
{
	vStartPos.clear();  // clear
	vStartRot.clear();
	std::string path = TrkDir()+"track.txt";
	CONFIGFILE param;
	if (!param.Load(path))
		return false;

	int sp_num = 0;
	float f3[3], f1;
	QUATERNION <float> fixer;  fixer.Rotate(3.141593, 0,0,1);
	
	while (param.GetParam("start position "+toStr(sp_num), f3))
	{
		MATHVECTOR <float, 3> pos(f3[2], f3[0], f3[1]);

		if (!param.GetParam("start orientation-xyz "+toStr(sp_num), f3))
			return false;

		if (!param.GetParam("start orientation-w "+toStr(sp_num), f1))
			return false;

		QUATERNION <float> orient(f3[2], f3[0], f3[1], f1);
		orient = fixer * orient;

		vStartPos.push_back(pos);  // add
		vStartRot.push_back(orient);
		sp_num++;
	}
	
	UpdStartPos();
	return true;
}

bool App::SaveStartPos(std::string path)
{
	CONFIGFILE param;
	if (!param.Load(path))
		return false;
		
	QUATERNION <float> fixer;  fixer.Rotate(-3.141593, 0,0,1);
	for (int i=0; i < 4; ++i)
	{
		int n = 0;  // 0- all same  i- edit 4
		//  pos
		float p3[3] = {vStartPos[n][1], vStartPos[n][2], vStartPos[n][0]};
		param.SetParam("start position "+toStr(i), p3);
		
		//  rot
		QUATERNION <float> orient = vStartRot[n];
		orient = fixer * orient;
		float f3[3] = {orient.y(), orient.z(), orient.x()}, f1 = orient.w();

		param.SetParam("start orientation-xyz "+toStr(i), f3);
		param.SetParam("start orientation-w "+toStr(i), f1);
	}
	return param.Write();
}


void App::UpdStartPos()
{
	if (!ndCar)
	{ 	//  car for start pos
 		ndCar = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		entCar = mSceneMgr->createEntity("car.mesh");
		entCar->setVisibilityFlags(RV_Hud);  ndCar->setPosition(Vector3(20000,0,0));
		ndCar->attachObject(entCar);
	}
	if (!ndStBox)
	{ 	//  start pos box
		MaterialPtr mtr = Ogre::MaterialManager::getSingleton().getByName("sphere_check");
		if (!mtr.isNull())
 		{	ndStBox = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			entStBox = mSceneMgr->createEntity("cube.mesh");
			entStBox->setVisibilityFlags(RV_Hud);  ndStBox->setPosition(Vector3(20000,0,0));
				entStBox->setCastShadows(true);  //`
				entStBox->setMaterial(mtr);  entStBox->setRenderQueueGroup(RQG_CarGlass);  // after road
			ndStBox->attachObject(entStBox);
	}	}
	if (!ndFluidBox)
	{ 	//  fluid edit box
		MaterialPtr mtr = Ogre::MaterialManager::getSingleton().getByName("fluid_box");
		if (!mtr.isNull())
 		{	ndFluidBox = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			entFluidBox = mSceneMgr->createEntity("box.mesh");
			entFluidBox->setVisibilityFlags(RV_Hud);  ndFluidBox->setPosition(Vector3(0,0,0));
				entFluidBox->setCastShadows(false);  //`
				entFluidBox->setMaterial(mtr);  entFluidBox->setRenderQueueGroup(RQG_CarGlass);
			ndFluidBox->attachObject(entFluidBox);
			ndFluidBox->setVisible(false);
	}	}
	if (!ndObjBox)
	{ 	//  picked object box
		MaterialPtr mtr = Ogre::MaterialManager::getSingleton().getByName("object_box");
		if (!mtr.isNull())
 		{	ndObjBox = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			entObjBox = mSceneMgr->createEntity("box_obj.mesh");
			entObjBox->setVisibilityFlags(RV_Hud);  ndObjBox->setPosition(Vector3(0,0,0));
				entObjBox->setCastShadows(false);  //`
				entObjBox->setMaterial(mtr);  entObjBox->setRenderQueueGroup(RQG_CarGlass);
			ndObjBox->attachObject(entObjBox);
			ndObjBox->setVisible(false);
	}	}
	if (vStartPos.size() < 4 || vStartRot.size() < 4)  return;

	float* pos = &vStartPos[0][0];
	float* rot = &vStartRot[0][0];

	Vector3 p1 = Vector3(pos[0],pos[2],-pos[1]);

	Quaternion q(rot[0],rot[1],rot[2],rot[3]);
	Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);

	Vector3 vrot(axi.z, -axi.x, -axi.y);
		QUATERNION <double> fix;  fix.Rotate(PI_d, 0, 1, 0);
		Quaternion qr;  qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();
	Quaternion q1;  q1.FromAngleAxis(-rad, vrot);  q1 = q1 * qr;
	//Vector3 vcx,vcy,vcz;  q1.ToAxes(vcx,vcy,vcz);

	ndCar->setPosition(p1);    ndCar->setOrientation(q1);
	ndStBox->setPosition(p1);  ndStBox->setOrientation(q1);
	if (road)
	ndStBox->setScale(Vector3(1,road->vStBoxDim.y,road->vStBoxDim.z));
	ndStBox->setVisible(edMode == ED_Start && bEdit());
}
