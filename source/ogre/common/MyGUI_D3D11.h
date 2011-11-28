#ifndef __MYGUI_OGRE_D3D11PLATFORM_H__
#define __MYGUI_OGRE_D3D11PLATFORM_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_OgreTexture.h"
#include "MyGUI_OgreVertexBuffer.h"
#include "MyGUI_OgreRenderManager.h"
#include "MyGUI_OgreDataManager.h"
#include "MyGUI_OgreDiagnostic.h"
#include "MyGUI_OgreTexture.h"
#include "MyGUI_LogManager.h"
#include "MyGUI_OgrePlatform.h"
#include "MyGUI_LastHeader.h"

// naive check if we have MyGUI 3.2 RC or 3.2 SVN. D3D11 ogre platform depends on MyGUI 3.2 SVN

// 3.2 SVN has this defined, RC doesnt
#ifdef MYGUI_PLATFORM_LOG_FILENAME

namespace MyGUI
{
	
	class OgreD3D11RenderManager :	public RenderManager,
		public IRenderTarget,
		public Ogre::WindowEventListener,
		public Ogre::RenderQueueListener,
		public Ogre::RenderSystem::Listener

	{
		//changes
	public:
		Ogre::GpuProgram* createDefaultVertexProgram();
		Ogre::GpuProgram* createDefaultFragmentProgram();
		Ogre::GpuProgram* createDefaultFragmentProgramWith1Texture();
		Ogre::GpuProgram *mDefaultVertexProgram,*mDefaultFragmentProgram,*mDefaultFragmentProgramWith1Texture;
		bool bUseShaders;
	public:
		OgreD3D11RenderManager();
		virtual ~OgreD3D11RenderManager();

		void initialise(Ogre::RenderWindow* _window, Ogre::SceneManager* _scene);
		void shutdown();

		static OgreD3D11RenderManager& getInstance();
		static OgreD3D11RenderManager* getInstancePtr();

		/** @see RenderManager::getViewSize */
		virtual const IntSize& getViewSize() const;

		/** @see RenderManager::getVertexFormat */
		virtual VertexColourType getVertexFormat();

		/** @see RenderManager::createVertexBuffer */
		virtual IVertexBuffer* createVertexBuffer();
		/** @see RenderManager::destroyVertexBuffer */
		virtual void destroyVertexBuffer(IVertexBuffer* _buffer);

		/** @see RenderManager::createTexture */
		virtual ITexture* createTexture(const std::string& _name);
		/** @see RenderManager::destroyTexture */
		virtual void destroyTexture(ITexture* _texture);
		/** @see RenderManager::getTexture */
		virtual ITexture* getTexture(const std::string& _name);

		/** @see RenderManager::isFormatSupported */
		virtual bool isFormatSupported(PixelFormat _format, TextureUsage _usage);

		/** @see IRenderTarget::begin */
		virtual void begin();
		/** @see IRenderTarget::end */
		virtual void end();

		/** @see IRenderTarget::doRender */
		virtual void doRender(IVertexBuffer* _buffer, ITexture* _texture, size_t _count);

		/** @see IRenderTarget::getInfo */
		virtual const RenderTargetInfo& getInfo();

		void setRenderSystem(Ogre::RenderSystem* _render);
		Ogre::RenderSystem* getRenderSystem();

		void setRenderWindow(Ogre::RenderWindow* _window);

		/** Set scene manager where MyGUI will be rendered */
		void setSceneManager(Ogre::SceneManager* _scene);

		/** Get GUI viewport index */
		size_t getActiveViewport();

		/** Set GUI viewport index */
		void setActiveViewport(unsigned short _num);

		Ogre::RenderWindow* getRenderWindow();

		bool getManualRender();
		void setManualRender(bool _value);

		size_t getBatchCount() const;

#if MYGUI_DEBUG_MODE == 1
		virtual bool checkTexture(ITexture* _texture);
#endif

	private:
		virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);
		virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation);
		virtual void windowResized(Ogre::RenderWindow* _window);

		// восстанавливаем буферы
		virtual void eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* parameters);

		void destroyAllResources();
		void updateRenderInfo();

	
	private:
		// флаг для обновления всех и вся
		bool mUpdate;

		IntSize mViewSize;

		Ogre::SceneManager* mSceneManager;

		VertexColourType mVertexFormat;

		// окно, на которое мы подписываемся для изменения размеров
		Ogre::RenderWindow* mWindow;

		// вьюпорт, с которым работает система
		unsigned short mActiveViewport;

		Ogre::RenderSystem* mRenderSystem;
		Ogre::TextureUnitState::UVWAddressingMode mTextureAddressMode;
		Ogre::LayerBlendModeEx mColorBlendMode, mAlphaBlendMode;
	
		RenderTargetInfo mInfo;

		typedef std::map<std::string, ITexture*> MapTexture;
		MapTexture mTextures;

		bool mIsInitialise;
		bool mManualRender;
		size_t mCountBatch;
	};

	class OgreD3D11Platform 
	{
	public:
		OgreD3D11Platform() :
			mIsInitialise(false)
		{
			mLogManager = new LogManager();
			mRenderManager = new OgreD3D11RenderManager();
			mDataManager = new OgreDataManager();
		}

		~OgreD3D11Platform()
		{
			assert(!mIsInitialise);
			delete mRenderManager;
			delete mDataManager;
			delete mLogManager;
		}

		void initialise(Ogre::RenderWindow* _window, Ogre::SceneManager* _scene, const std::string& _group = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, const std::string& _logName = MYGUI_PLATFORM_LOG_FILENAME)
		{
			assert(!mIsInitialise);
			mIsInitialise = true;

			if (!_logName.empty())
				LogManager::getInstance().createDefaultSource(_logName);

			mRenderManager->initialise(_window, _scene);
			mDataManager->initialise(_group);
		}

		void shutdown()
		{
			assert(mIsInitialise);
			mIsInitialise = false;

			mRenderManager->shutdown();
			mDataManager->shutdown();
		}

		OgreD3D11RenderManager* getRenderManagerPtr()
		{
			assert(mIsInitialise);
			return mRenderManager;
		}

		OgreDataManager* getDataManagerPtr()
		{
			assert(mIsInitialise);
			return mDataManager;
		}

	private:
		bool mIsInitialise;
		OgreD3D11RenderManager* mRenderManager;
		OgreDataManager* mDataManager;
		LogManager* mLogManager;
	};

} 

#endif

#endif 
