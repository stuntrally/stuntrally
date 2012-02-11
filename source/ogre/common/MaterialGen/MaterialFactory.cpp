#include "pch.h"
#include "../Defines.h"

#include "MaterialFactory.h"
#include "MaterialDefinition.h"
#include "MaterialGenerator.h"
#include "ShaderProperties.h"

#include "GlassMaterial.h"
#include "PipeGlassMaterial.h"
#include "ArrowMaterial.h"
#include "WaterMaterial.h"
#include "ImpostorMaterial.h"
#include "ParticleMaterial.h"

#ifndef ROAD_EDITOR
	#include "../../OgreGame.h"
#else
	#include "../../../editor/OgreApp.h"
#endif
#include "../QTimer.h"

#include <OgreConfigFile.h>
#include <OgreResourceGroupManager.h>
#include <OgreStringConverter.h>
#include <OgreResourceGroupManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreShadowCameraSetupPSSM.h>
#include <OgreTerrain.h>
using namespace Ogre;

// use shader cache
// if you disable it, startup time will be A LOT longer...
// so the only reason you would disable this is to trace down a bug
#define USE_CACHE

// are we using a customized terrain material generator?
#define USE_CUSTOM_TERRAIN_MATERIAL

//----------------------------------------------------------------------------------------

// ms_Singleton was renamed to msSingleton in ogre 1.8
#if OGRE_VERSION_MINOR < 8
template<> MaterialFactory* Ogre::Singleton<MaterialFactory>::ms_Singleton = 0;

MaterialFactory* MaterialFactory::getSingletonPtr(void)
{
    return ms_Singleton;
}
MaterialFactory& MaterialFactory::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}
#else
template<> MaterialFactory* Ogre::Singleton<MaterialFactory>::msSingleton = 0;

MaterialFactory* MaterialFactory::getSingletonPtr(void)
{
    return msSingleton;
}
MaterialFactory& MaterialFactory::getSingleton(void)
{  
    assert( msSingleton );  return ( *msSingleton );  
}
#endif

//----------------------------------------------------------------------------------------

MaterialFactory::MaterialFactory() : 
	bNormalMap(1), bEnvMap(1), bShadows(1), bShadowsDepth(1), bShadowsSoft(1), bShadowsFade(0),
	iTexSize(4096), iNumShadowTex(3), iShadowsFilterSize(4), fShaderQuality(0.5), fShadowsFadeDistance(20.f),
	bSettingsChanged(1), // always have to generate at start
	
	mSceneMgr(0), mTerrain(0), mPSSM(0)
{
	QTimer ti;  ti.update(); /// time
	
	// find all files with *.matdef extension in all resource groups
	StringVector resourceGroups = ResourceGroupManager::getSingleton().getResourceGroups();
	for (StringVector::iterator it = resourceGroups.begin();
		it != resourceGroups.end(); ++it)
	{
		StringVectorPtr files = ResourceGroupManager::getSingleton().findResourceNames(
			(*it),
			"*.matdef");
		
		for (StringVector::iterator fit = files->begin();
			fit != files->end(); ++fit)
		{
			loadDefsFromFile( (*fit) );
		}
	}
	
	// create our generators
	mGenerator = new MaterialGenerator();
	mGenerator->mParent = this;
	
	MaterialGenerator* glass = static_cast<MaterialGenerator*>(new GlassMaterialGenerator());
	glass->mParent = this;
	mCustomGenerators.push_back(glass);
	
	MaterialGenerator* pipeglass = static_cast<MaterialGenerator*>(new PipeGlassMaterialGenerator());
	pipeglass->mParent = this;
	mCustomGenerators.push_back(pipeglass);
	
	MaterialGenerator* arrow = static_cast<MaterialGenerator*>(new ArrowMaterialGenerator());
	arrow->mParent = this;
	mCustomGenerators.push_back(arrow);
	
	MaterialGenerator* water = static_cast<MaterialGenerator*>(new WaterMaterialGenerator());
	water->mParent = this;
	mCustomGenerators.push_back(water);
	
	MaterialGenerator* impostor = static_cast<MaterialGenerator*>(new ImpostorMaterialGenerator());
	impostor->mParent = this;
	mCustomGenerators.push_back(impostor);
	
	MaterialGenerator* particle = static_cast<MaterialGenerator*>(new ParticleMaterialGenerator());
	particle->mParent = this;
	mCustomGenerators.push_back(particle);
	
	ti.update(); /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time loading material definitions: ") + toStr(dt) + " ms");
}

//----------------------------------------------------------------------------------------

MaterialFactory::~MaterialFactory()
{
	for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
		it!=mDefinitions.end(); ++it)
		delete (*it);
	
	deleteShaderCache();
	
	delete mGenerator;
	
	std::vector<MaterialGenerator*>::iterator gIt;
	for (gIt = mCustomGenerators.begin(); gIt != mCustomGenerators.end(); ++gIt)
		delete (*gIt);
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setSceneManager(SceneManager* pSceneMgr)
{
	mSceneMgr = pSceneMgr;
}

SceneManager* MaterialFactory::getSceneManager()
{
	return mSceneMgr;
}

void MaterialFactory::setTerrain(Terrain* pTerrain)
{
	mTerrain = pTerrain;
}

Terrain* MaterialFactory::getTerrain()
{
	return mTerrain;
}

void MaterialFactory::setPSSMCameraSetup(PSSMShadowCameraSetup* setup)
{
	mPSSM = setup;
}

//----------------------------------------------------------------------------------------

void MaterialFactory::deleteShaderCache()
{
	for (shaderMap::iterator it=mShaderCache.begin(); it!=mShaderCache.end(); ++it)
		delete (*it).second;
	
	mShaderCache.clear();
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setFog(bool fog)
{
	for (std::vector<std::string>::iterator it=fogMtrs.begin();
		it != fogMtrs.end(); ++it)
	{
		MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it) );
		if (mat->getTechnique(0)->getPass(0)->hasVertexProgram())
		{
			GpuProgramParametersSharedPtr vparams = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			vparams->setNamedConstant("enableFog", fog ? Real(1.0) : Real(0.0));
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setWind(bool wind)
{
	for (std::vector<std::string>::iterator it=windMtrs.begin();
		it != windMtrs.end(); ++it)
	{
		MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it) );
		if (mat->getTechnique(0)->getPass(0)->hasVertexProgram())
		{
			GpuProgramParametersSharedPtr vparams = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
			vparams->setNamedConstant("enableWind", wind ? Real(1.0) : Real(0.0));
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setSoftParticleDepth(TexturePtr depthtexture)
{
	if(MaterialGenerator::MRTSupported())
	{
		for (std::vector<std::string>::iterator it=softMtrs.begin();
			it != softMtrs.end(); ++it)
		{
			MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it) );
			TextureUnitState* tus =mat->getTechnique(0)->getPass(0)->getTextureUnitState("depthMap");
			tus->setTextureName(depthtexture->getName());
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setSoftParticles(bool bEnable)
{
	if(MaterialGenerator::MRTSupported())
	{
		for (std::vector<std::string>::iterator it=softMtrs.begin();
			it != softMtrs.end(); ++it)
		{
			MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it) );
			if (mat->getTechnique(0)->getPass(0)->hasFragmentProgram())
			{
				GpuProgramParametersSharedPtr vparams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				vparams->setNamedConstant("useSoftParticles", bEnable ? 1.0f: -1.0f);
			}
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setShadowsEnabled(bool bEnable)
{
	for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
		it!=mDefinitions.end(); ++it)
	{
		MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it)->getName() );
			
		if (mat.isNull()) continue;
		
		Material::TechniqueIterator techIt = mat->getTechniqueIterator();
		while (techIt.hasMoreElements())
		{
			Technique* tech = techIt.getNext();
			Technique::PassIterator passIt = tech->getPassIterator();
			while (passIt.hasMoreElements())
			{
				Pass* pass = passIt.getNext();
									
				if (pass->hasFragmentProgram())
				{
					if ( pass->getFragmentProgramParameters()->_findNamedConstantDefinition("enableShadows", false))
						pass->getFragmentProgramParameters()->setNamedConstant("enableShadows", Real(bEnable));
				}
			}
		}
		
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::loadDefsFromFile(const std::string& file)
{
	try
	{
		mFile.load(file, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, 
			"\t:=", /* seperator */
			true    /* trimWhitespace */
		);
	}
	catch (Ogre::Exception&) 
	{
		LogO("[MaterialFactory] WARNING: could not load file " + file);
		
		return;
	}
	
	int defI = 0; // counter for num def's.
	
	// go through all sections (in this case mat def's) in this file
	ConfigFile::SectionIterator seci = mFile.getSectionIterator();
	String secName, key, value;

	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		if (secName == StringUtil::BLANK) { seci.getNext(); continue; }
		
		MaterialProperties* newProps;

		// now we check if material definition has a parent.
		
		// the parent attribute allows you to base a material
		// definition upon one that you already have, to eliminate
		// redundant code.
		
		// note that this has one implication, you have to specify
		// the parent material definition *before* specifying the
		// material definition that's based upon it.
		
		// it's important to note that ogre's ConfigFile loads sections
		// in alphabetical order, so really the only good option is to
		// have derived material definitions in seperate files.
		
		//!todo
		// parse "parent" attribute regardless of the material order in the file
		// (i.e. add materials that have a parent that isn't created yet into a queue, to process later)
		
		if (mFile.getSetting("parent", secName) != StringUtil::BLANK)
		{
			// has parent
			const std::string parent = mFile.getSetting("parent", secName);
			
			// look if we have this parent flying around somewhere
			std::vector<MaterialDefinition*>::iterator it;
			for (it=mDefinitions.begin(); it!=mDefinitions.end(); ++it)
			{
				if ( (*it)->getName() == parent) {
					break;
				}
			}
			
			if (it == mDefinitions.end()) {
				// parent not found
				LogO("[MaterialFactory] parent '" + parent + "' of material "
					"definition '" + secName + "' not found. Make sure you are"
					" declaring / loading in the right order.");
				seci.getNext();
				continue;
			}
			else
			{
				// parent found, we can base our new material definition upon this parent
				MaterialProperties* props = (*it)->getProps();
				newProps = new MaterialProperties(*props); // use copy constructor
				newProps->abstract = false;
			}
		}
		else
		{
			// has no parent
			// this means we have no template for material definition, 
			// so just create an empty MaterialProperties
			newProps = new MaterialProperties();
		}
		
		// now, loop through all settings (=properties) in this section (=mat. definition)
		// and save the property from file in our MaterialProperties.
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
			newProps->setProperty(i->first, i->second);
		
		// ok, we have the MaterialProperties, now we can create MaterialDefinition based on it
		MaterialDefinition* newDef = new MaterialDefinition(this, newProps);
		newDef->setName(secName);
		mDefinitions.push_back(newDef);
		++defI;
	}
	
	LogO("[MaterialFactory] loaded " + toStr(defI) + " definitions from " + file);
}

//----------------------------------------------------------------------------------------

void MaterialFactory::setShaderParams(MaterialPtr mat)
{
	if (mat.isNull()) return;
		
	Material::TechniqueIterator techIt = mat->getTechniqueIterator();
	while (techIt.hasMoreElements())
	{
		Technique* tech = techIt.getNext();
		Technique::PassIterator passIt = tech->getPassIterator();
		while (passIt.hasMoreElements())
		{
			Pass* pass = passIt.getNext();
								
			if (pass->hasFragmentProgram())
			{
				// shadow fading parameters
				if ( getShadowsFade()
					&& pass->getFragmentProgramParameters()->_findNamedConstantDefinition("fadeStart_farDist", false)
					&& mSceneMgr
				)
				{
					float fadeDist;
					if (mSceneMgr->getShadowFarDistance() != 0)
						fadeDist = getShadowsFadeDistance()/mSceneMgr->getShadowFarDistance();
					else
						fadeDist = 0.f;
														
					pass->getFragmentProgramParameters()->setNamedConstant("fadeStart_farDist", Vector3(
						fadeDist,
						mSceneMgr->getShadowFarDistance(),
						float(getShadowsFade())
					));
				}
				// terrain lightmap name & size
				if ( mTerrain && !mTerrain->getLightmap().isNull() )
				{
					if (pass->getFragmentProgramParameters()->_findNamedConstantDefinition("terrainWorldSize", false))
						pass->getFragmentProgramParameters()->setNamedConstant( "terrainWorldSize", Real( mTerrain->getWorldSize() ) );
					
					Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
					while (tusIt.hasMoreElements())
					{
						TextureUnitState* tus = tusIt.getNext();
						if (tus->getName() == "terrainLightMap")
						{
							tus->setTextureName( mTerrain->getLightmap()->getName() );
						}
					}
				}
				
				if (pass->getFragmentProgramParameters()->_findNamedConstantDefinition("invTerSize", false))
					pass->getFragmentProgramParameters()->setNamedConstant("invTerSize", 1.f / Real(pApp->sc.td.fTerWorldSize));

				// pssm split points
				if ( pass->getFragmentProgramParameters()->_findNamedConstantDefinition("pssmSplitPoints", false) && mPSSM)
				{
					const PSSMShadowCameraSetup::SplitPointList& splitPointList = mPSSM->getSplitPoints();
					Vector4 splitPoints;
					for (size_t i = 0; i < splitPointList.size(); ++i)
						splitPoints[i] = splitPointList[i];
						
					pass->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialFactory::generate()
{
	if (/*1||*/bSettingsChanged)
	{
		QTimer ti;  ti.update(); /// time
		LogO("[MaterialFactory] generating new materials...");
		
		deleteShaderCache();
		fogMtrs.clear();
		timeMtrs.clear();
		softMtrs.clear();
		
		for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
			it!=mDefinitions.end(); ++it)
		{
			// don't generate abstract materials
			if ((*it)->getProps()->abstract) continue;
						
			// find an appropriate generator
			MaterialGenerator* generator;
			if ((*it)->getProps()->customGenerator == "")
				generator = mGenerator; // default
			else
			{
				// iterate through custom generators
				std::vector<MaterialGenerator*>::iterator gIt;
				for (gIt = mCustomGenerators.begin(); gIt != mCustomGenerators.end(); ++gIt)
				{
					if ( (*gIt)->mName == (*it)->getProps()->customGenerator)
					{
						generator = (*gIt);
						break;
					}
				}
				if (gIt == mCustomGenerators.end())
				{
					LogO("[MaterialFactory] WARNING: Custom generator '" + (*it)->getProps()->customGenerator +
						"' referenced by material '" + (*it)->getName() + "' not found. Using default generator.");
					generator = mGenerator; 
				}
			}

			// shader cache - check if same shader already exists
			ShaderProperties* shaderProps = new ShaderProperties( (*it)->mProps, this );
			
			bool exists = false;
			shaderMap::iterator sit;
			for (sit = mShaderCache.begin();
				sit != mShaderCache.end(); ++sit)
			{
				if ( sit->second->isEqual( shaderProps ) )
				{
					exists = true;
					break;
				}
			}
			
			if (!exists)
				generator->mShaderCached = false;
			else
			{
				generator->mShaderCached = true;
				generator->mVertexProgram = sit->first.first;
				generator->mFragmentProgram = sit->first.second;
			}
						
			generator->mDef = (*it);
			generator->mShader = shaderProps;
			generator->generate();
			
			// insert into cache
			#ifdef USE_CACHE
			if (!exists)
			{
				if (!generator->mVertexProgram.isNull() && !generator->mFragmentProgram.isNull()) 
					mShaderCache[ std::make_pair(generator->mVertexProgram, generator->mFragmentProgram) ] = shaderProps;
				else
					delete shaderProps;
			}
			else
				delete shaderProps;
			#else
			delete shaderProps;
			#endif
		}
		
		bSettingsChanged = false;
		
		ti.update(); /// time
		float dt = ti.dt * 1000.f;
		LogO(String("::: Time MaterialFactory: ") + toStr(dt) + " ms");

		// recreate cloned car materials
		#ifndef ROAD_EDITOR
		pApp->recreateCarMtr();
		#endif
	}
	else
	{
		LogO("[MaterialFactory] settings not changed, using old materials");
	}
		
	
	// update params	
	for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
		it!=mDefinitions.end(); ++it)
	{
		MaterialPtr mat = MaterialManager::getSingleton().getByName( (*it)->getName() );
		setShaderParams(mat);
	}
	
	#ifdef USE_CUSTOM_TERRAIN_MATERIAL
	// update terrain params
	if (!mTerrain) return;
	MaterialPtr terrainMat = mTerrain->_getMaterial();
	if (!terrainMat.isNull())
	{
		Material::TechniqueIterator techIt = terrainMat->getTechniqueIterator();
		while (techIt.hasMoreElements())
		{
			Technique* tech = techIt.getNext();
			Technique::PassIterator passIt = tech->getPassIterator();
			while (passIt.hasMoreElements())
			{
				Pass* pass = passIt.getNext();
				
				if (!pass->hasFragmentProgram()
				 || !pass->getFragmentProgramParameters()->_findNamedConstantDefinition("fadeStart_farDist", false)) continue;
				
				float fadeDist;
				if (mSceneMgr->getShadowFarDistance() != 0)
					fadeDist = getShadowsFadeDistance()/mSceneMgr->getShadowFarDistance();
				else
					fadeDist = 0.f;
																
				pass->getFragmentProgramParameters()->setNamedConstant("fadeStart_farDist", Vector3(
					fadeDist,
					mSceneMgr->getShadowFarDistance(),
					float(getShadowsFade())
				));
			}
		}
	}
	#endif
}

//----------------------------------------------------------------------------------------

void MaterialFactory::update()
{
	for (std::vector<std::string>::const_iterator it = timeMtrs.begin();
		it != timeMtrs.end(); ++it)
	{
		MaterialPtr mtr = MaterialManager::getSingleton().getByName( (*it) );

		if (!mtr.isNull())
		{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
			while (techIt.hasMoreElements())
			{	Technique* tech = techIt.getNext();
				Technique::PassIterator passIt = tech->getPassIterator();
				while (passIt.hasMoreElements())
				{	Pass* pass = passIt.getNext();

					// time
					if (pass->hasFragmentProgram() && pass->getFragmentProgramParameters()->_findNamedConstantDefinition("time", false))
						pass->getFragmentProgramParameters()->setNamedConstantFromTime( "time", 1 );
				}
			}	
		}	
	}
}

//----------------------------------------------------------------------------------------
