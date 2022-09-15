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


void App::CreateBox(SceneNode*& nd, Entity*& ent, String sMat, String sMesh, int x)
{
	if (nd)  return;
	MaterialPtr mtr;
	bool e = sMat.empty();
	if (!e)
	{	mtr = MaterialManager::getSingleton().getByName(sMat);
		if (!mtr)  return;
	}
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ent = mSceneMgr->createEntity(sMesh);
	ent->setVisibilityFlags(RV_Hud);  nd->setPosition(Vector3(x,0,0));
		ent->setCastShadows(false);  if (!e)  ent->setMaterial(mtr);
		ent->setRenderQueueGroup(RQG_CarGhost);  // after road
	nd->attachObject(ent);
	nd->setVisible(false);
}


void App::UpdStartPos()
{
	CreateBox(ndCar, entCar, "", "car.mesh");
	
	CreateBox(ndStBox[0], entStBox[0], "start_box", "cube.mesh", 20000);
	CreateBox(ndStBox[1], entStBox[1], "end_box", "cube.mesh", 20000);

	CreateBox(ndFluidBox, entFluidBox, "fluid_box", "box_fluids.mesh");
	CreateBox(ndObjBox, entObjBox, "object_box", "box_obj.mesh");
	CreateBox(ndEmtBox, entEmtBox, "emitter_box", "box_obj.mesh");

	//  start, end boxes
	for (int i=0; i < 2; ++i)
	{
		Vector3 p1 = Axes::toOgre(scn->sc->startPos[i]);
		Quaternion q1 = Axes::toOgre(scn->sc->startRot[i]);
		if (i == iEnd)
		{
			ndCar->setPosition(p1);  ndCar->setOrientation(q1);
			ndCar->setVisible(scn->road);  // hide before load
		}
		ndStBox[i]->setPosition(p1);  ndStBox[i]->setOrientation(q1);

		if (scn->road)
			ndStBox[i]->setScale(Vector3(1, scn->road->vStBoxDim.y, scn->road->vStBoxDim.z));
	
		ndStBox[i]->setVisible(edMode == ED_Start && bEdit());
	}
}
