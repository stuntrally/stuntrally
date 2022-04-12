#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "data/SceneXml.h"
#include "data/CData.h"
#include "ShapeData.h"
#include "CScene.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
#else
	#include "../CGame.h"
#endif
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
using namespace Ogre;



///  create particle Emitters  : : : : : : : : 
//----------------------------------------------------------------------------------------------------------------------
void CScene::CreateEmitters()
{
	SceneNode* rt = app->mSceneMgr->getRootSceneNode();
	for (int i=0; i < sc->emitters.size(); ++i)
	{
		String n = "PE_" +toStr(i);
		SEmitter& em = sc->emitters[i];
		if (em.name.empty())  continue;

		ParticleSystem* ps = app->mSceneMgr->createParticleSystem(n, em.name);  //ToDel(ps);
		ps->setVisibilityFlags(RV_Particles);
		ps->setRenderQueueGroup(RQG_CarParticles);

		SceneNode* nd = rt->createChildSceneNode(em.pos);  //ToDel(nb);
		nd->attachObject(ps);
		ps->getEmitter(0)->setEmissionRate(em.rate);
		ps->getEmitter(0)->setParameter("width",  toStr(em.size.x));
		ps->getEmitter(0)->setParameter("height", toStr(em.size.y));
		ps->getEmitter(0)->setParameter("depth",  toStr(em.size.z));
		ps->getEmitter(0)->setUp(em.up);
		ps->_update(em.upd);  //  started already 2 sec ago
		if (em.stat)
			ps->setSpeedFactor(0.f);  // static

		em.nd = nd;  em.par = ps;
	}
}

void CScene::DestroyEmitters(bool clear)
{
	for (int i=0; i < sc->emitters.size(); ++i)
	{
		SEmitter& em = sc->emitters[i];
		if (em.par) {  app->mSceneMgr->destroyParticleSystem(em.par);  em.par = 0;  }
		if (em.nd)  {  app->mSceneMgr->destroySceneNode(em.nd);  em.nd = 0;  }
	}
	if (clear)
		sc->emitters.clear();
}


//  Pick
//-------------------------------------------------------------------------------------------------------

#ifdef SR_EDITOR
void App::UpdEmtPick()
{
	int emts = scn->sc->emitters.size();
	bool vis = edMode == ED_Emitters && !bMoveCam && emts > 0 && iEmtCur >= 0;
	if (emts > 0)
		iEmtCur = std::min(iEmtCur, emts-1);

	if (!ndEmtBox)  return;
	ndEmtBox->setVisible(vis);
	if (!vis)  return;
	
	const SEmitter& e = scn->sc->emitters[iEmtCur];

	ndEmtBox->setPosition(e.pos);
	//ndEmtBox->setOrientation(rotO);
	ndEmtBox->setScale(e.size);
}
#endif
