#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/Axes.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <Ogre.h>
using namespace Ogre;


void App::UpdStartPos()
{
	if (!ndCar)
	{ 	//  car for start pos
 		ndCar = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		entCar = mSceneMgr->createEntity("car.mesh");
		entCar->setVisibilityFlags(RV_Hud);
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

	Vector3 p1 = Axes::toOgre(scn->sc->startPos);
	Quaternion q1 = Axes::toOgre(scn->sc->startRot);
	ndCar->setPosition(p1);  ndCar->setOrientation(q1);
	ndCar->setVisible(scn->road);  // hide before load

	ndStBox->setPosition(p1);  ndStBox->setOrientation(q1);
	if (scn->road)
	ndStBox->setScale(Vector3(1, scn->road->vStBoxDim.y, scn->road->vStBoxDim.z));
	ndStBox->setVisible(edMode == ED_Start && bEdit());
}
