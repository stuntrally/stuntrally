#include "pch.h"
#include "TerrainMaterial.h"

#include <OgreTerrain.h>

#include "../../shiny/Platforms/Ogre/OgreMaterial.hpp"
#include "../../shiny/Main/Factory.hpp"

namespace
{
	Ogre::String getComponent(int num)
	{
			if (num == 0)	return "x";
		else if (num == 1)	return "y";
		else if (num == 2)	return "z";
		else				return "w";
	}
}


TerrainMaterial::TerrainMaterial()
{
	mLayerDecl.samplers.push_back(Ogre::TerrainLayerSampler("albedo_specular", Ogre::PF_BYTE_RGBA));
	mLayerDecl.samplers.push_back(Ogre::TerrainLayerSampler("normal_height", Ogre::PF_BYTE_RGBA));

	mLayerDecl.elements.push_back(Ogre::TerrainLayerSamplerElement(0, Ogre::TLSS_ALBEDO, 0, 3));
	mLayerDecl.elements.push_back(Ogre::TerrainLayerSamplerElement(0, Ogre::TLSS_SPECULAR, 3, 1));
	mLayerDecl.elements.push_back(Ogre::TerrainLayerSamplerElement(1, Ogre::TLSS_NORMAL, 0, 3));
	mLayerDecl.elements.push_back(Ogre::TerrainLayerSamplerElement(1, Ogre::TLSS_HEIGHT, 3, 1));


	mProfiles.push_back(OGRE_NEW Profile(this, "SM2", "Profile for rendering on Shader Model 2 capable cards"));
	setActiveProfile("SM2");
}

// -----------------------------------------------------------------------------------------------------------------------

TerrainMaterial::Profile::Profile(Ogre::TerrainMaterialGenerator* parent, const Ogre::String& name, const Ogre::String& desc)
	: Ogre::TerrainMaterialGenerator::Profile(parent, name, desc)
{
}

TerrainMaterial::Profile::~Profile()
{
	sh::Factory::getInstance().destroyMaterialInstance(mMatName);
	sh::Factory::getInstance().destroyMaterialInstance(mMatNameComp);
}

void TerrainMaterial::Profile::createMaterial(const Ogre::String& matName, const Ogre::Terrain* terrain, bool renderCompositeMap)
{
	sh::Factory::getInstance().destroyMaterialInstance(matName);

	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(matName);
	if (!mat.isNull())
		Ogre::MaterialManager::getSingleton().remove(matName);

	mMaterial = sh::Factory::getInstance().createMaterialInstance(matName);

	if (!renderCompositeMap)
	{
		Ogre::MaterialPtr ogreMat = static_cast<sh::OgreMaterial*>(mMaterial->getMaterial())->getOgreMaterial();

		Ogre::Material::LodValueList list;
		list.push_back(Ogre::TerrainGlobalOptions::getSingleton().getCompositeMapDistance());
		ogreMat->setLodLevels(list);
	}

	mMaterial->setProperty("allow_fixed_function", sh::makeProperty<sh::BooleanValue>(new sh::BooleanValue(false)));

	sh::MaterialInstancePass* p = mMaterial->createPass();

	p->setProperty("vertex_program", sh::makeProperty<sh::StringValue>(new sh::StringValue("terrain_vertex")));
	p->setProperty("fragment_program", sh::makeProperty<sh::StringValue>(new sh::StringValue("terrain_fragment")));

	p->mShaderProperties.setProperty("composite_map", sh::makeProperty<sh::BooleanValue>(new sh::BooleanValue(renderCompositeMap)));


	// global normal map
	sh::MaterialInstanceTextureUnit* normalMap = p->createTextureUnit("normalMap");
	normalMap->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getTerrainNormalMap()->getName())));
	normalMap->setProperty("tex_address_mode", sh::makeProperty<sh::StringValue>(new sh::StringValue("clamp")));

	// light map
	sh::MaterialInstanceTextureUnit* lightMap = p->createTextureUnit("lightMap");
	lightMap->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getLightmap()->getName())));
	lightMap->setProperty("tex_address_mode", sh::makeProperty<sh::StringValue>(new sh::StringValue("clamp")));

	unsigned int maxLayers = getMaxLayers(terrain);
	unsigned int numBlendTextures = std::min(terrain->getBlendTextureCount(maxLayers), terrain->getBlendTextureCount());
	unsigned int numLayers = std::min(maxLayers, static_cast<unsigned int>(terrain->getLayerCount()));

	p->mShaderProperties.setProperty("num_layers", sh::makeProperty<sh::StringValue>(new sh::StringValue(Ogre::StringConverter::toString(numLayers))));
	p->mShaderProperties.setProperty("num_blendmaps", sh::makeProperty<sh::StringValue>(new sh::StringValue(Ogre::StringConverter::toString(numBlendTextures))));

	// blend maps
	for(int i = 0; i < numBlendTextures; ++i)
	{
		sh::MaterialInstanceTextureUnit* blendTex = p->createTextureUnit("blendMap" + Ogre::StringConverter::toString(i));
		blendTex->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getBlendTextureName(i))));
		blendTex->setProperty("tex_address_mode", sh::makeProperty<sh::StringValue>(new sh::StringValue("clamp")));
	}

	// layer diffuse
	for (int i = 0; i < numLayers; ++i)
	{
		sh::MaterialInstanceTextureUnit* diffuseTex = p->createTextureUnit("diffuseMap" + Ogre::StringConverter::toString(i));
		diffuseTex->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getLayerTextureName(i, 0))));
		p->mShaderProperties.setProperty("blendmap_component_" + Ogre::StringConverter::toString(i),
			sh::makeProperty<sh::StringValue>(new sh::StringValue(Ogre::StringConverter::toString(int((i-1) / 4)) + "." + getComponent(int((i-1) % 4)))));
	}

	// layer normalheight
	for (int i = 0; i < numLayers; ++i)
	{
		sh::MaterialInstanceTextureUnit* normalTex = p->createTextureUnit("normalMap" + Ogre::StringConverter::toString(i));
		normalTex->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getLayerTextureName(i, 1))));
	}

	// shadow
	if (!renderCompositeMap)
	{
		for (int i = 0; i < 3; ++i)
		{
			sh::MaterialInstanceTextureUnit* shadowTex = p->createTextureUnit("shadowMap" + Ogre::StringConverter::toString(i));
			shadowTex->setProperty("content_type", sh::makeProperty<sh::StringValue>(new sh::StringValue("shadow")));
		}
	}

	// composite map
	sh::MaterialInstanceTextureUnit* compositeMap = p->createTextureUnit("compositeMap");
	compositeMap->setProperty("direct_texture", sh::makeProperty<sh::StringValue>(new sh::StringValue(terrain->getCompositeMap()->getName())));

	// uv multipliers
	unsigned int numUVMul = numLayers / 4;
	if (numLayers % 4)
		++numUVMul;
	for (int i = 0; i < numUVMul; ++i)
	{
		sh::Vector4* uvMul = new sh::Vector4(
			terrain->getLayerUVMultiplier(i * 4),
			terrain->getLayerUVMultiplier(i * 4 + 1),
			terrain->getLayerUVMultiplier(i * 4 + 2),
			terrain->getLayerUVMultiplier(i * 4 + 3)
			);
		for (int j=0; j<4; ++j)
		{
			p->mShaderProperties.setProperty("uv_component_" + Ogre::StringConverter::toString(i*4+j), sh::makeProperty<sh::StringValue>(new sh::StringValue(
			Ogre::StringConverter::toString(i) + "." + getComponent(j) )));
		}
		p->mShaderProperties.setProperty("uv_mul_" + Ogre::StringConverter::toString(i), sh::makeProperty<sh::Vector4>(uvMul));
	}
	p->mShaderProperties.setProperty("num_uv_mul", sh::makeProperty<sh::StringValue>(new sh::StringValue(Ogre::StringConverter::toString(numUVMul))));

	p->mShaderProperties.setProperty("shadowtexture_offset", sh::makeProperty<sh::StringValue>(new sh::StringValue(
		Ogre::StringConverter::toString(0))));
}


Ogre::MaterialPtr TerrainMaterial::Profile::generate(const Ogre::Terrain* terrain)
{
	const Ogre::String& matName = terrain->getMaterialName();
	mMatName = matName;

	createMaterial(matName, terrain, false);

	return Ogre::MaterialManager::getSingleton().getByName(matName);
}

Ogre::MaterialPtr TerrainMaterial::Profile::generateForCompositeMap(const Ogre::Terrain* terrain)
{
	const Ogre::String& matName = terrain->getMaterialName()+"/comp";
	mMatNameComp = matName;

	createMaterial(matName, terrain, true);

	return Ogre::MaterialManager::getSingleton().getByName(matName);
}

Ogre::uint8 TerrainMaterial::Profile::getMaxLayers(const Ogre::Terrain* terrain) const
{
	// count the texture units free
	Ogre::uint8 freeTextureUnits = 16;
	--freeTextureUnits;  // normalmap
	--freeTextureUnits;  // colourmap
	--freeTextureUnits;  // lightmap

	freeTextureUnits -= 3; // shadow PSSM

	//--freeTextureUnits; // caustics

	// each layer needs 2.25 units(1xdiffusespec, 1xnormalheight, 0.25xblend)

	///  max is 4 layers,  more would need 2nd pass ...
	return static_cast<Ogre::uint8>(freeTextureUnits / 2.25f);
}

void TerrainMaterial::Profile::updateParams(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain)
{
}

void TerrainMaterial::Profile::updateParamsForCompositeMap(const Ogre::MaterialPtr& mat, const Ogre::Terrain* terrain)
{
}

void TerrainMaterial::Profile::requestOptions(Ogre::Terrain* terrain)
{
	terrain->_setMorphRequired(true);
	terrain->_setNormalMapRequired(true); // global normal map
	terrain->_setLightMapRequired(true, true);
	terrain->_setCompositeMapRequired(true);
}
