#include "pch.h"
#include "Def_Str.h"
#include "TerrainMaterial.h"
#include "CScene.h"
#include "data/SceneXml.h"

#include <OgreTerrain.h>
#include <OgreMaterialManager.h>

#include "../../shiny/Platforms/Ogre/OgreMaterial.hpp"
#include "../../shiny/Main/Factory.hpp"
using namespace Ogre;


#define  STR(a)  sh::makeProperty<sh::StringValue>(new sh::StringValue( a ))


namespace
{
	String getComponent(int num)
	{
			if (num == 0)	return "x";
		else if (num == 1)	return "y";
		else if (num == 2)	return "z";
		else				return "w";
	}
}


TerrainMaterial::TerrainMaterial(CScene* scn1)
	: scn(scn1)
{
	mLayerDecl.samplers.push_back(TerrainLayerSampler("albedo_specular", PF_BYTE_RGBA));
	mLayerDecl.samplers.push_back(TerrainLayerSampler("normal_height", PF_BYTE_RGBA));

	mLayerDecl.elements.push_back(TerrainLayerSamplerElement(0, TLSS_ALBEDO, 0, 3));
	mLayerDecl.elements.push_back(TerrainLayerSamplerElement(0, TLSS_SPECULAR, 3, 1));
	mLayerDecl.elements.push_back(TerrainLayerSamplerElement(1, TLSS_NORMAL, 0, 3));
	mLayerDecl.elements.push_back(TerrainLayerSamplerElement(1, TLSS_HEIGHT, 3, 1));


	mProfiles.push_back(OGRE_NEW Profile(scn1, this, "SM2", "Profile for rendering on Shader Model 2 capable cards"));
	setActiveProfile("SM2");
}

// -----------------------------------------------------------------------------------------------------------------------

TerrainMaterial::Profile::Profile(CScene* scn1, TerrainMaterialGenerator* parent, const String& name, const String& desc)
	: TerrainMaterialGenerator::Profile(parent, name, desc), scn(scn1)
{
}

TerrainMaterial::Profile::~Profile()
{
	sh::Factory::getInstance().destroyMaterialInstance(mMatName);
	sh::Factory::getInstance().destroyMaterialInstance(mMatNameComp);
}

void TerrainMaterial::Profile::createMaterial(const String& matName, const Terrain* terrain, bool renderCompositeMap)
{
	sh::Factory::getInstance().destroyMaterialInstance(matName);

	MaterialPtr mat = MaterialManager::getSingleton().getByName(matName);
	if (mat)
		MaterialManager::getSingleton().remove(matName);

	mMaterial = sh::Factory::getInstance().createMaterialInstance(matName);

	if (!renderCompositeMap)
	{
		MaterialPtr ogreMat = static_cast<sh::OgreMaterial*>(mMaterial->getMaterial())->getOgreMaterial();

		Material::LodValueList list;
		list.push_back(TerrainGlobalOptions::getSingleton().getCompositeMapDistance());
		ogreMat->setLodLevels(list);
	}

	mMaterial->setProperty("allow_fixed_function", sh::makeProperty<sh::BooleanValue>(new sh::BooleanValue(false)));

	sh::MaterialInstancePass* p = mMaterial->createPass();

	p->setProperty("vertex_program",   STR("terrain_vertex"));
	p->setProperty("fragment_program", STR("terrain_fragment"));

	p->mShaderProperties.setProperty("composite_map", sh::makeProperty<sh::BooleanValue>(new sh::BooleanValue(renderCompositeMap)));

	typedef sh::MaterialInstanceTextureUnit* MatTex;

	//  global normal map ?-
	MatTex normalMap = p->createTextureUnit("normalMap");
	normalMap->setProperty("direct_texture",   STR(terrain->getTerrainNormalMap()->getName()));
	normalMap->setProperty("tex_address_mode", STR("clamp"));

	//  light map
	MatTex lightMap = p->createTextureUnit("lightMap");
	lightMap->setProperty("direct_texture",   STR(terrain->getLightmap()->getName()));
	lightMap->setProperty("tex_address_mode", STR("clamp"));

	uint maxLayers = getMaxLayers(terrain),
		numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount()),  // = 1
		numLayers = std::min(maxLayers, static_cast<uint>(terrain->getLayerCount()));

	p->mShaderProperties.setProperty("num_layers",    STR(toStr(numLayers)));
	p->mShaderProperties.setProperty("num_blendmaps", STR(toStr(numBlendTextures)));

	//  blend maps
	for (uint i = 0; i < numBlendTextures; ++i)
	{
		MatTex blendTex = p->createTextureUnit("blendMap" + toStr(i));
		blendTex->setProperty("direct_texture",   STR("blendmapRTT"));
		blendTex->setProperty("tex_address_mode", STR("clamp"));
	}

	//  layer diffuse+spec
	for (uint i = 0; i < numLayers; ++i)
	{
		MatTex diffuseTex = p->createTextureUnit("diffuseMap" + toStr(i));
		diffuseTex->setProperty("direct_texture", STR(terrain->getLayerTextureName(i, 0)));
		p->mShaderProperties.setProperty("blendmap_component_" + toStr(i),
			STR(toStr(i / 4) + "." + getComponent(i % 4)));
	}

	//  layer normal+height
	for (uint i = 0; i < numLayers; ++i)
	{
		MatTex normalTex = p->createTextureUnit("normalMap" + toStr(i));
		normalTex->setProperty("direct_texture", STR(terrain->getLayerTextureName(i, 1)));
	}

	//  shadow
	if (!renderCompositeMap)
	{
		for (int i = 0; i < 3; ++i)
		{
			MatTex shadowTex = p->createTextureUnit("shadowMap" + toStr(i));
			shadowTex->setProperty("content_type", STR("shadow"));
		}
	}

	//  composite map
	MatTex compositeMap = p->createTextureUnit("compositeMap");
	compositeMap->setProperty("direct_texture", STR(terrain->getCompositeMap()->getName()));

	//  uv multipliers ?-
	uint numUVMul = numLayers / 4;
	if (numLayers % 4)
		++numUVMul;
	for (int i = 0; i < numUVMul; ++i)
	{
		int ii = i * 4;
		sh::Vector4* uvMul = new sh::Vector4(
			terrain->getLayerUVMultiplier(ii),     terrain->getLayerUVMultiplier(ii + 1),
			terrain->getLayerUVMultiplier(ii + 2), terrain->getLayerUVMultiplier(ii + 3) );

		for (int j=0; j<4; ++j)
		{
			p->mShaderProperties.setProperty("uv_component_" + toStr(i*4+j), STR(
				toStr(i) + "." + getComponent(j) ));
		}
		p->mShaderProperties.setProperty("uv_mul_" + toStr(i), sh::makeProperty<sh::Vector4>(uvMul));
	}
	p->mShaderProperties.setProperty("num_uv_mul", STR(toStr(numUVMul)));

	p->mShaderProperties.setProperty("shadowtexture_offset", STR(toStr(0)));
}


MaterialPtr TerrainMaterial::Profile::generate(const Terrain* terrain)
{
	const String& matName = terrain->getMaterialName();
	mMatName = matName;

	createMaterial(matName, terrain, false);

	return MaterialManager::getSingleton().getByName(matName);
}

MaterialPtr TerrainMaterial::Profile::generateForCompositeMap(const Terrain* terrain)
{
	const String& matName = terrain->getMaterialName()+"/comp";
	mMatNameComp = matName;

	createMaterial(matName, terrain, true);

	return MaterialManager::getSingleton().getByName(matName);
}

uint8 TerrainMaterial::Profile::getMaxLayers(const Terrain* terrain) const
{
	// count the texture units free
	uint8 freeTextureUnits = 16;
	--freeTextureUnits;  // normalmap
	--freeTextureUnits;  // colourmap
	--freeTextureUnits;  // lightmap

	freeTextureUnits -= 3; // shadow PSSM

	//--freeTextureUnits; // caustics

	// each layer needs 2.25 units (1 diffusespec, 1 normalheight, 0.25 blend)

	///  max is 4 layers,  more would need 2nd pass ...
	return static_cast<uint8>(freeTextureUnits / 2.25f);
}

void TerrainMaterial::Profile::updateParams(const MaterialPtr& mat, const Terrain* terrain)
{
}

void TerrainMaterial::Profile::updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain)
{
}

void TerrainMaterial::Profile::requestOptions(Terrain* terrain)
{
	terrain->_setMorphRequired(true);
	terrain->_setNormalMapRequired(true); // global normal map
	terrain->_setLightMapRequired(true, true);
	terrain->_setCompositeMapRequired(true);
}
