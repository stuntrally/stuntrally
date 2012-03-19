/*
	Renderer2D    
	-------        
	
	Copyright (c) 2012 Benjamin Kinsey    
	
	Permission is hereby granted, free of charge, to any person obtaining a copy    
	of this software and associated documentation files (the "Software"), to deal    
	in the Software without restriction, including without limitation the rights    
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    
	copies of the Software, and to permit persons to whom the Software is    
	furnished to do so, subject to the following conditions:                                                                                      
	
	The above copyright notice and this permission notice shall be included in    
	all copies or substantial portions of the Software.                                                                                      
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER    
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,    
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    
	THE SOFTWARE.     

*/

#include "pch.h"
#include "OgreRenderer2D.h"
#include <Ogre.h>


Renderer2D::Renderer2D(const std::string& textureName, unsigned int width, unsigned int height) :
	mQueueStarted(false),
	mRenderSystem(Ogre::Root::getSingletonPtr()->getRenderSystem())
{
	Ogre::Texture* ogreTexture = static_cast<Ogre::Texture*>(Ogre::TextureManager::getSingletonPtr()->createManual(textureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_R8G8B8A8, Ogre::TU_RENDERTARGET).get());
	mRenderTarget = ogreTexture->getBuffer()->getRenderTarget();
	mViewport = mRenderTarget->addViewport(NULL);			
	ogreTexture->load();

	mRenderOperation = new Ogre::RenderOperation();

	_createVertexBuffer();

	// Store Texel offset for quick use
	mHorizontalTexelOffset = mRenderSystem->getHorizontalTexelOffset();
	mVerticalTexelOffset = mRenderSystem->getVerticalTexelOffset();
}

Renderer2D::Renderer2D(Ogre::RenderTexture* texture) :
	mQueueStarted(false),
	mRenderSystem(Ogre::Root::getSingletonPtr()->getRenderSystem()),
	mRenderTarget(texture),
	mViewport(texture->getViewport(0))
{
	mRenderOperation = new Ogre::RenderOperation();

	_createVertexBuffer();

	// Store Texel offset for quick use
	mHorizontalTexelOffset = mRenderSystem->getHorizontalTexelOffset();
	mVerticalTexelOffset = mRenderSystem->getVerticalTexelOffset();
}

Renderer2D::Renderer2D(Ogre::Viewport* viewport) :
	mQueueStarted(false),
	mRenderSystem(Ogre::Root::getSingletonPtr()->getRenderSystem()),
	mRenderTarget(viewport->getTarget()),
	mViewport(viewport)
{
	mRenderOperation = new Ogre::RenderOperation();

	_createVertexBuffer();

	// Store Texel offset for quick use
	mHorizontalTexelOffset = mRenderSystem->getHorizontalTexelOffset();
	mVerticalTexelOffset = mRenderSystem->getVerticalTexelOffset();
}

Renderer2D::~Renderer2D()
{
	_destroyVertexBuffer();

	delete mRenderOperation;
}

void Renderer2D::_createVertexBuffer()
{
	mRenderOperation->vertexData = OGRE_NEW Ogre::VertexData();
	mRenderOperation->vertexData->vertexStart = 0;

	_declareVertexStructure();

	// Create the Vertex Buffer, using the Vertex Structure we previously declared in _declareVertexStructure.
	Ogre::HardwareVertexBufferSharedPtr vBuff = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		mRenderOperation->vertexData->vertexDeclaration->getVertexSize(0), // declared Vertex used
		VERTEX_COUNT,
		Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
		false );

	mVertexBuffer = vBuff.get();

	// Bind the created buffer to the renderOperation object.  Now we can manipulate the buffer, and the RenderOp keeps the changes.
			
	mRenderOperation->vertexData->vertexBufferBinding->setBinding( 0, vBuff );
	mRenderOperation->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	mRenderOperation->useIndexes = false;
}

void Renderer2D::_destroyVertexBuffer()
{
	OGRE_DELETE mRenderOperation->vertexData;
	mRenderOperation->vertexData = NULL;
	mVertexBuffer = NULL;
}

void Renderer2D::_declareVertexStructure()
{
	Ogre::VertexDeclaration* vd = mRenderOperation->vertexData->vertexDeclaration;

	// Add position - Vector3 : 4 bytes per float * 2 floats = 8 bytes

	vd->addElement( 0, 0, Ogre::VET_FLOAT2, Ogre::VES_POSITION );

	// Add color - ColorValue : 4 bytes per float * 4 floats = 16 bytes

	vd->addElement( 0, Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT2 ), Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE );

	// Add texture coordinates - Ogre::Vector2 : 4 bytes per float * 2 floats = 8 bytes

	vd->addElement( 0, Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT2 ) +
						Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT4 ),
						Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES );

	/* Our structure representing the Vertices used in the buffer (32 bytes):
		struct Vertex
		{
			Vector2 pos;
			ColorValue color;
			Vector2 uv;
		};
	*/
}

void Renderer2D::_configureRenderSystem()
{
	// set-up matrices
	mRenderSystem->_setWorldMatrix( Ogre::Matrix4::IDENTITY );
	mRenderSystem->_setProjectionMatrix( Ogre::Matrix4::IDENTITY );
	mRenderSystem->_setViewMatrix( Ogre::Matrix4::IDENTITY );

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
	mRenderSystem->_setPolygonMode(Ogre::PM_SOLID);

	Ogre::TextureUnitState::UVWAddressingMode uvwAddressMode;
	uvwAddressMode.u = Ogre::TextureUnitState::TAM_CLAMP;
	uvwAddressMode.v = Ogre::TextureUnitState::TAM_CLAMP;
	uvwAddressMode.w = Ogre::TextureUnitState::TAM_CLAMP;

	Ogre::LayerBlendModeEx colorBlendMode;
	colorBlendMode.blendType = Ogre::LBT_COLOUR;
	colorBlendMode.source1 = Ogre::LBS_TEXTURE;
	colorBlendMode.source2 = Ogre::LBS_DIFFUSE;
	colorBlendMode.operation = Ogre::LBX_MODULATE;
			
	Ogre::LayerBlendModeEx alphaBlendMode;
	alphaBlendMode.blendType = Ogre::LBT_ALPHA;
	alphaBlendMode.source1 = Ogre::LBS_TEXTURE;
	alphaBlendMode.source2 = Ogre::LBS_DIFFUSE;
	alphaBlendMode.operation = Ogre::LBX_MODULATE;

	// initialise texture settings
	mRenderSystem->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
	mRenderSystem->_setTextureCoordSet(0, 0);
	mRenderSystem->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
	mRenderSystem->_setTextureAddressingMode(0, uvwAddressMode);
	mRenderSystem->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
	mRenderSystem->_setAlphaRejectSettings(Ogre::CMPF_ALWAYS_PASS, 0, false);
	mRenderSystem->_setTextureBlendMode(0, colorBlendMode);
	mRenderSystem->_setTextureBlendMode(0, alphaBlendMode);
	mRenderSystem->_disableTextureUnitsFrom(1);

	// enable alpha blending
	mRenderSystem->_setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ONE, Ogre::SBF_ONE);
}

void Renderer2D::_localize(Ogre::Vector2& vertex)
{
	vertex.x = ((vertex.x + mHorizontalTexelOffset) / static_cast<float>(mViewport->getActualWidth())) * 2 - 1;
	vertex.y = ((vertex.y + mVerticalTexelOffset) / static_cast<float>(mViewport->getActualHeight())) * -2 + 1;
}

void Renderer2D::_rotatePoint(Ogre::Vector2& v2, int rotationInDegrees, const Ogre::Vector2& origin)
{
	Ogre::Vector2 originalPoint(v2.x,v2.y);

	v2.x = (Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(static_cast<Ogre::Real>(rotationInDegrees)))) * (originalPoint.x - origin.x)) - (Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(static_cast<Ogre::Real>(rotationInDegrees)))) * (originalPoint.y - origin.y)) + origin.x;
	v2.y = (Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(static_cast<Ogre::Real>(rotationInDegrees)))) * (originalPoint.x - origin.x)) + (Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(static_cast<Ogre::Real>(rotationInDegrees)))) * (originalPoint.y - origin.y)) + origin.y;
}

void Renderer2D::_performRenderOperation(bool frameStarted, const std::string& imageName, int vertexCount)
{
	// Configure Render System - Order is important!

	mRenderSystem->_setRenderTarget(mRenderTarget);

	// When the RenderTarget is set, the Viewport dimensions are not always honored. By setting the dimensions again,
	// the correct dimensions are used.
	mViewport->setDimensions(mViewport->getLeft(), mViewport->getTop(), mViewport->getWidth(), mViewport->getHeight());

	mRenderSystem->_setViewport(mViewport);

	if(!frameStarted)
	{
		mRenderSystem->_beginFrame();
	}

	_configureRenderSystem();

	mRenderSystem->_setTexture(0, true, imageName);

	// Set clipping region

	bool useClipping = false;
	if((mClippingBounds.right > mClippingBounds.left) && (mClippingBounds.bottom > mClippingBounds.top))
	{
		useClipping = true;

		mRenderSystem->setScissorTest(
			true,
			static_cast<unsigned int>(mClippingBounds.left),
			static_cast<unsigned int>(mClippingBounds.top),
			static_cast<unsigned int>(mClippingBounds.right),
			static_cast<unsigned int>(mClippingBounds.bottom)
			);
	}

	// Perform render operation

	mRenderOperation->vertexData->vertexStart = 0;
	mRenderOperation->vertexData->vertexCount = vertexCount;
	mRenderSystem->_render(*mRenderOperation);

	if(useClipping)
	{
		mRenderSystem->setScissorTest(false);
	}

	if(!frameStarted)
	{
		mRenderSystem->_endFrame();
	}
}

void Renderer2D::beginRectQueue(std::string imageFile, std::string resourceGroupName)
{
	if(mQueueStarted)
	{
		throw "Cannot begin the Queue, it has already begun! Please end the queue before beginning it again. : beginRectQueue";
	}

	mQueueStarted = true;

	mQueueImage = imageFile;
	mQueueImageResourceGroup = resourceGroupName;

	// Lock buffer

	mVertexBufferIndex = (Vertex*)mVertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

	mVertexCount = 0;
}

void Renderer2D::clear(Ogre::ColourValue cv)
{
	mRenderSystem->_setRenderTarget(mRenderTarget);

	mRenderSystem->clearFrameBuffer(Ogre::FBT_COLOUR, cv);
}

void Renderer2D::drawLine(bool frameStarted, Ogre::Vector2 p1, Ogre::Vector2 p2, Ogre::ColourValue cv, unsigned int thickness)
{
	if(mQueueStarted)
	{
		throw "Cannot perform draw operations when Rect Queue has started! Please end the queue before performing draw operations. : drawLine";
	}

	Ogre::Vector2 direction = (p2 - p1).perpendicular();
	direction.normalise();

	Ogre::Vector2 offset = (direction * (thickness * 0.5f));
	Ogre::Vector2 p1Left	= p1 + offset;
	Ogre::Vector2 p1Right = p1 - offset;
	Ogre::Vector2 p2Left	= p2 + offset;
	Ogre::Vector2 p2Right = p2 - offset;

	Ogre::Vector2 verts[6];

	verts[0] = p1Right;
	verts[1] = p2Right;
	verts[2] = p1Left;

	verts[3] = p2Right;
	verts[4] = p2Left;
	verts[5] = p1Left;

	// Convert to screen coordinates
	for(int i = 0; i < 6; ++i)
	{
		_localize(verts[i]);
	}

	// Lock buffer

	Vertex* data = (Vertex*)mVertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

	// Write vertices to vertex buffer

	for ( size_t x = 0; x < 6; x++ )
	{
		data[x].pos = verts[x];
		data[x].color = cv;
		data[x].uv.x = 1.0f;
		data[x].uv.y = 1.0f;
	}

	// Unlock buffer

	mVertexBuffer->unlock();

	_performRenderOperation(frameStarted, "", 6);
}

/*
* Check Desintation values. For a value of 0 or less, the viewport dimensions should be used.
*/
void checkDestRect(Ogre::Rect& dest, Ogre::Viewport* viewport)
{
	// A value of 0 (or less) indicates the full size of the Viewport.
	if(dest.right <= 0)
	{
		dest.right = static_cast<long>(viewport->getActualWidth());
	}
	if(dest.bottom <= 0)
	{
		dest.bottom = static_cast<long>(viewport->getActualHeight());
	}
}

/**
* Check Origin values. If default values are used, the origin should be set to the center of destination area.
*/
void checkOrigin(Ogre::Vector2& origin, const Ogre::Rect& dest)
{
	if(origin.x == -1)
	{
		origin.x = dest.left + ((dest.right - dest.left) / 2.0f);
	}
	if(origin.y == -1)
	{
		origin.y = dest.top + ((dest.bottom - dest.top) / 2.0f);
	}
}

/**
* Loads the image from file if not already loaded, or retrieves the already loaded image.
*/
Ogre::TexturePtr checkImage(const std::string& imageFile, const std::string& resourceGroupName)
{
	if(imageFile == "")
	{
		return Ogre::TexturePtr(NULL);
	}
	else
	{
		Ogre::TextureManager* tm = Ogre::TextureManager::getSingletonPtr();
		if(!tm->resourceExists(imageFile))
		{
			return tm->load(imageFile, resourceGroupName, Ogre::TEX_TYPE_2D, 0, 1.0f);
		}
		else
		{
			return static_cast<Ogre::TexturePtr>(tm->getByName(imageFile));
		}
	}
}

/**
* Check Source values. If default values are used, the source dimensions should match the dimensions of the image.
*/
void checkSourceRect(Ogre::Rect& source, Ogre::TexturePtr texture)
{
	if(texture.isNull())
	{
		return;
	}
	else
	{
		if(source.right <= 0)
		{
			source.right = static_cast<unsigned int>(texture->getWidth());
		}
		if(source.bottom <= 0)
		{
			source.bottom = static_cast<unsigned int>(texture->getHeight());
		}
	}
}

void buildVertexData(const Ogre::Rect& dest, Ogre::Vector2* verts)
{
	Ogre::Vector2	    topLeft( static_cast<float>(dest.left),  static_cast<float>(dest.top)    );
	Ogre::Vector2	 bottomLeft( static_cast<float>(dest.left),  static_cast<float>(dest.bottom) );
	Ogre::Vector2      topRight( static_cast<float>(dest.right), static_cast<float>(dest.top)    );
	Ogre::Vector2	bottomRight( static_cast<float>(dest.right), static_cast<float>(dest.bottom) );

	verts[0] = bottomLeft;
	verts[1] = bottomRight;
	verts[2] = topLeft;

	verts[3] = bottomRight;
	verts[4] = topRight;
	verts[5] = topLeft;
}

void buildUVData(Ogre::TexturePtr texture, const Ogre::Rect& source, Ogre::Vector2* uv)
{
	float uvleft	= 0.0f;
	float uvtop		= 0.0f;
	float uvright	= 1.0f;
	float uvbottom	= 1.0f;

	if(!texture.isNull())
	{
		float imageWidth = static_cast<float>(texture->getWidth());
		float imageHeight = static_cast<float>(texture->getHeight());

		float uvleft	= static_cast<float>(source.left)	 /	 imageWidth;
		float uvtop		= static_cast<float>(source.top)	 /	 imageHeight;
		float uvright	= static_cast<float>(source.right)	 /	 imageWidth;
		float uvbottom	= static_cast<float>(source.bottom)	 /	 imageHeight;
	}

	uv[0] = Ogre::Vector2( uvleft,	uvbottom );
	uv[1] = Ogre::Vector2( uvright,	uvbottom );
	uv[2] = Ogre::Vector2( uvleft,	uvtop );
	uv[3] = Ogre::Vector2( uvright,	uvbottom );
	uv[4] = Ogre::Vector2( uvright,	uvtop );
	uv[5] = Ogre::Vector2( uvleft,	uvtop );
}

void writeDataToVertexBuffer(Ogre::HardwareVertexBuffer* vertexBuffer, Ogre::Vector2* verts, Ogre::Vector2* uv, const Ogre::ColourValue& cv)
{
	// Lock buffer

	Renderer2D::Vertex* data = (Renderer2D::Vertex*)vertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

	// Write vertices to vertex buffer

	for ( size_t x = 0; x < 6; x++ )
	{
		data[x].pos = verts[x];
		data[x].color = cv;
		data[x].uv = uv[x];
	}

	// Unlock buffer

	vertexBuffer->unlock();
}

void Renderer2D::drawImage(bool frameStarted, const std::string& imageFile, std::string resourceGroupName, Ogre::Rect dest, Ogre::ColourValue cv, Ogre::Rect source, int degRotation, Ogre::Vector2 origin)
{
	if(mQueueStarted)
	{
		throw "Cannot perform draw operations when Rect Queue has started! Please end the queue before performing draw operations. : drawImage";
	}

	// Check if default values are used. (Dest matches viewport dimensions by default)
	checkDestRect(dest, mViewport);

	// Check if default values are used. (Origin is centered around Dest by default)
	checkOrigin(origin, dest);
	
	// Make sure the image is loaded
	Ogre::TexturePtr texture = checkImage(imageFile, resourceGroupName);

	// Check if default values are used. (Source matches image dimensions by default)
	checkSourceRect(source, texture);

	// Build Vertex Data
	
	Ogre::Vector2 verts[6];
	buildVertexData(dest, verts);

	// Apply rotation
	
	if(degRotation != 0)
	{
		for(int i = 0; i < 6; ++i)
		{
			_rotatePoint(verts[i],degRotation,origin);
		}
	}

	// Convert to screen coordinates
	for(int i = 0; i < 6; ++i)
	{
		_localize(verts[i]);
	}

	// Build UV Data

	Ogre::Vector2 uv[6];
	buildUVData(texture, source, uv);

	// Write vertices and uv coords to vertex buffer
	writeDataToVertexBuffer(mVertexBuffer, verts, uv, cv);

	// Perform Rendering operation
	
	std::string image = imageFile;
	std::string::size_type lastForwardSlash = image.find_last_of('/');
	if(lastForwardSlash != std::string::npos)
	{
		image = image.substr(lastForwardSlash);
	}

	_performRenderOperation(frameStarted, image, 6);
}

void Renderer2D::drawRect(bool frameStarted, Ogre::Rect dest, Ogre::ColourValue cv, int degRotation, Ogre::Vector2 origin)
{
	if(mQueueStarted)
	{
		throw "Cannot perform draw operations when Rect Queue has started! Please end the queue before performing draw operations. : drawRect";
	}

	// Check if default values are used. (Dest matches viewport dimensions by default)
	checkDestRect(dest, mViewport);

	// Check if default values are used. (Origin is centered around Dest by default)
	checkOrigin(origin, dest);

	// Build Vertex Data
	
	Ogre::Vector2 verts[6];
	buildVertexData(dest, verts);

	// Apply rotation

	if(degRotation != 0)
	{
		for(int i = 0; i < 6; ++i)
		{
			_rotatePoint(verts[i],degRotation,origin);
		}
	}

	// Convert to screen coordinates
	for(int i = 0; i < 6; ++i)
	{
		_localize(verts[i]);
	}

	// Build UV Data

	Ogre::Vector2 uv[6];

	float uvleft	= 0.0f;
	float uvtop		= 0.0f;
	float uvright	= 1.0f;
	float uvbottom	= 1.0f;

	uv[0] = Ogre::Vector2( uvleft,	uvbottom );
	uv[1] = Ogre::Vector2( uvright,	uvbottom );
	uv[2] = Ogre::Vector2( uvleft,	uvtop );
	uv[3] = Ogre::Vector2( uvright,	uvbottom );
	uv[4] = Ogre::Vector2( uvright,	uvtop );
	uv[5] = Ogre::Vector2( uvleft,	uvtop );

	// Write vertices and uv coords to vertex buffer
	writeDataToVertexBuffer(mVertexBuffer, verts, uv, cv);

	// Perform Rendering operation
	_performRenderOperation(frameStarted, "", 6);
}

void Renderer2D::drawTiledImage(bool frameStarted, const std::string& imageFile, std::string resourceGroupName, Ogre::Rect dest, Ogre::ColourValue cv, Ogre::Rect source, int degRotation, Ogre::Vector2 origin)
{
	if(mQueueStarted)
	{
		throw "Cannot perform draw operations when Rect Queue has started! Please end the queue before performing draw operations. : drawTiledImage";
	}

	// Check if default values are used. (Dest matches viewport dimensions by default)
	checkDestRect(dest, mViewport);

	// Check if default values are used. (Origin is centered around Dest by default)
	checkOrigin(origin, dest);
	
	// Make sure the image is loaded
	Ogre::TexturePtr texture = checkImage(imageFile, resourceGroupName);

	// Check if default values are used. (Source matches image dimensions by default)
	checkSourceRect(source, texture);
		
	// Build UV Data - It will be the same for all tiled images

	Ogre::Vector2 uv[6];
	buildUVData(texture, source, uv);

	// Draw source rect, tiled in the dest area

	float imageWidth = static_cast<float>(texture->getWidth());
	float imageHeight = static_cast<float>(texture->getHeight());

	Ogre::Vector2 verts[6];

	unsigned int numVertsAdded = 0;

	float left		= static_cast<float>(dest.left);
	float right		= static_cast<float>(dest.right);
	float top		= static_cast<float>(dest.top);
	float bottom	= static_cast<float>(dest.bottom);
	
	float x = left;
	float y = top;

	while( y < bottom )
	{
		// Draw the image
			
			// Build Vertex Data

			Ogre::Vector2	    topLeft( static_cast<float>(x),  static_cast<float>(y)  );
			Ogre::Vector2	 bottomLeft( static_cast<float>(x),  static_cast<float>(y + (source.bottom - source.top)) );
			Ogre::Vector2      topRight( static_cast<float>(x + (source.right - source.left)), static_cast<float>(y)  );
			Ogre::Vector2	bottomRight( static_cast<float>(x + (source.right - source.left)), static_cast<float>(y + (source.bottom - source.top)) );

			verts[0] = bottomLeft;
			verts[1] = bottomRight;
			verts[2] = topLeft;

			verts[3] = bottomRight;
			verts[4] = topRight;
			verts[5] = topLeft;

			// Handle rotation

			if(degRotation != 0)
			{
				for(int i = 0; i < 6; ++i)
				{
					_rotatePoint(verts[i],degRotation,origin);
				}
			}

			// Convert to screen coordinates
			for(int i = 0; i < 6; ++i)
			{
				_localize(verts[i]);
			}

			// Lock buffer - 8 floats per vertex, 6 vertices per quad (image) that we render

			Vertex* data = (Vertex*)mVertexBuffer->lock((sizeof(float) * 8) * numVertsAdded,(sizeof(float) * 8) * 6,Ogre::HardwareBuffer::HBL_NORMAL);

			// Write vertices to vertex buffer

			for ( size_t index = 0; index < 6; index++ )
			{
				data[index].pos = verts[index];
				data[index].color = cv;
				data[index].uv = uv[index];
			}

			// Unlock buffer

			mVertexBuffer->unlock();

			numVertsAdded += 6;				

		// Advance x/y position

		x += imageWidth;
		if( x >= right )
		{
			x = left;
			y += imageHeight;
		}
	}

	// Perform Rendering operation

	std::string image = imageFile;
	std::string::size_type lastForwardSlash = image.find_last_of('/');
	if(lastForwardSlash != std::string::npos)
	{
		image = image.substr(lastForwardSlash);
	}

	_performRenderOperation(frameStarted, image, numVertsAdded);
}

void Renderer2D::endRectQueue(bool frameStarted)
{
	if(!mQueueStarted)
	{
		throw "Cannot end the Rect Queue, the Rect Queue has not been started! Please begin the queue prior to ending it. : endRectQueue";
	}

	// Unlock buffer

	mVertexBuffer->unlock();

	if(mVertexCount <= 0)
	{
		mQueueStarted = false;
		mQueueImage = "";
		mQueueImageResourceGroup = "";
		mVertexBufferIndex = NULL;
		mVertexCount = 0;

		return;
	}

	// Perform Rendering operation

	std::string image = mQueueImage;
	std::string::size_type lastForwardSlash = image.find_last_of('/');
	if(lastForwardSlash != std::string::npos)
	{
		image = image.substr(lastForwardSlash);
	}

	_performRenderOperation(frameStarted, image, mVertexCount);

	mQueueStarted = false;
	mQueueImage = "";
	mQueueImageResourceGroup = "";
	mVertexBufferIndex = NULL;
	mVertexCount = 0;
}

Ogre::Rect Renderer2D::getClippingBounds()
{
	return mClippingBounds;
}

bool Renderer2D::getRectQueueStarted()
{
	return mQueueStarted;
}

Ogre::RenderTarget* Renderer2D::getRenderTarget()
{
	return mRenderTarget;
}

void Renderer2D::queueRect(Ogre::Rect dest, Ogre::ColourValue cv, Ogre::Rect source, int degRotation, Ogre::Vector2 origin)
{
	if(!mQueueStarted)
	{
		throw "Cannot queue draw operations when Rect Queue has not started! Please begin the queue before queueing draw operations. : queueRect";
	}

	// Check if default values are used. (Dest matches viewport dimensions by default)
	checkDestRect(dest, mViewport);

	// Check if default values are used. (Origin is centered around Dest by default)
	checkOrigin(origin, dest);

	// Make sure the image is loaded
	Ogre::TexturePtr texture = checkImage(mQueueImage, mQueueImageResourceGroup);

	// Check if default values are used. (Source matches image dimensions by default)
	checkSourceRect(source, texture);

	// Build Vertex Data
	
	Ogre::Vector2 verts[6];
	buildVertexData(dest, verts);

	// Apply rotation
	
	if(degRotation != 0)
	{
		for(int i = 0; i < 6; ++i)
		{
			_rotatePoint(verts[i],degRotation,origin);
		}
	}

	// Convert to screen coordinates
	for(int i = 0; i < 6; ++i)
	{
		_localize(verts[i]);
	}

	// Build UV Data

	Ogre::Vector2 uv[6];
	buildUVData(texture, source, uv);


	// Add Vertex data to locked Vertex Buffer

	Vertex* data = mVertexBufferIndex;

	// Write vertices to vertex buffer

	for ( size_t x = 0; x < 6; x++ )
	{
		data[x].pos = verts[x];
		data[x].color = cv;
		data[x].uv = uv[x];
	}

	// Increment Vertex Buffer Pointer

	mVertexBufferIndex += 6;

	mVertexCount += 6;
}

void Renderer2D::setClippingBounds(const Ogre::Rect& r)
{
	mClippingBounds = r;
}

void Renderer2D::setRenderTexture(Ogre::RenderTexture* texture)
{
	mRenderTarget = texture;
	mViewport = texture->getViewport(0);
}

void Renderer2D::setViewport(Ogre::Viewport* viewport)
{
	mRenderTarget = viewport->getTarget();
	mViewport = viewport;
}

void Renderer2D::writeContentsToFile(const std::string& fileName)
{
	mRenderTarget->writeContentsToFile(fileName);
}

