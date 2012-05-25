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

#include <OgreCompositor.h>
#include <OgreCompositorManager.h>
#include <OgreCompositorChain.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionPass.h>
#include <OgreCompositionTargetPass.h>
#include <OgreRenderWindow.h>
#include <OgreCamera.h>

using namespace Ogre;
class BaseApp;
//#define NUMTONEMAPPERS 5
//#define NUMGLARETYPES 2
//#define NUMSTARTYPES 2

class HDRCompositor : public Ogre::CompositorInstance::Listener
{
public:

    enum TONEMAPPER
	{
		TM_NONE = 0,
		TM_LINEAR,
		TM_REINHARDS,
		TM_REINHARDSMOD,
		TM_LOG,
		TM_ADAPTLOG,
		TM_REINHARDLOCAL,
		TM_COUNT
	};

	enum GLARETYPE
	{
		GT_NONE = 0,
		GT_BLUR
	};

	enum STARTYPE
	{
		ST_NONE = 0,
		ST_PLUS,
		ST_CROSS,
		ST_PLUSCROSS
	};

	enum MATID
	{
		MID_KEY = 12345,
		MID_LUMSCALE1,
		MID_LUMSCALE2,
		MID_LUMSCALE4,
		MID_LUMSCALE8,
		MID_BUILDLOCAL,
		MID_DOWN,
		MID_BRIGHT,
		MID_GAUSSBLUR,
		MID_STARH,
		MID_STARV,
		MID_FINAL,
		MID_ADAPT,
		MID_SCALE //must be last		
	};

protected:

	TONEMAPPER	m_ToneMapper;
	GLARETYPE	m_GlareType;
	float		m_GlareStrength;
	int			m_GlarePasses;
	STARTYPE	m_StarType;
	float		m_StarStrength;
	int			m_StarPasses;
	bool		m_AutoKeying;
	float		m_Key;
	bool		m_LumAdaption;
	float		m_FrameTime;
	float		m_AdaptationScale;
	int			m_DebugRendertarget;
	float		m_BrightPassOffset;
	float		m_BrightPassThresshold;
	const int	m_Scales;
	float		m_E;
	float		m_Phi;

//	RenderWindow*	m_Window;
//	Camera*			m_Cam;

	CompositorPtr m_Compositor;
	CompositionTechnique *m_HDRTechnique;
	
	float		m_VpWidth,m_VpHeight;
	BaseApp * mApp;
	
public:

	HDRCompositor(BaseApp *app);
	~HDRCompositor(void) { }
	
//	void Release(void);

//	void Enable(const bool Enable);
//	bool IsEnabled(void) const { return m_Enable; }

	TONEMAPPER GetToneMapper(void) const { return m_ToneMapper; }
	void SetToneMapper(const TONEMAPPER ToneMapper) { m_ToneMapper = ToneMapper;  }

	GLARETYPE GetGlareType(void) const { return m_GlareType; }
	void SetGlareType(const GLARETYPE GlareType) { m_GlareType = GlareType;  }

	float GetGlareStrength(void) const { return m_GlareStrength; }
	void SetGlareStrength(const float Strength) { m_GlareStrength = Strength; }

	int GetGlarePasses(void) const {	return m_GlarePasses; }
	void SetGlarePasses(const float Passes) { m_GlarePasses = Passes;  }

	STARTYPE GetStarType(void) const { return m_StarType; }
	void SetStarType(const STARTYPE StarType) {	m_StarType = StarType;  }

	float GetStarStrength(void) const {	return m_StarStrength; }
	void SetStarStrength(const float Strength) { m_StarStrength = Strength; }

	int GetStarPasses(void) const {	return m_StarPasses; }
	void SetStarPasses(const float Passes) { m_StarPasses = Passes;  }

	bool GetAutoKeying(void) const { return m_AutoKeying; }
	void SetAutoKeying(const bool AutoKeying) {	m_AutoKeying = AutoKeying;  }

	bool GetLumAdaption(void) const { return m_LumAdaption; }
	void SetLumAdapdation(bool LumAdaptation) {	m_LumAdaption = LumAdaptation;  }

	float GetKey(void) const { return m_Key; }
	void SetKey(const float Key) { m_Key = Key; }

	float GetAdaptationScale(void) const {return m_AdaptationScale;}
	void SetAdaptationScale(const float Scale) {m_AdaptationScale = Scale;}

	float GetLocalE(void) const {	return m_E; }
	void SetLocalE(const float e) { m_E = e; }

	float GetLocalPhi(void) const {	return m_Phi; }
	void SetLocalPhi(const float phi) { m_Phi = phi; }
	
	void SetFrameTime(const float FrameTime) { m_FrameTime = FrameTime; }

	//called once on material setup
	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	//called every frame
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	virtual void notifyViewportSize(int width, int height);

	void DebugRendertarget(const int rt){if(rt != m_DebugRendertarget) { m_DebugRendertarget = rt;  }}

	static String ToneMapperToString(const TONEMAPPER ToneMapper);
	static String GlareTypeToString(const GLARETYPE GlareType);
	static String StarTypeToString(const STARTYPE StarType);
	CompositorPtr Create(void);

private:


	void CreateTextureDef(const String name,const unsigned int width,const unsigned int height,const Ogre::PixelFormat format);
	void BrightPass(void);
	void RenderDownSample(void);
	void CalculateLuminance(void);
	void BuildGlare(void);
	
	void BuildStar(void);

	void CalculateKey(void);

	void BuildScales(void);

	void FinalRendering(void);

	void BuildGaussGlare(float* out,float rho,float strength, float vpScale);

	void BuildGaussStarH(float* out,float rho,float strength);

	void BuildGaussStarV(float* out,float rho,float strength);
};