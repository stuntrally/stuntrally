#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/data/SceneXml.h"
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
using namespace Ogre;


//  Sky Dome
//----------------------------------------------------------------------------------------------------------------------
void App::CreateSkyDome(String sMater, Vector3 sc)
{
	ManualObject* m = mSceneMgr->createManualObject();
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

	ndSky = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndSky->attachObject(m);
	ndSky->setScale(sc);
}

//  fog
void App::UpdFog(bool bForce)
{
	const ColourValue clr(0.5,0.6,0.7,1);
	bool ok = !pSet->bFog || bForce;
	if (ok)
		mSceneMgr->setFog(FOG_LINEAR, clr, 1.f, sc->fogStart, sc->fogEnd);
	else
		mSceneMgr->setFog(FOG_NONE, clr, 1.f, 9000, 9200);

	mFactory->setSharedParameter("fogColorSun",  sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClr2.x, sc->fogClr2.y, sc->fogClr2.z, sc->fogClr2.w)));
	mFactory->setSharedParameter("fogColorAway", sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClr.x,  sc->fogClr.y,  sc->fogClr.z,  sc->fogClr.w)));
	mFactory->setSharedParameter("fogColorH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClrH.x, sc->fogClrH.y, sc->fogClrH.z, sc->fogClrH.w)));
	mFactory->setSharedParameter("fogParamsH",   sh::makeProperty<sh::Vector4>(new sh::Vector4(
		sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f, sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart) )));
}

inline ColourValue Clr3(const Vector3& v)
{
	return ColourValue(v.x, v.y, v.z);
}

void App::UpdSun()
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc->ldYaw, -sc->ldPitch);
	sun->setDirection(dir);
	sun->setDiffuseColour(Clr3(sc->lDiff));
	sun->setSpecularColour(Clr3(sc->lSpec));
	mSceneMgr->setAmbientLight(Clr3(sc->lAmb));
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

	fct.setSharedParameter("fogColorSun",  sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogColorAway", sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogColorH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogParamsH",   sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));

	fct.setSharedParameter("pssmSplitPoints", sh::makeProperty<sh::Vector3>(new sh::Vector3(0,0,0)));
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
	fct.setGlobalSetting("terrain_triplanarLayer", toStr(sc->td.triplanar1Layer));

	fct.setGlobalSetting("water_reflect", b2s(pSet->water_reflect));
	fct.setGlobalSetting("water_refract", b2s(pSet->water_refract));
	fct.setSharedParameter("waterEnabled", sh::makeProperty<sh::FloatValue> (new sh::FloatValue(0.0)));
	fct.setSharedParameter("waterLevel", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterTimer", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterSunFade_sunHeight", sh::makeProperty<sh::Vector2>(new sh::Vector2(1, 0.6)));
	fct.setSharedParameter("windDir_windSpeed", sh::makeProperty<sh::Vector3>(new sh::Vector3(0.5, -0.8, 0.2)));


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
		else  assert(0);
	}
	mFactory->setCurrentLanguage(lang);

	mFactory->loadAllFiles();
}
