#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <Ogre.h>
using namespace Ogre;


bool App::LoadStartPos(std::string path1, bool tool)
{
	std::string path = path1+"track.txt";
	CONFIGFILE param;
	if (!param.Load(path))
		return false;

	float f3[3], f1;
	QUATERNION <float> fixer;  fixer.Rotate(3.141593, 0,0,1);
	
	param.GetParam("start position 0", f3);
	MATHVECTOR <float, 3> pos(f3[2], f3[0], f3[1]);

	if (!param.GetParam("start orientation-xyz 0", f3))
		return false;

	if (!param.GetParam("start orientation-w 0", f1))
		return false;

	QUATERNION <float> rot(f3[2], f3[0], f3[1], f1);
	rot = fixer * rot;

	vStartPos = pos;
	vStartRot = rot;

	if (!tool)
		UpdStartPos();
	return true;
}

bool App::SaveStartPos(std::string path)
{
	CONFIGFILE param;
	if (!param.Load(path))
		return false;
		
	QUATERNION <float> fixer;  fixer.Rotate(-3.141593, 0,0,1);

	//  pos
	float p3[3] = {vStartPos[1], vStartPos[2], vStartPos[0]};
	param.SetParam("start position 0", p3);
		
	//  rot
	QUATERNION <float> rot = vStartRot;
	rot = fixer * rot;
	float f3[3] = {rot.y(), rot.z(), rot.x()}, f1 = rot.w();

	param.SetParam("start orientation-xyz 0", f3);
	param.SetParam("start orientation-w 0", f1);

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
		MaterialPtr mtr = MaterialManager::getSingleton().getByName("start_box");
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
		MaterialPtr mtr = MaterialManager::getSingleton().getByName("fluid_box");
		if (!mtr.isNull())
 		{	ndFluidBox = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			entFluidBox = mSceneMgr->createEntity("box_fluids.mesh");
			entFluidBox->setVisibilityFlags(RV_Hud);  ndFluidBox->setPosition(Vector3(0,0,0));
				entFluidBox->setCastShadows(false);  //`
				entFluidBox->setMaterial(mtr);  entFluidBox->setRenderQueueGroup(RQG_CarGlass);
			ndFluidBox->attachObject(entFluidBox);
			ndFluidBox->setVisible(false);
	}	}
	if (!ndObjBox)
	{ 	//  picked object box
		MaterialPtr mtr = MaterialManager::getSingleton().getByName("object_box");
		if (!mtr.isNull())
 		{	ndObjBox = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			entObjBox = mSceneMgr->createEntity("box_obj.mesh");
			entObjBox->setVisibilityFlags(RV_Hud);  ndObjBox->setPosition(Vector3(0,0,0));
				entObjBox->setCastShadows(false);  //`
				entObjBox->setMaterial(mtr);  entObjBox->setRenderQueueGroup(RQG_CarGlass);
			ndObjBox->attachObject(entObjBox);
			ndObjBox->setVisible(false);
	}	}

	float* pos = &vStartPos[0];
	float* rot = &vStartRot[0];

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
	if (scn->road)
	ndStBox->setScale(Vector3(1, scn->road->vStBoxDim.y, scn->road->vStBoxDim.z));
	ndStBox->setVisible(edMode == ED_Start && bEdit());
}
