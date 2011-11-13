#include "pch.h"
#include "../Defines.h"

#include "MaterialFactory.h"
#include "MaterialDefinition.h"
#include "MaterialGenerator.h"
#include "GlassMaterial.h"
#include "PipeGlassMaterial.h"
//#include "WaterMaterial.h"
#include "ShaderProperties.h"

#ifndef ROAD_EDITOR
	#include "../OgreGame.h"
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
using namespace Ogre;

//----------------------------------------------------------------------------------------

template<> MaterialFactory* Ogre::Singleton<MaterialFactory>::ms_Singleton = 0;
MaterialFactory* MaterialFactory::getSingletonPtr(void)
{
    return ms_Singleton;
}
MaterialFactory& MaterialFactory::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

//----------------------------------------------------------------------------------------

MaterialFactory::MaterialFactory() : 
	bShaders(1), bNormalMap(1), bEnvMap(1), bShadows(1), bShadowsDepth(1),
	iTexSize(4096), iNumShadowTex(3),
	bSettingsChanged(1) // always have to generate at start
{	
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
	
	//MaterialGenerator* water = static_cast<MaterialGenerator*>(new WaterMaterialGenerator());
	//water->mParent = this;
	//mCustomGenerators.push_back(water);
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

void MaterialFactory::loadDefsFromFile(const std::string& file)
{
	try
	{
		mFile.load(file, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, 
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
	Ogre::ConfigFile::SectionIterator seci = mFile.getSectionIterator();
	Ogre::String secName, key, value;

	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		if (secName == Ogre::StringUtil::BLANK) { seci.getNext(); continue; }
		
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
		
		if (mFile.getSetting("parent", secName) != Ogre::StringUtil::BLANK)
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
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
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

void MaterialFactory::generate()
{
	if (bSettingsChanged)
	{
		QTimer ti;  ti.update(); /// time
		LogO("[MaterialFactory] generating new materials...");
		
		deleteShaderCache();
		splitMtrs.clear();
		terrainLightMapMtrs.clear();
		
		for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
			it!=mDefinitions.end(); ++it)
		{
			// don't generate abstract materials
			if ((*it)->getProps()->abstract) continue;
			
			//LogO("generating " + (*it)->getName());
			
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
					LogO("[MaterialFactory] WARNING: Custom generator '" + (*it)->getProps()->customGenerator + "' \
referenced by material '" + (*it)->getName() + "' not found. Using default generator.");
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
			if (!exists)
			{
				if (!generator->mVertexProgram.isNull() && !generator->mFragmentProgram.isNull()) 
					mShaderCache[ std::make_pair(generator->mVertexProgram, generator->mFragmentProgram) ] = shaderProps;
			}
			else
				delete shaderProps;
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
		LogO("[MaterialFactory] settings not changed, using old materials");
		
}

//----------------------------------------------------------------------------------------

