#include "BaseApp.h"
#include <Ogre.h>

void BaseApp::createCompositor()
{
	if (pSet->bloom)
	{
		//create bloom compositor
		Ogre::CompositorPtr comp = Ogre::CompositorManager::getSingleton().create(
				"Bloom", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			Ogre::CompositionTechnique *t = comp->createTechnique();
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
				def->width = 128;
				def->height = 128;
				def->formatList.push_back(Ogre::PF_A8R8G8B8);
			}
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt1");
				def->width = 128;
				def->height = 128;
				def->formatList.push_back(Ogre::PF_A8R8G8B8);
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("rt1");
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("rt0");
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Blur0");
				pass->setInput(0, "rt1");
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("rt1");
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Blur1");
				pass->setInput(0, "rt0");
			}
			{
				Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				{ Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/BloomBlend");
				pass->setInput(0, "rt1");
				}
			}
		}
		Ogre::CompositorManager::getSingleton().addCompositor(mViewport, "Bloom");
	
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Bloom", true);
	}
	else if (pSet->hdr)
	{
		; // TODO
	}
}
