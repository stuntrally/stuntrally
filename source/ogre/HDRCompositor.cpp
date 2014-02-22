/**
* This source file is part of HDRlib
* an addon for OGRE (Object-oriented Graphics Rendering Engine)
* For the latest info, see http://www.ogre3d.org/
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.

* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place - Suite 330, Boston, MA 02111-1307, USA, or go to
* http://www.gnu.org/copyleft/lesser.txt.
*
* @author	Christian Luksch
* @see		Readme.txt
*/
#include "pch.h"
#include "HDRCompositor.h"
#include "Ogre.h"


HDRCompositor::HDRCompositor(BaseApp *app) : m_Scales(7)
{
//	m_Window = win;
//	m_Cam = cam;
	mApp = app;

	m_Compositor.setNull();
	m_HDRTechnique = NULL;

	m_ToneMapper = TM_NONE;
	m_GlareType = GT_NONE;
	m_GlareStrength = 1.f;
	m_StarType = ST_NONE;
	m_StarStrength = 1.f;
	m_AutoKeying = false;
	m_LumAdaption = false;
	m_Key = 0.2f;
	m_AdaptationScale = 1;
	m_DebugRendertarget = 0;
	m_BrightPassOffset = 1.0f;
	m_BrightPassThresshold = 3.0f;
	m_E = 0.55;
	m_Phi = 8;
	m_GlarePasses = 1;
	m_StarPasses = 1;
	
	m_FrameTime = 1/60.f;

}

//called once on material setup
void HDRCompositor::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{	
	if(pass_id == MID_DOWN || pass_id == MID_LUMSCALE2)
	{
		//GpuProgramParameters::RealConstantEntry* entry;
		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("rcpScreenSize");
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 0, 1.f/m_VpWidth);
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 1, 1.f/m_VpHeight);
	}
	else if(pass_id == MID_LUMSCALE4)
	{
		//GpuProgramParameters::RealConstantEntry* entry;
		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("rcpScreenSize");
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 0, 2.f/m_VpWidth);
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 1, 2.f/m_VpHeight);
	}
	else if(pass_id == MID_LUMSCALE8)
	{
		//GpuProgramParameters::RealConstantEntry* entry;
		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("rcpScreenSize");
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 0, 4.f/m_VpWidth);
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def.physicalIndex + 1, 4.f/m_VpHeight);
	}
	else if(pass_id == MID_GAUSSBLUR)
	{
		float gs[4*13];

		BuildGaussGlare(gs, 2.5, 1, 4);

		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("GaussStuff[0]");
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstants(def.physicalIndex, gs, 13*4);
	}
	else if(pass_id == MID_STARH)
	{
		float gs[4*13];

		BuildGaussStarH(gs,5,3);

		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("GaussStuff[0]");
		
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstants(def.physicalIndex, gs, 13*4);
	}
	else if(pass_id == MID_STARV)
	{
		float gs[4*13];

		BuildGaussStarV(gs,5,3);

		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("GaussStuff[0]");
		
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstants(def.physicalIndex, gs, 13*4);
	}
	else if(pass_id >= MID_SCALE)
	{
		float gs[4*13];

		int scale = pass_id - MID_SCALE;
		int scaleSize = (1 << ((scale + 1) / 2));

		float s = 1.f / 2.f / sqrt(2.f);
		s *= pow(1.6, scale + 1);
		//s /= scaleSize;

		//scale approxs usedown
		// 1    0.356		1 
		// 2    0.565		2
		// 3    0.91		2
		// 4	1.448		4
		// 5	2.317		4
		// 6	3.707		8
		// 7	5.932		8

		BuildGaussGlare(gs, s, 1, scaleSize);

		const Ogre::GpuConstantDefinition& def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getConstantDefinition("GaussStuff[0]");
		mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstants(def.physicalIndex, gs, 13*4);
	}
}

void HDRCompositor::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
	const Ogre::GpuConstantDefinition* def;

	switch(pass_id)
	{
	case MID_KEY:	def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("Key");
					if(def)
						mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_Key);
					break;

	case MID_BRIGHT:	def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("BRIGHT_PASS_THRESHOLD");
						if(def)
							mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_BrightPassThresshold);

						def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("BRIGHT_PASS_OFFSET");
						if(def)
							mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_BrightPassOffset);
						
						break;

	case MID_ADAPT: def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("dTime");
					if(def)
						mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_FrameTime);

					def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("AdaptationScale");
					if(def)
						mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_AdaptationScale);
					break;

	case MID_FINAL:if(m_ToneMapper != TM_NONE && mat->getTechnique(0)->getPass(0)->hasFragmentProgram())					
					{
						def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("GlareStrength");
						if(def)
						{
							if(m_GlareType != GT_NONE)
								mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_GlareStrength);
							else
								mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, 0.f);
						}
						def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("StarStrength");
						if(def)
						{
							if(m_StarType != ST_NONE)
								mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_StarStrength);
							else
								mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, 0.f);
						}
					}
					break;
	case MID_BUILDLOCAL:
					def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("e");
					if(def)
					{
						mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_E);
					}
					def = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_findNamedConstantDefinition("phi");
					if(def)
					{
						mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->_writeRawConstant(def->physicalIndex, m_Phi);
					}
					break;

	}
}

void HDRCompositor::notifyViewportSize(int width, int height)
{
	m_VpWidth = width;
	m_VpHeight = height;
}

String HDRCompositor::ToneMapperToString(const TONEMAPPER ToneMapper)
{
	switch(ToneMapper)
	{
	case TM_LINEAR: return "Linear"; 
	case TM_REINHARDS: return "Reinhard's"; 
	case TM_REINHARDSMOD: return "Modified Reinhard's";
	case TM_LOG: return "Logarithmic";
	case TM_ADAPTLOG: return "AdaptiveLog";
	case TM_REINHARDLOCAL: return "Reinhard(Local)";
	default:return "None";
	}
}

String HDRCompositor::GlareTypeToString(const GLARETYPE GlareType)
{
	switch(GlareType)
	{
	case GT_BLUR: return "Blur";
	default:return "None";
	}
}

String HDRCompositor::StarTypeToString(const STARTYPE StarType)
{
	switch(StarType)
	{
	case ST_PLUS: return "Plus";
	case ST_CROSS: return "Cross";
	case ST_PLUSCROSS: return "Plus & Cross";
	default:return "None";
	}
}

CompositorPtr HDRCompositor::Create(void)
{

	m_Compositor = CompositorManager::getSingleton().create("HDR", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	
	m_HDRTechnique = m_Compositor->createTechnique();

	m_HDRTechnique->setCompositorLogicName("HDR");
	CompositionTechnique::TextureDefinition *texdef = m_HDRTechnique->createTextureDefinition("Scene");
	texdef->refTexName = "mrt_output";
	texdef->refCompName="gbuffer";
//	texdef->scope=Ogre::CompositionTechnique::TextureScope::TS_GLOBAL;
	//render scene
//	CreateTextureDef("Scene",0,0,PF_FLOAT16_RGB);
		
//	CompositionTargetPass *tp = m_HDRTechnique->createTargetPass();
//	tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);

//	tp->setOutputName("Scene");

	RenderDownSample();

    CalculateLuminance();
	
	CalculateKey();

	if(m_GlareType || m_StarType)
	{
		BrightPass();
	
		if(m_GlareType)
			BuildGlare();

		if(m_StarType)
			BuildStar();
	}

	if(m_ToneMapper == TM_REINHARDLOCAL)
		BuildScales();

	FinalRendering();

	return m_Compositor;
}

void HDRCompositor::CreateTextureDef(const String name,const unsigned int width,const unsigned int height,const Ogre::PixelFormat format)
{
	CompositionTechnique::TextureDefinition *texdef = m_HDRTechnique->createTextureDefinition(name);
	texdef->width = width;
	texdef->height = height;
	texdef->formatList.push_back(format);
}

void HDRCompositor::BrightPass(void)
{		
	CompositionTargetPass* tp;
	CompositionPass* pass;

	//first do brightpass
	CreateTextureDef("Bright",m_VpWidth/4,m_VpHeight/4,PF_A8R8G8B8);
	
	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Bright"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("BrightPass");
	pass->setInput(0,"scenedown4");
	pass->setInput(1,"Luminance");
	pass->setInput(2,"Key");

	pass->setIdentifier(MID_BRIGHT);

	/*GpuProgramParameters::RealConstantEntry* entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("g_fMiddleGray");
	if(entry)
		entry->val[0] = m_Key;*/

	//downsamples of bright for glare and/or star
	/*if(m_GlareType != GT_NONE || m_StarType != ST_NONE)
	{
		CreateTextureDef("Bright2",m_Window->getWidth()/8,m_Window->getHeight()/8,PF_A8R8G8B8);
		//CreateTextureDef("Bright4",m_Window->getWidth()/16,m_Window->getHeight()/16,PF_A8R8G8B8);
		//CreateTextureDef("Bright8",m_Window->getWidth()/32,m_Window->getHeight()/32,PF_A8R8G8B8);

		//birght/2
		tp = m_HDRTechnique->createTargetPass();
		tp->setInputMode(CompositionTargetPass::IM_NONE);
		tp->setOutputName("Bright2");
		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("BrightDown_1");
		pass->setInput(0,"Bright");

		entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		if(entry)
		{
			entry->val[0] = 4.f/m_Window->getWidth();
			entry->val[1] = 4.f/m_Window->getHeight();
		}

		//birght/4
		//tp = m_HDRTechnique->createTargetPass();
		//tp->setInputMode(CompositionTargetPass::IM_NONE);
		//tp->setOutputName("Bright4");
		//pass = tp->createPass();
		//pass->setType(CompositionPass::PT_RENDERQUAD);
		//pass->setMaterialName("BrightDown_2");
		//pass->setInput(0,"Bright2");

		//entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		//if(entry)
		//{
		//	entry->val[0] = 8.f/m_Window->getWidth();
		//	entry->val[1] = 8.f/m_Window->getHeight();
		//}

		//birght/8
		//tp = m_HDRTechnique->createTargetPass();
		//tp->setInputMode(CompositionTargetPass::IM_NONE);
		//tp->setOutputName("Bright8");
		//pass = tp->createPass();
		//pass->setType(CompositionPass::PT_RENDERQUAD);
		//pass->setMaterialName("BrightDown_3");
		//pass->setInput(0,"Bright4");

		//entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		//if(entry)
		//{
		//	entry->val[0] = 16.f/m_Window->getWidth();
		//	entry->val[1] = 16.f/m_Window->getHeight();
		//}
	}*/
}

void HDRCompositor::RenderDownSample(void)
{
	CompositionTargetPass* tp;
	CompositionPass* pass;

	//Downscale4
	CreateTextureDef("scenedown4",m_VpWidth/4,m_VpHeight/4,PF_FLOAT16_RGB);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("scenedown4"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Down4");
	pass->setInput(0,"Scene",0);
	
	pass->setIdentifier(MID_DOWN);

//	GpuProgramParameters::RealConstantEntry* entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
//	entry->val[0] = 1.f/m_Window->getWidth();
//	entry->val[1] = 1.f/m_Window->getHeight();
}

void HDRCompositor::CalculateLuminance(void)
{
	CreateTextureDef("Lum64",64,64,PF_FLOAT16_R);
	CreateTextureDef("Lum16",16,16,PF_FLOAT16_R);
	CreateTextureDef("Lum4",4,4,PF_FLOAT16_R);
	CreateTextureDef("Luminance",1,1,PF_FLOAT16_R);

	CompositionTargetPass *tp;
	CompositionPass *pass;

	//intial to 64
	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lum64");

	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("LuminanceInitial");
	pass->setInput(0,"scenedown4");

	//down to 16
	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lum16");

	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Luminance1");
	pass->setInput(0,"Lum64");
		
	//down to 4	
	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lum4");
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Luminance2");
	pass->setInput(0,"Lum64");
	
	//final to 1
	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Luminance");
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	//if(m_ToneMapper != TM_ADAPTLOG)
		pass->setMaterialName("LuminanceFinalAvg");
	//else
	//	pass->setMaterialName("LuminanceFinalMax");
	pass->setInput(0,"Lum4");

	if(m_LumAdaption)
	{
		CreateTextureDef("AdaptedLumLast",1,1,PF_FLOAT16_R);

		tp = m_HDRTechnique->createTargetPass();
		tp->setInputMode(CompositionTargetPass::IM_NONE);
		tp->setOutputName("AdaptedLumLast");
					
		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("CopyHDR");
		pass->setInput(0,"AdaptedLum");

		CreateTextureDef("AdaptedLum",1,1,PF_FLOAT16_R);

		tp = m_HDRTechnique->createTargetPass();
		tp->setInputMode(CompositionTargetPass::IM_NONE);
		tp->setOutputName("AdaptedLum");
					
		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("AdaptLuminance");
		pass->setInput(0,"Luminance");
		pass->setInput(1,"AdaptedLumLast");//old
		pass->setIdentifier(MID_ADAPT);
	}
}

void HDRCompositor::BuildScales(void)
{
	CompositionTargetPass *tp;
	CompositionPass *pass;

	//first build luminacepass scales

	CreateTextureDef("Lumscale1",m_VpWidth,m_VpHeight,PF_FLOAT16_RGB);
	CreateTextureDef("Lumscale2",m_VpWidth/2,m_VpHeight/2,PF_FLOAT16_RGB);
	CreateTextureDef("Lumscale4",m_VpWidth/4,m_VpHeight/4,PF_FLOAT16_RGB);
	CreateTextureDef("Lumscale8",m_VpWidth/8,m_VpHeight/8,PF_FLOAT16_RGB);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lumscale1"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("LuminanceScale");
	pass->setInput(0,"Scene",0);
	if(m_LumAdaption)
		pass->setInput(1, "AdaptedLum");
	else
		pass->setInput(1, "Luminance");
	pass->setInput(2, "Key");
	
	pass->setIdentifier(MID_LUMSCALE1);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lumscale2"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Down2");
	pass->setInput(0,"Lumscale1");
	
	pass->setIdentifier(MID_LUMSCALE2);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lumscale4"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Down2");
	pass->setInput(0,"Lumscale2");
	
	pass->setIdentifier(MID_LUMSCALE4);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Lumscale8"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("Down2");
	pass->setInput(0,"Lumscale4");
	
	pass->setIdentifier(MID_LUMSCALE8);

	//filter downsampled scenes, each filter scale is 1.6 times larger
	//starting with:
	float s = 1.f / 2.f / sqrt(2.f);
	//sample scales 1, 2, 2, 4, 4, 8, 8 and build a total of 7 scales
	
	for(int i = 0; i < m_Scales; i++)
	{
		String strScale = "scale";
		strScale += StringConverter::toString(i+1);

		int scaleSize = (1 << ((i + 1) / 2));

		String strLumScale = "Lumscale";
		strLumScale += StringConverter::toString(scaleSize);

		CreateTextureDef(strScale, m_VpWidth/scaleSize, m_VpHeight/scaleSize, PF_FLOAT16_RGB);

		tp = m_HDRTechnique->createTargetPass();
		tp->setInputMode(CompositionTargetPass::IM_NONE);
		tp->setOutputName(strScale);
	
		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("GaussBlur");
		pass->setInput(0,strLumScale);
		pass->setIdentifier(MID_SCALE + i);
	}

	//build local adapt map
	CreateTextureDef("LocalAdaptMap", m_VpWidth, m_VpHeight, PF_FLOAT16_RGB);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("LocalAdaptMap"); 
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	pass->setMaterialName("BuildLocalAdaptMap");
	pass->setInput(0,"Key");
	pass->setInput(1,"scale1");
	pass->setInput(2,"scale2");
	pass->setInput(3,"scale3");
	pass->setInput(4,"scale4");
	pass->setInput(5,"scale5");
	pass->setInput(6,"scale6");
	pass->setInput(7,"scale7");
	
	pass->setIdentifier(MID_BUILDLOCAL);
}

void HDRCompositor::BuildGlare(void)
{
	if(m_GlareType == GT_BLUR)
	{
		CompositionTargetPass *tp;
		CompositionPass *pass;

		String strDst;
		String strSrc = "Bright";

		for(int i=0; i<m_GlarePasses; i++)
		{
			String strDst = "Glare";
			
			if(i < m_GlarePasses-1 )
				strDst += StringConverter::toString(i);
						
			CreateTextureDef(strDst,m_VpWidth/4,m_VpHeight/4,PF_A8R8G8B8);
		
			tp = m_HDRTechnique->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName(strDst);
		
			pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("GaussBlur");
			pass->setInput(0,strSrc);
			pass->setIdentifier(MID_GAUSSBLUR);

			strSrc = strDst;
		}

		//entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		//if(entry)
		//{
		//	entry->val[0] = 32.f/m_Window->getWidth();
		//	entry->val[1] = 32.f/m_Window->getHeight();
		//}

		/*pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("GaussBlur_2");
		pass->setInput(0,"Glare");
		pass->setInput(1,"Bright2");

		entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		if(entry)
		{
			entry->val[0] = 8.f/m_Window->getWidth();
			entry->val[1] = 8.f/m_Window->getHeight();
		}

		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("GaussBlur_3");
		pass->setInput(0,"Glare");
		pass->setInput(1,"Bright4");

		entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		if(entry)
		{
			entry->val[0] = 16.f/m_Window->getWidth();
			entry->val[1] = 16.f/m_Window->getHeight();
		}

		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("GaussBlur_4");
		pass->setInput(0,"Glare");
		pass->setInput(1,"Bright2");
		
		entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("rcpScreenSize");
		if(entry)
		{
			entry->val[0] = 32.f/m_Window->getWidth();
			entry->val[1] = 32.f/m_Window->getHeight();
		}*/
	}
}

void HDRCompositor::BuildStar(void)
{
	if(m_StarType == ST_PLUS)
	{
		CompositionTargetPass *tp;
		CompositionPass *pass;

		String strDstH, strDstV;
		String strSrcH = "Bright";
		String strSrcV = "Bright";

		for(int i=0; i<m_StarPasses; i++)
		{
			String strDstH = "StarH";
			String strDstV = "StarV";
			
			if(i < m_StarPasses-1 )
			{
				strDstH += StringConverter::toString(i);
				strDstV += StringConverter::toString(i);
			}
						
			CreateTextureDef(strDstH,m_VpWidth/4,m_VpHeight/4,PF_A8R8G8B8);
					
			tp = m_HDRTechnique->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName(strDstH);
		
			pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("GaussBlur");
			pass->setInput(0,strSrcH);
			pass->setIdentifier(MID_STARH);

			strSrcH = strDstH;

			CreateTextureDef(strDstV,m_VpWidth/4,m_VpHeight/4,PF_A8R8G8B8);

			tp = m_HDRTechnique->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName(strDstV);
		
			pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("GaussBlur");
			pass->setInput(0,strSrcV);
			pass->setIdentifier(MID_STARV);

			strSrcV = strDstV;
		}

		CreateTextureDef("Star",m_VpWidth/4,m_VpHeight/4,PF_A8R8G8B8);

		//merge horizontal and vertical blur
		tp = m_HDRTechnique->createTargetPass();
		tp->setInputMode(CompositionTargetPass::IM_NONE);
		tp->setOutputName("Star");
		
		pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERQUAD);
		pass->setMaterialName("Merge2");
		pass->setInput(0,"StarH");
		pass->setInput(1,"StarV");
	}
}

void HDRCompositor::CalculateKey(void)
{
	CompositionTargetPass *tp;
	CompositionPass *pass;

	CreateTextureDef("Key",1,1,PF_FLOAT16_RGB);

	tp = m_HDRTechnique->createTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	tp->setOutputName("Key");
	
	pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);
	if(m_AutoKeying)
	{
		pass->setMaterialName("AutoKey");
		pass->setInput(0,"Luminance");
	}
	else
	{
		pass->setIdentifier(MID_KEY);
		pass->setMaterialName("ManualKey");
//		GpuProgramParameters::RealConstantEntry* entry = pass->getMaterial()->getTechnique(0)->getPass(0)->getFragmentProgramParameters()->getNamedRealConstantEntry("Key");
//		if(entry)
//			entry->val[0] = m_Key;
	}
	
}

void HDRCompositor::FinalRendering(void)
{
	CompositionTargetPass *tp = m_HDRTechnique->getOutputTargetPass();
	tp->setInputMode(CompositionTargetPass::IM_NONE);
	
	CompositionPass *pass = tp->createPass();
	pass->setType(CompositionPass::PT_RENDERQUAD);

	if(m_ToneMapper == TM_NONE || m_DebugRendertarget)
        pass->setMaterialName("CompositorCopyback");
	else if(m_ToneMapper == TM_LINEAR)
		pass->setMaterialName("HDRFinalLinear");
	else if(m_ToneMapper == TM_REINHARDS)
		pass->setMaterialName("HDRFinalReinh");
	else if(m_ToneMapper == TM_REINHARDSMOD)
		pass->setMaterialName("HDRFinalReinhMod");
	else if(m_ToneMapper == TM_LOG)
		pass->setMaterialName("HDRFinalLog");
	else if(m_ToneMapper == TM_ADAPTLOG)
		pass->setMaterialName("HDRFinalAdaptiveLog");
	else if(m_ToneMapper == TM_REINHARDLOCAL)
		pass->setMaterialName("HDRFinalReinhLocal");

	if(m_DebugRendertarget == 1)
		pass->setInput(0,"scenedown4");
	else if(m_DebugRendertarget == 2)
		pass->setInput(0,"Lumscale1");
	else if(m_DebugRendertarget == 3)
		pass->setInput(0,"Lumscale2");
	else if(m_DebugRendertarget == 4)
		pass->setInput(0,"Lumscale4");
	else if(m_DebugRendertarget == 5)
		pass->setInput(0,"scale0");
	else if(m_DebugRendertarget == 6)
		pass->setInput(0,"scale1");
	else if(m_DebugRendertarget == 7)
		pass->setInput(0,"scale2");
	else if(m_DebugRendertarget == 8)
		pass->setInput(0,"scale6");
	/*else if(m_DebugRendertarget == 2)
		pass->setInput(0,"Lum64");
	else if(m_DebugRendertarget == 3)
		pass->setInput(0,"Lum16");
	else if(m_DebugRendertarget == 4)
		pass->setInput(0,"Lum4");
	else if(m_DebugRendertarget == 5)
		pass->setInput(0,"Luminance");
	else if(m_DebugRendertarget == 6)
		pass->setInput(0,"Bright");
	else if(m_DebugRendertarget == 7)
		pass->setInput(0,"Glare");
	else if(m_DebugRendertarget == 8)
		pass->setInput(0,"Star");*/
	else
	{
		pass->setInput(0,"Scene",0);

		if(m_ToneMapper != TM_NONE)
		{
			if(m_ToneMapper == TM_REINHARDLOCAL)
			{
				pass->setInput(0, "Lumscale1");

				pass->setInput(1,"Key");

				if(m_GlareType != GT_NONE)
					pass->setInput(2,"Glare");
				if(m_StarType != ST_NONE)
					pass->setInput(3,"Star");

				pass->setInput(4, "LocalAdaptMap");
			}
			else
			{
				if(m_LumAdaption)
					pass->setInput(1, "AdaptedLum");
				else
					pass->setInput(1, "Luminance");
				pass->setInput(2, "Key");
			}
		}

		if(m_ToneMapper != TM_REINHARDLOCAL)
		{
			if(m_GlareType != GT_NONE)
				pass->setInput(3,"Glare");

			if(m_StarType != ST_NONE)
				pass->setInput(4,"Star");
		}
	}

	pass->setIdentifier(MID_FINAL);
}

void HDRCompositor::BuildGaussGlare(float* out,float rho,float strength, float vpScale)
{
	float tu = vpScale / (float)m_VpWidth;
	float tv = vpScale / (float)m_VpHeight;

	float totalWeight = 0.0f;
	int index=0;

	float prae = 1/(2*3.14159265*(rho*rho));

	for( int x = -2; x <= 2; x++ )
	{
		for( int y = -2; y <= 2; y++ )
		{
			// Exclude pixels with a block distance greater than 2. This will
			// create a kernel which approximates a 5x5 kernel using only 13
			// sample points instead of 25; this is necessary since 2.0 shaders
			// only support 16 texture grabs.
			if( abs(x) + abs(y) > 2 )
				continue;

			// Get the unscaled Gaussian intensity for this offset
			out[index*4+0] = x * tu;
			out[index*4+1] = y * tv;
			out[index*4+2] = prae * pow(2.7182818028, -0.5*( ( x*x*vpScale*vpScale + y*y*vpScale*vpScale )/( rho*rho ) ) );
			totalWeight += out[index*4+2];
			out[index*4+3] = 0.f;

			index++;
		}
	}

	// Divide the current weight by the total weight of all the samples; Gaussian
	// blur kernels add to 1.0f to ensure that the intensity of the image isn't
	// changed when the blur occurs. An optional multiplier variable is used to
	// add or remove image intensity during the blur.
	for( int i=0; i < index; i++ )
		out[i*4+2] /= totalWeight/strength;
}

void HDRCompositor::BuildGaussStarH(float* out,float rho,float strength)
{
	float tu = 4.0f / (float)m_VpWidth;
	
	float totalWeight = 0.0f;
	int index=0;

	float prae = 1/(2*3.14159265*(rho*rho));

	for( int x = -6; x <= 6; x++ )
	{
		out[index*4+0] = x * tu;
		out[index*4+1] = 0;
		out[index*4+2] = prae * pow(2.7182818028, -0.5*( ( x*x )/( rho*rho ) ) );
		totalWeight += out[index*4+2];
		out[index*4+3] = 0.f;

		index++;
	}

	for( int i=0; i < index; i++ )
		out[i*4+2] /= totalWeight/strength;
}

void HDRCompositor::BuildGaussStarV(float* out,float rho,float strength)
{
	float tv = 4.0f / (float)m_VpHeight;
	
	float totalWeight = 0.0f;
	int index=0;

	float prae = 1/(2*3.14159265*(rho*rho));

	for( int y = -6; y <= 6; y++ )
	{
		out[index*4+0] = 0;
		out[index*4+1] = y * tv;
		out[index*4+2] = prae * pow(2.7182818028, -0.5*( ( y*y )/( rho*rho ) ) );
		totalWeight += out[index*4+2];
		out[index*4+3] = 0.f;

		index++;
	}

	for( int i=0; i < index; i++ )
		out[i*4+2] /= totalWeight/strength;
}

