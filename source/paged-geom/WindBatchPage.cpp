/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

//WindBatchPage.cpp
//WindBatchPage is an extension to PagedGeometry which displays entities as static geometry but that is affected by wind.
//-------------------------------------------------------------------------------------
#include "pch.h"
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreEntity.h>
#include <OgreRenderSystem.h>
#include <OgreRenderSystemCapabilities.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>

#include "WindBatchPage.h"
#include "WindBatchedGeometry.h"

// to dump the shader source in a file
#include <fstream>

using namespace Ogre;
using namespace Forests;

//-------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
///
void WindBatchPage::init(PagedGeometry *geom, const Any &data)
{
   int datacast = !data.isEmpty() ? Ogre::any_cast<int>(data) : 0;
#ifdef _DEBUG
	if (datacast < 0)
   {
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
         "Data of WindBatchPage must be a positive integer. It representing the LOD level this detail level stores.",
         "WindBatchPage::WindBatchPage");
   }
#endif

   m_pBatchGeom   = new WindBatchedGeometry(geom->getSceneManager(), geom->getSceneNode(), geom);
	m_nLODLevel    = datacast; 
	m_pPagedGeom   = geom;
	m_bFadeEnabled = false;

	const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
   m_bShadersSupported = caps->hasCapability(RSC_VERTEX_PROGRAM) ? true : false;    // <-- DELETE THIS

   ++s_nRefCount;
}


//-----------------------------------------------------------------------------
///
void WindBatchPage::_updateShaders()
{
	if (!m_bShadersSupported)
		return;

	unsigned int i = 0;
	BatchedGeometry::TSubBatchIterator it = m_pBatchGeom->getSubBatchIterator();
	while (it.hasMoreElements())
   {
      BatchedGeometry::SubBatch *subBatch = it.getNext();
		const MaterialPtr &ptrMat = m_vecUnfadedMaterials[i++];

		//Check if lighting should be enabled
		bool lightingEnabled = false;
		for (unsigned short t = 0, techCnt = ptrMat->getNumTechniques(); t < techCnt; ++t)
      {
			Technique *tech = ptrMat->getTechnique(t);
			for (unsigned short p = 0, passCnt = tech->getNumPasses(); p < passCnt; ++p)
         {
            if (tech->getPass(p)->getLightingEnabled())
            {
					lightingEnabled = true;
					break;
				}
			}

			if (lightingEnabled)
            break;
		}

		//Compile the shader script based on various material / fade options
		StringUtil::StrStreamType tmpName;
		tmpName << "BatchPage_";
		if (m_bFadeEnabled)
			tmpName << "fade_";
		if (lightingEnabled)
			tmpName << "lit_";
		if (subBatch->m_pVertexData->vertexDeclaration->findElementBySemantic(VES_DIFFUSE) != NULL)
			tmpName << "clr_";

		for (unsigned short i = 0; i < subBatch->m_pVertexData->vertexDeclaration->getElementCount(); ++i)
      {
			const VertexElement *el = subBatch->m_pVertexData->vertexDeclaration->getElement(i);
			if (el->getSemantic() == VES_TEXTURE_COORDINATES)
         {
				String uvType;
            switch (el->getType())
            {
            case VET_FLOAT1: uvType = "1"; break;
            case VET_FLOAT2: uvType = "2"; break;
            case VET_FLOAT3: uvType = "3"; break;
            case VET_FLOAT4: uvType = "4"; break;
            }
            tmpName << uvType << '_';
			}
		}

		tmpName << "vp";

		const String vertexProgName = tmpName.str();

		String shaderLanguage;
		if (Root::getSingleton().getRenderSystem()->getName() == "Direct3D9 Rendering Subsystem")
			shaderLanguage = "hlsl";
		else if(Root::getSingleton().getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
			shaderLanguage = "glsl";
		else
			shaderLanguage = "cg";

		///T removed code for shader creation (we have our own shader)


		//Now that the shader is ready to be applied, apply it
		StringUtil::StrStreamType materialSignature;
		materialSignature << "BatchMat|";
		materialSignature << ptrMat->getName() << "|";
		if (m_bFadeEnabled)
      {
			materialSignature << m_fVisibleDist << "|";
			materialSignature << m_fInvisibleDist << "|";
		}

		//Search for the desired material
		MaterialPtr generatedMaterial = MaterialManager::getSingleton().getByName(materialSignature.str());
		if (generatedMaterial.isNull())
      {
			//Clone the material
			generatedMaterial = ptrMat->clone(materialSignature.str());

			//And apply the fade shader
			for (unsigned short t = 0; t < generatedMaterial->getNumTechniques(); ++t){
				Technique *tech = generatedMaterial->getTechnique(t);
				for (unsigned short p = 0; p < tech->getNumPasses(); ++p){
					Pass *pass = tech->getPass(p);

					//Setup vertex program
					if (pass->getVertexProgramName() == "")
						pass->setVertexProgram(vertexProgName);

					try{
						GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
						
						///T removed a lot of params that are not needed in our own shader

						if(params->_findNamedConstantDefinition("time",false))
							params->setNamedConstantFromTime("time", 1);
					}
					catch (const Ogre::Exception &e)
					{
						///T don't create exception.log
					}
					catch (...)
               {
						OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                     "Error configuring batched geometry transitions. If you're using materials with custom\
                     vertex shaders, they will need to implement fade transitions to be compatible with BatchPage.",
                     "BatchPage::_updateShaders()");
					}
				}
			}

		}

		//Apply the material
		subBatch->setMaterial(generatedMaterial);
	}

}
