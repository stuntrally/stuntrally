#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/data/SceneXml.h"
#include "../common/CScene.h"
#include "../../road/Road.h"  // sun rot
#include "../shiny/Main/Factory.hpp"
#include "../../vdrift/dbl.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
#else
	#include "../CGame.h"
	#include "../../vdrift/settings.h"
#endif
#include <OgreRoot.h>
#include <OgreManualObject.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreRenderWindow.h>
using namespace Ogre;


//  Sky Dome
//-------------------------------------------------------------------------------------
void CScene::CreateSkyDome(String sMater, Vector3 sc, float yaw)
{
	ManualObject* m = app->mSceneMgr->createManualObject();
	m->begin(sMater, RenderOperation::OT_TRIANGLE_LIST);

	//  divisions- quality
	int ia = 32*2, ib = 24,iB = 24 +1/*below_*/, i=0;
	//int ia = 4, ib = 4, i=0;
	//  angles, max
	float a,b;  const float B = PI_d/2.f, A = 2.f*PI_d;
	float bb = B/ib, aa = A/ia;  // add
	ia += 1;

	//  up/dn y  )
	for (b = 0.f; b <= B+bb/*1*/*iB; b += bb)
	{
		float cb = sinf(b), sb = cosf(b);
		float y = sb;

		//  circle xz  o
		for (a = 0.f; a <= A; a += aa, ++i)
		{
			float x = cosf(a)*cb, z = sinf(a)*cb;
			m->position(x,y,z);

			m->textureCoord(a/A, b/B);

			if (a > 0.f && b > 0.f)  // rect 2tri
			{
				m->index(i-1);  m->index(i);     m->index(i-ia);
				m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
			}
		}
	}
	m->end();
	AxisAlignedBox aab;  aab.setInfinite();
	m->setBoundingBox(aab);  // always visible
	m->setRenderQueueGroup(RQG_Sky);
	m->setCastShadows(false);
	#ifdef SR_EDITOR
	m->setVisibilityFlags(RV_Sky);  // hide on minimap
	#endif

	app->ndSky = app->mSceneMgr->getRootSceneNode()->createChildSceneNode();
	app->ndSky->attachObject(m);
	app->ndSky->setScale(sc);
	Quaternion q;  q.FromAngleAxis(Degree(-yaw), Vector3::UNIT_Y);
	app->ndSky->setOrientation(q);
}


//  Sun
//-------------------------------------------------------------------------------------
void CScene::UpdSky()
{
	Quaternion q;  q.FromAngleAxis(Degree(-sc->skyYaw), Vector3::UNIT_Y);
	app->ndSky->setOrientation(q);
	UpdSun();
}

void CScene::UpdSun()
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc->ldYaw - sc->skyYaw, -sc->ldPitch);
	sun->setDirection(dir);
	sun->setDiffuseColour( sc->lDiff.GetClr());
	sun->setSpecularColour(sc->lSpec.GetClr());
	app->mSceneMgr->setAmbientLight(sc->lAmb.GetClr());
}

//  Fog
void CScene::UpdFog(bool bForce)
{
	const ColourValue clr(0.5,0.6,0.7,1);
	bool ok = !app->pSet->bFog || bForce;
	if (ok)
		app->mSceneMgr->setFog(FOG_LINEAR, clr, 1.f, sc->fogStart, sc->fogEnd);
	else
		app->mSceneMgr->setFog(FOG_NONE, clr, 1.f, 9000, 9200);

	Vector4 v;
	v = sc->fogClr2.GetRGBA();
	app->mFactory->setSharedParameter("fogColorSun",  sh::makeProperty<sh::Vector4>(new sh::Vector4(v.x, v.y, v.z, v.w)));
	v = sc->fogClr.GetRGBA();
	app->mFactory->setSharedParameter("fogColorAway", sh::makeProperty<sh::Vector4>(new sh::Vector4(v.x, v.y, v.z, v.w)));
	v = sc->fogClrH.GetRGBA();
	app->mFactory->setSharedParameter("fogColorH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(v.x, v.y, v.z, v.w)));
	app->mFactory->setSharedParameter("fogParamsH",   sh::makeProperty<sh::Vector4>(new sh::Vector4(
		sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f, sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart) )));
	
	// fluid fog, default off
	app->mFactory->setSharedParameter("fogFluidH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(-900.f, 1/17.f, 1.f, 0)));
	app->mFactory->setSharedParameter("fogFluidClr",  sh::makeProperty<sh::Vector4>(new sh::Vector4(0.5f,0.7,0.9f, 1.f)));
}


//  Weather  rain,snow
//-------------------------------------------------------------------------------------
void CScene::CreateWeather()
{
	if (!pr && !sc->rainName.empty())
	{	pr = app->mSceneMgr->createParticleSystem("Rain", sc->rainName);
		pr->setVisibilityFlags(RV_Particles);
		app->mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RQG_Weather);
		pr->getEmitter(0)->setEmissionRate(0);
	}
	if (!pr2 && !sc->rain2Name.empty())
	{	pr2 = app->mSceneMgr->createParticleSystem("Rain2", sc->rain2Name);
		pr2->setVisibilityFlags(RV_Particles);
		app->mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RQG_Weather);
		pr2->getEmitter(0)->setEmissionRate(0);
	}
}
void CScene::DestroyWeather()
{
	if (pr)  {  app->mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  app->mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }
}

void CScene::UpdateWeather(Camera* cam, float mul)
{
	const Vector3& pos = cam->getPosition(), dir = cam->getDirection();
	static Vector3 oldPos = Vector3::ZERO;

	Vector3 vel = (pos-oldPos) * app->mWindow->getLastFPS();  oldPos = pos;
	Vector3 par = pos + dir * 12.f + vel * 0.6f;

	if (pr && sc->rainEmit > 0)
	{
		ParticleEmitter* pe = pr->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(mul * sc->rainEmit);
	}
	if (pr2 && sc->rain2Emit > 0)
	{
		ParticleEmitter* pe = pr2->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(mul * sc->rain2Emit);
	}
}


//  Material Factory defaults
//----------------------------------------------------------------------------------------------------------------------
void App::SetFactoryDefaults()
{
	sh::Factory& fct = sh::Factory::getInstance();
	fct.setReadSourceCache(true);
	fct.setWriteSourceCache(true);
	fct.setReadMicrocodeCache(true);
	fct.setWriteMicrocodeCache(true);
	
	fct.setGlobalSetting("fog", "true");
	fct.setGlobalSetting("wind", "true");
	fct.setGlobalSetting("mrt_output", "false");
	fct.setGlobalSetting("shadows", "false");
	fct.setGlobalSetting("shadows_pssm", "false");
	fct.setGlobalSetting("shadows_depth", b2s(pSet->shadow_type >= Sh_Depth));
	fct.setGlobalSetting("lighting", "true");
	fct.setGlobalSetting("terrain_composite_map", "false");
	fct.setGlobalSetting("soft_particles", "false");
	#ifdef SR_EDITOR
	fct.setGlobalSetting("editor", "true");
	#else
	fct.setGlobalSetting("editor", "false");
	#endif

	scn->UpdFog(false);

	fct.setSharedParameter("pssmSplitPoints", sh::makeProperty<sh::Vector3>(new sh::Vector3(0.1,0.2,0.5)));
	fct.setSharedParameter("shadowFar_fadeStart", sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("arrowColour1", sh::makeProperty <sh::Vector3>(new sh::Vector3(0,0,0)));
	fct.setSharedParameter("arrowColour2", sh::makeProperty <sh::Vector3>(new sh::Vector3(0,0,0)));
	fct.setSharedParameter("windTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,500,0,-1)));
	fct.setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,500,0,-1)));
	fct.setSharedParameter("terrainWorldSize", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(1024)));
	fct.setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(1.0)));

	fct.setGlobalSetting("terrain_specular", b2s(pSet->ter_mtr >= 1));
	fct.setGlobalSetting("terrain_normal",   b2s(pSet->ter_mtr >= 2));
	fct.setGlobalSetting("terrain_parallax", b2s(pSet->ter_mtr >= 3));
	fct.setGlobalSetting("terrain_triplanarType", toStr(pSet->ter_tripl));
	fct.setGlobalSetting("terrain_triplanarLayer", toStr(scn->sc->td.triplanarLayer1));
	fct.setGlobalSetting("terrain_triplanarLayer2", toStr(scn->sc->td.triplanarLayer2));

	fct.setGlobalSetting("terrain_emissive_specular", b2s(scn->sc->td.emissive));
	fct.setSharedParameter("ter_scaleNormal", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(1.f)));
	fct.setSharedParameter("ter_specular_pow", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(32.f)));
	fct.setSharedParameter("ter_specular_pow_em", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(2.f)));

	fct.setGlobalSetting("water_reflect", b2s(pSet->water_reflect));
	fct.setGlobalSetting("water_refract", b2s(pSet->water_refract));
	fct.setSharedParameter("waterEnabled", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0.0)));
	fct.setSharedParameter("waterLevel", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterTimer", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterSunFade_sunHeight", sh::makeProperty<sh::Vector2>(new sh::Vector2(1, 0.6)));
	fct.setSharedParameter("windDir_windSpeed", sh::makeProperty<sh::Vector3>(new sh::Vector3(0.5, -0.8, 0.2)));
	fct.setSharedParameter("paceParams", sh::makeProperty<sh::Vector4>(new sh::Vector4(1.f, 5.f, 0.02f, 1.f)));


	///  uncomment to enable shader output to files
	//mFactory->setShaderDebugOutputEnabled(true);

	sh::Language lang;
	if (pSet->shader_mode == "")
	{
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		lang = sh::Language_HLSL;
	#else
		lang = sh::Language_GLSL;
	#endif
	}else
	{	     if (pSet->shader_mode == "glsl") lang = sh::Language_GLSL;
		else if (pSet->shader_mode == "cg")	  lang = sh::Language_CG;
		else if (pSet->shader_mode == "hlsl") lang = sh::Language_HLSL;
		else
		{	LogO("Error: Unknown shader_mode !");  assert(0);  }
	}
	mFactory->setCurrentLanguage(lang);

	mFactory->loadAllFiles();
}

void CScene::UpdPaceParams()  // pacenotes
{
	app->mFactory->setSharedParameter("paceParams", sh::makeProperty<sh::Vector4>(new sh::Vector4(
		app->pSet->pace_size, 5.f / app->pSet->pace_near,
		0.03f * app->pSet->pace_near, app->pSet->pace_alpha)));
}
