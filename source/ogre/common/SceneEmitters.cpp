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

		ParticleSystem* ps;
		try
		{	ps = app->mSceneMgr->createParticleSystem(n, em.name);  //ToDel(ps);
		}
		catch (Exception& ex)
		{
			LogO("Warning: emitter " + toStr(i) + " particle system: " + em.name + " doesn't exist");
			continue;
		}
		ps->setVisibilityFlags(RV_Particles);
		ps->setRenderQueueGroup(RQG_CarParticles);

		SceneNode* nd = rt->createChildSceneNode(em.pos);  //ToDel(nb);
		nd->attachObject(ps);
		em.nd = nd;  em.ps = ps;

		em.par.x = s2r(ps->getParameter("particle_width"));  // store orig
		em.par.y = s2r(ps->getParameter("particle_height"));
		em.UpdEmitter();
		ps->_update(em.upd);  //  started already 2 sec ago
		ps->setSpeedFactor(em.stat ? 0.f : 1.f);  // static
	}
}

void SEmitter::UpdEmitter()
{
	if (!ps)  return;
	ps->setParameter("particle_width",  toStr(parScale * par.x));
	ps->setParameter("particle_height", toStr(parScale * par.y));
	ps->getEmitter(0)->setParameter("width",  toStr(size.x));
	ps->getEmitter(0)->setParameter("height", toStr(size.z));
	ps->getEmitter(0)->setParameter("depth",  toStr(size.y));  // h
	ps->getEmitter(0)->setEmissionRate(rate);
}

void CScene::DestroyEmitters(bool clear)
{
	for (int i=0; i < sc->emitters.size(); ++i)
	{
		SEmitter& em = sc->emitters[i];
		if (em.nd) {  app->mSceneMgr->destroySceneNode(em.nd);  em.nd = 0;  }
		if (em.ps) {  app->mSceneMgr->destroyParticleSystem(em.ps);  em.ps = 0;  }
	}
	if (clear)
		sc->emitters.clear();
}


//-------------------------------------------------------------------------------------------------------
#ifdef SR_EDITOR
void App::UpdEmtBox()
{
	int emts = scn->sc->emitters.size();
	bool vis = edMode == ED_Particles && emts > 0 && !bMoveCam && bParticles;
	if (emts > 0)
		iEmtCur = std::max(0, std::min(iEmtCur, emts-1));

	if (!ndEmtBox)  return;
	ndEmtBox->setVisible(vis);
	if (!vis)  return;
	
	const SEmitter& em = scn->sc->emitters[iEmtCur];
	ndEmtBox->setPosition(em.pos);
	//ndEmtBox->setOrientation(Quaternion(Degree(em.rot), em.up));
	ndEmtBox->setScale(em.size);
}

void App::SetEmtType(int rel)
{
	int emtAll = vEmtNames.size();
	if (emtAll <= 0)  return;
	iEmtNew = (iEmtNew + rel + emtAll) % emtAll;
	
	int emts = scn->sc->emitters.size();
	if (emts <= 0)  return;
	SEmitter& em = scn->sc->emitters[iEmtCur];
	em.name = vEmtNames[iEmtNew];
}
#endif
