#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/data/CData.h"
#include "../vdrift/pathmanager.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/Axes.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/WaterRTT.h"
#include "../ogre/common/RenderBoxScene.h"
#include "settings.h"
#include "../shiny/Main/Factory.hpp"
#include "../shiny/Platforms/Ogre/OgrePlatform.hpp"
#include "../shiny/Platforms/Ogre/OgreMaterial.hpp"
#include <OgreTerrainPaging.h>
#include <OgreTerrainGroup.h>
using namespace Ogre;


//  ctor
//----------------------------------------------------------------------------------------------------------------------
App::App(SETTINGS* pSet1)
{
	pSet = pSet1;
	Axes::Init();
	
	mBrSize[0] = 16.f;	mBrSize[1] = 24.f;	mBrSize[2] = 16.f;	mBrSize[3] = 16.f;
	mBrIntens[0] = 20.f;mBrIntens[1] = 20.f;mBrIntens[2] = 20.f;mBrIntens[3] = 20.f;
	mBrPow[0] = 2.f;	mBrPow[1] = 2.f;	mBrPow[2] = 2.f;	mBrPow[3] = 2.f;
	mBrFq[0] = 1.f;		mBrFq[1] = 1.f;		mBrFq[2] = 1.f;		mBrFq[3] = 1.f;
	mBrNOf[0] = 0.f;	mBrNOf[1] = 0.f;	mBrNOf[2] = 0.f;	mBrNOf[3] = 0.f;
	mBrOct[0] = 5;		mBrOct[1] = 5;		mBrOct[2] = 5;		mBrOct[3] = 5;
	mBrShape[0] = BRS_Sinus;  mBrShape[1] = BRS_Sinus;
	mBrShape[2] = BRS_Sinus;  mBrShape[3] = BRS_Sinus;
	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	updBrush();

	///  new
	scn = new CScene(this);

	gcom = new CGuiCom(this);
	gcom->mGui = mGui;
	gcom->sc = scn->sc;

	gui = new CGui(this);
	gui->viewBox = new wraps::RenderBoxScene();
	gui->gcom = gcom;
	gui->sc = scn->sc;
	gui->scn = scn;
	gui->data = scn->data;
}

const Ogre::String App::csBrShape[BRS_ALL] =
{ "Triangle", "Sinus", "Noise", "Noise2", "N-gon" };  // static


///  material factory setup
//---------------------------------------------------------------------------------------------------------------------------
void App::postInit()
{
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/" + "materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());
	
	mFactory = new sh::Factory(platform);
	SetFactoryDefaults();

	mFactory->setMaterialListener(this);
}


App::~App()
{
	DestroyObjects(false);

	delete scn;

	delete mFactory;  //!

	gui->viewBox->destroy();
	delete gui->viewBox;

	BltWorldDestroy();
	
	delete[] pBrFmask;  pBrFmask = 0;

	delete[] mBrushData;

	delete gcom;
	delete gui;
}
	

//  util
//---------------------------------------------------------------------------------------------------------------
ManualObject* App::Create2D(const String& mat, Real s, bool dyn)
{
	ManualObject* m = mSceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);
	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_STRIP);
	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RQG_Hud2);
	return m;
}


void App::materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex)
{
	Ogre::Technique* t = static_cast<sh::OgreMaterial*>(m->getMaterial())->getOgreTechniqueForConfiguration (configuration, lodIndex);

	if (pSet->shadow_type == Sh_None)
	{
		t->setShadowCasterMaterial("");
		return;
	}

	/*if (m->hasProperty("transparent") && m->hasProperty("cull_hardware") &&
		sh::retrieveValue<sh::StringValue>(m->getProperty("cull_hardware"), 0).get() == "none")
	{
		// Crash !?
		assert(!MaterialManager::getSingleton().getByName("PSSM/shadow_caster_nocull").isNull());
		t->setShadowCasterMaterial("shadowcaster_nocull");
	}*/

	if (m->hasProperty("instancing") && sh::retrieveValue<sh::StringValue>(m->getProperty("instancing"), 0).get() == "true")
	{
		t->setShadowCasterMaterial("shadowcaster_instancing");
	}

	if (!m->hasProperty("transparent") || !sh::retrieveValue<sh::BooleanValue>(m->getProperty("transparent"), 0).get())
	{
		t->setShadowCasterMaterial("shadowcaster_noalpha");
	}
}
