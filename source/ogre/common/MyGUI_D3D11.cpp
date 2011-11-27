#include "MyGUI_OgreDataManager.h"
#include "MyGUI_OgreRenderManager.h"
#include "MyGUI_OgreTexture.h"
#include "MyGUI_OgreVertexBuffer.h"
#include "MyGUI_LogManager.h"
#include "MyGUI_OgreDiagnostic.h"
#include "MyGUI_Timer.h"
#include "MyGUI_Gui.h"
#include "MyGUI_D3D11.h"
namespace MyGUI
{
	OgreD3D11RenderManager& OgreD3D11RenderManager::getInstance()
	{
		return *getInstancePtr();
	}
	OgreD3D11RenderManager* OgreD3D11RenderManager::getInstancePtr()
	{
		return static_cast<OgreD3D11RenderManager*>(RenderManager::getInstancePtr());
	}

	OgreD3D11RenderManager::OgreD3D11RenderManager() :
		mUpdate(false),
		mSceneManager(nullptr),
		mWindow(nullptr),
		mActiveViewport(0),
		mRenderSystem(nullptr),
		mIsInitialise(false),
		mManualRender(false),
		mCountBatch(0),
		bUseShaders(false)
	{
	}

	OgreD3D11RenderManager::~OgreD3D11RenderManager()
	{
	
	}

	void OgreD3D11RenderManager::initialise(Ogre::RenderWindow* _window, Ogre::SceneManager* _scene)
	{
		MYGUI_PLATFORM_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
		MYGUI_PLATFORM_LOG(Info, "* Initialise: " << getClassTypeName());

		mColorBlendMode.blendType = Ogre::LBT_COLOUR;
		mColorBlendMode.source1 = Ogre::LBS_TEXTURE;
		mColorBlendMode.source2 = Ogre::LBS_DIFFUSE;
		mColorBlendMode.operation = Ogre::LBX_MODULATE;

		mAlphaBlendMode.blendType = Ogre::LBT_ALPHA;
		mAlphaBlendMode.source1 = Ogre::LBS_TEXTURE;
		mAlphaBlendMode.source2 = Ogre::LBS_DIFFUSE;
		mAlphaBlendMode.operation = Ogre::LBX_MODULATE;

		mTextureAddressMode.u = Ogre::TextureUnitState::TAM_CLAMP;
		mTextureAddressMode.v = Ogre::TextureUnitState::TAM_CLAMP;
		mTextureAddressMode.w = Ogre::TextureUnitState::TAM_CLAMP;

		mSceneManager = nullptr;
		mWindow = nullptr;
		mUpdate = false;
		mRenderSystem = nullptr;
		mActiveViewport = 0;

		mDefaultVertexProgram = createDefaultVertexProgram();
		mDefaultFragmentProgram = createDefaultFragmentProgram();
		mDefaultFragmentProgramWith1Texture = createDefaultFragmentProgramWith1Texture();
		if(mDefaultVertexProgram != 0 && mDefaultFragmentProgram != 0 && mDefaultFragmentProgramWith1Texture != 0)
		{
			bUseShaders = true;
		}

		Ogre::Root* root = Ogre::Root::getSingletonPtr();
		if (root != nullptr)
			setRenderSystem(root->getRenderSystem());
		setRenderWindow(_window);
		setSceneManager(_scene);

		MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully initialized");
		mIsInitialise = true;
	}

	void OgreD3D11RenderManager::shutdown()
	{
		MYGUI_PLATFORM_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
		MYGUI_PLATFORM_LOG(Info, "* Shutdown: " << getClassTypeName());

		destroyAllResources();

		setSceneManager(nullptr);
		setRenderWindow(nullptr);
		setRenderSystem(nullptr);

		MYGUI_PLATFORM_LOG(Info, getClassTypeName() << " successfully shutdown");
		mIsInitialise = false;
	}

	void OgreD3D11RenderManager::setRenderSystem(Ogre::RenderSystem* _render)
	{
		// отписываемся
		if (mRenderSystem != nullptr)
		{
			mRenderSystem->removeListener(this);
			mRenderSystem = nullptr;
		}

		mRenderSystem = _render;

		// подписываемся на рендер евент
		if (mRenderSystem != nullptr)
		{
			mRenderSystem->addListener(this);

			// формат цвета в вершинах
			Ogre::VertexElementType vertex_type = mRenderSystem->getColourVertexElementType();
			if (vertex_type == Ogre::VET_COLOUR_ARGB)
				mVertexFormat = VertexColourType::ColourARGB;
			else if (vertex_type == Ogre::VET_COLOUR_ABGR)
				mVertexFormat = VertexColourType::ColourABGR;

			updateRenderInfo();
		}
	}

	Ogre::RenderSystem* OgreD3D11RenderManager::getRenderSystem()
	{
		return mRenderSystem;
	}

	void OgreD3D11RenderManager::setRenderWindow(Ogre::RenderWindow* _window)
	{
		// отписываемся
		if (mWindow != nullptr)
		{
			Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
			mWindow = nullptr;
		}

		mWindow = _window;

		if (mWindow != nullptr)
		{
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
			windowResized(mWindow);
		}
	}

	void OgreD3D11RenderManager::setSceneManager(Ogre::SceneManager* _scene)
	{
		if (nullptr != mSceneManager)
		{
			mSceneManager->removeRenderQueueListener(this);
			mSceneManager = nullptr;
		}

		mSceneManager = _scene;

		if (nullptr != mSceneManager)
		{
			mSceneManager->addRenderQueueListener(this);
		}
	}

	void OgreD3D11RenderManager::setActiveViewport(unsigned short _num)
	{
		mActiveViewport = _num;

		if (mWindow != nullptr)
		{
			Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
			Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

			// рассылка обновлений
			windowResized(mWindow);
		}
	}

	void OgreD3D11RenderManager::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
	{
		Gui* gui = Gui::getInstancePtr();
		if (gui == nullptr)
			return;

		if (Ogre::RENDER_QUEUE_OVERLAY != queueGroupId)
			return;

		Ogre::Viewport* viewport = mSceneManager->getCurrentViewport();
		if (nullptr == viewport
			|| !viewport->getOverlaysEnabled())
			return;

		if (mWindow->getNumViewports() <= mActiveViewport
			|| viewport != mWindow->getViewport(mActiveViewport))
			return;

		mCountBatch = 0;

		static Timer timer;
		static unsigned long last_time = timer.getMilliseconds();
		unsigned long now_time = timer.getMilliseconds();
		unsigned long time = now_time - last_time;

		onFrameEvent((float)((double)(time) / (double)1000));

		last_time = now_time;

		//begin();
		setManualRender(true);
		onRenderToTarget(this, mUpdate);
		//end();

		// сбрасываем флаг
		mUpdate = false;
	}

	void OgreD3D11RenderManager::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation)
	{
	}

	void OgreD3D11RenderManager::eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* parameters)
	{
		if (eventName == "DeviceLost")
		{
		}
		else if (eventName == "DeviceRestored")
		{
			// обновить всех
			mUpdate = true;
		}
	}

	IVertexBuffer* OgreD3D11RenderManager::createVertexBuffer()
	{
		return new OgreVertexBuffer();
	}

	void OgreD3D11RenderManager::destroyVertexBuffer(IVertexBuffer* _buffer)
	{
		delete _buffer;
	}

	// для оповещений об изменении окна рендера
	void OgreD3D11RenderManager::windowResized(Ogre::RenderWindow* _window)
	{
		if (_window->getNumViewports() > mActiveViewport)
		{
			Ogre::Viewport* port = _window->getViewport(mActiveViewport);
#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 7, 0) && OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
			Ogre::OrientationMode orient = port->getOrientationMode();
			if (orient == Ogre::OR_DEGREE_90 || orient == Ogre::OR_DEGREE_270)
				mViewSize.set(port->getActualHeight(), port->getActualWidth());
			else
				mViewSize.set(port->getActualWidth(), port->getActualHeight());
#else
			mViewSize.set(port->getActualWidth(), port->getActualHeight());
#endif

			// обновить всех
			mUpdate = true;

			updateRenderInfo();

			onResizeView(mViewSize);
		}
	}

	void OgreD3D11RenderManager::updateRenderInfo()
	{
		if (mRenderSystem != nullptr)
		{
			mInfo.maximumDepth = mRenderSystem->getMaximumDepthInputValue();
			mInfo.hOffset = mRenderSystem->getHorizontalTexelOffset() / float(mViewSize.width);
			mInfo.vOffset = mRenderSystem->getVerticalTexelOffset() / float(mViewSize.height);
			mInfo.aspectCoef = float(mViewSize.height) / float(mViewSize.width);
			mInfo.pixScaleX = 1.0f / float(mViewSize.width);
			mInfo.pixScaleY = 1.0f / float(mViewSize.height);
		}
	}

	void OgreD3D11RenderManager::doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count)
	{
		if (getManualRender())
		{
			begin();
			setManualRender(false);
		}

		if (_texture)
		{
			if(bUseShaders)
			{
				mRenderSystem->bindGpuProgram(mDefaultFragmentProgramWith1Texture);
			}
			OgreTexture* texture = static_cast<OgreTexture*>(_texture);
			Ogre::TexturePtr texture_ptr = texture->getOgreTexture();
			if (!texture_ptr.isNull())
			{
				mRenderSystem->_setTexture(0, true, texture_ptr);
				mRenderSystem->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_NONE);
			}
		}
		else
		{
			if(bUseShaders)
			{
				mRenderSystem->bindGpuProgram(mDefaultFragmentProgram);
			}
		}

		OgreVertexBuffer* buffer = static_cast<OgreVertexBuffer*>(_buffer);
		Ogre::RenderOperation* operation = buffer->getRenderOperation();
		operation->vertexData->vertexCount = _count;

		mRenderSystem->_render(*operation);

		++ mCountBatch;
	}

	void OgreD3D11RenderManager::begin()
	{
		// set-up matrices
		mRenderSystem->_setWorldMatrix(Ogre::Matrix4::IDENTITY);
		mRenderSystem->_setViewMatrix(Ogre::Matrix4::IDENTITY);

#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 7, 0) && OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
		Ogre::OrientationMode orient = mWindow->getViewport(mActiveViewport)->getOrientationMode();
		mRenderSystem->_setProjectionMatrix(Ogre::Matrix4::IDENTITY * Ogre::Quaternion(Ogre::Degree(orient * 90.f), Ogre::Vector3::UNIT_Z));
#else
		mRenderSystem->_setProjectionMatrix(Ogre::Matrix4::IDENTITY);
#endif

		// initialise render settings
		mRenderSystem->setLightingEnabled(false);
		mRenderSystem->_setDepthBufferParams(false, false);
		mRenderSystem->_setDepthBias(0, 0);
		mRenderSystem->_setCullingMode(Ogre::CULL_NONE);
		mRenderSystem->_setFog(Ogre::FOG_NONE);
		mRenderSystem->_setColourBufferWriteEnabled(true, true, true, true);
		mRenderSystem->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
		mRenderSystem->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);
		mRenderSystem->setShadingType(Ogre::SO_GOURAUD);

		// initialise texture settings
		mRenderSystem->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
		mRenderSystem->_setTextureCoordSet(0, 0);
		mRenderSystem->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_NONE);
		mRenderSystem->_setTextureAddressingMode(0, mTextureAddressMode);
		mRenderSystem->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
#if OGRE_VERSION < MYGUI_DEFINE_VERSION(1, 6, 0)
		mRenderSystem->_setAlphaRejectSettings(Ogre::CMPF_ALWAYS_PASS, 0);
#else
		mRenderSystem->_setAlphaRejectSettings(Ogre::CMPF_ALWAYS_PASS, 0, false);
#endif
		mRenderSystem->_setTextureBlendMode(0, mColorBlendMode);
		mRenderSystem->_setTextureBlendMode(0, mAlphaBlendMode);
		mRenderSystem->_disableTextureUnitsFrom(1);

		// enable alpha blending
		mRenderSystem->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

		// always use wireframe
		// TODO: add option to enable wireframe mode in platform
		mRenderSystem->_setPolygonMode(Ogre::PM_SOLID);
	
		if(bUseShaders)
		{
			mRenderSystem->bindGpuProgram(mDefaultVertexProgram);
		}

	}

	void OgreD3D11RenderManager::end()
	{
	}

	ITexture* OgreD3D11RenderManager::createTexture(const std::string& _name)
	{
		MapTexture::const_iterator item = mTextures.find(_name);
		MYGUI_PLATFORM_ASSERT(item == mTextures.end(), "Texture '" << _name << "' already exist");

		OgreTexture* texture = new OgreTexture(_name, OgreDataManager::getInstance().getGroup());
		mTextures[_name] = texture;
		return texture;
	}

	void OgreD3D11RenderManager::destroyTexture(ITexture* _texture)
	{
		if (_texture == nullptr) return;

		MapTexture::iterator item = mTextures.find(_texture->getName());
		MYGUI_PLATFORM_ASSERT(item != mTextures.end(), "Texture '" << _texture->getName() << "' not found");

		mTextures.erase(item);
		delete _texture;
	}

	ITexture* OgreD3D11RenderManager::getTexture(const std::string& _name)
	{
		MapTexture::const_iterator item = mTextures.find(_name);
		if (item == mTextures.end())
		{
			Ogre::TexturePtr texture = (Ogre::TexturePtr)Ogre::TextureManager::getSingleton().getByName(_name);
			if (!texture.isNull())
			{
				ITexture* result = createTexture(_name);
				static_cast<OgreTexture*>(result)->setOgreTexture(texture);
				return result;
			}
			return nullptr;
		}
		return item->second;
	}

	bool OgreD3D11RenderManager::isFormatSupported(PixelFormat _format, TextureUsage _usage)
	{
		return Ogre::TextureManager::getSingleton().isFormatSupported(
			Ogre::TEX_TYPE_2D,
			OgreTexture::convertFormat(_format),
			OgreTexture::convertUsage(_usage));
	}

	void OgreD3D11RenderManager::destroyAllResources()
	{
		for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
		{
			delete item->second;
		}
		mTextures.clear();
	}

#if MYGUI_DEBUG_MODE == 1
	bool OgreD3D11RenderManager::checkTexture(ITexture* _texture)
	{
		for (MapTexture::const_iterator item = mTextures.begin(); item != mTextures.end(); ++item)
		{
			if (item->second == _texture)
				return true;
		}
		return false;
	}
#endif

	const IntSize& OgreD3D11RenderManager::getViewSize() const
	{
		return mViewSize;
	}

	VertexColourType OgreD3D11RenderManager::getVertexFormat()
	{
		return mVertexFormat;
	}

	const RenderTargetInfo& OgreD3D11RenderManager::getInfo()
	{
		return mInfo;
	}

	size_t OgreD3D11RenderManager::getActiveViewport()
	{
		return mActiveViewport;
	}

	Ogre::RenderWindow* OgreD3D11RenderManager::getRenderWindow()
	{
		return mWindow;
	}

	bool OgreD3D11RenderManager::getManualRender()
	{
		return mManualRender;
	}

	void OgreD3D11RenderManager::setManualRender(bool _value)
	{
		mManualRender = _value;
	}

	size_t OgreD3D11RenderManager::getBatchCount() const
	{
		return mCountBatch;
	}
	
	Ogre::GpuProgram* OgreD3D11RenderManager::createDefaultVertexProgram()
	{
		Ogre::HighLevelGpuProgramManager* mgr = Ogre::HighLevelGpuProgramManager::getSingletonPtr();
		std::string progName ="mygui_default_VP";

		Ogre::HighLevelGpuProgramPtr ret = mgr->createProgram(progName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", Ogre::GPT_VERTEX_PROGRAM);

		ret->setParameter("profiles", "vs_4_0");
		ret->setParameter("entry_point", "main_vp");

		Ogre::StringUtil::StrStreamType sourceStr;
	
		sourceStr <<
		"void main_vp( \n"
		"	in float4 position : POSITION, \n"
		"	in float4 diffuse : DIFFUSE, \n"
		"	in float2 uv :TEXCOORD0, \n"
		"	out float4 oPos : POSITION, \n"
		"	out float4 oDiffuse : TEXCOORD0, \n"
		"   out float2 oUV : TEXCOORD1) \n"
		"{ \n"
		"	oPos = position;"
		"   oDiffuse = diffuse; \n"
		"   oUV = uv; \n"
		"} \n";
	
		ret->setSource(sourceStr.str());
		ret->load();
	
		return ret->_getBindingDelegate();
	}

	//----------------------------------------------------------------------------------------

	Ogre::GpuProgram* OgreD3D11RenderManager::createDefaultFragmentProgram()
	{
		Ogre::HighLevelGpuProgramManager* mgr = Ogre::HighLevelGpuProgramManager::getSingletonPtr();
		std::string progName = "mygui_default_FP";

		Ogre::HighLevelGpuProgramPtr ret = mgr->createProgram(progName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", Ogre::GPT_FRAGMENT_PROGRAM);

		ret->setParameter("profiles", "ps_4_0");
		ret->setParameter("entry_point", "main_fp");

		Ogre::StringUtil::StrStreamType sourceStr;
	
		sourceStr <<
		"float4 main_fp(in float4 iPos : POSITION, \n"
		"in float4 diffuse : TEXCOORD0, \n"
		"in float2 uv : TEXCOORD1 \n"
		"): COLOR0 \n"
		"{ \n"
		"	return diffuse; \n"
		"} \n";
	
		ret->setSource(sourceStr.str());
		ret->load();
	
		return ret->_getBindingDelegate();
	}

	Ogre::GpuProgram* OgreD3D11RenderManager::createDefaultFragmentProgramWith1Texture()
	{
		Ogre::HighLevelGpuProgramManager* mgr = Ogre::HighLevelGpuProgramManager::getSingletonPtr();
		std::string progName = "mygui_default_FP";

		Ogre::HighLevelGpuProgramPtr ret = mgr->createProgram(progName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", Ogre::GPT_FRAGMENT_PROGRAM);

		ret->setParameter("profiles", "ps_4_0");
		ret->setParameter("entry_point", "main_fp");

		Ogre::StringUtil::StrStreamType sourceStr;
	
		sourceStr <<
		"float4 main_fp(in float4 iPos : POSITION, \n"
		"in float4 diffuse : TEXCOORD0, \n"
		"in float2 uv : TEXCOORD1, \n"
		"uniform sampler2D diffuseMap : TEXUNIT0): COLOR0 \n"
		"{ \n"
		"	float4 diffuseTex = tex2D(diffuseMap, uv); \n"
		"	diffuseTex = diffuseTex * diffuse ; \n"
		"	return diffuseTex ; \n"
		"} \n";
	
		ret->setSource(sourceStr.str());
		ret->load();

		return ret->_getBindingDelegate();
	}

}