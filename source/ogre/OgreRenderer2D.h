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

#ifndef OGRE_RENDERER_2D_H
#define OGRE_RENDERER_2D_H

#include <Ogre.h>

class Renderer2D
{
public:

	/**
	* Vertex structure used for rendering operations.
	*/
	struct Vertex
	{
		Ogre::Vector2 pos;
		Ogre::ColourValue color;
		Ogre::Vector2 uv;
	};

	/**
	* This constructor creates a RenderTexture that will receive rendering operations.
	*/
	Renderer2D(const std::string& textureName, unsigned int width, unsigned int height);

	/**
	* This constructor requires a RenderTexture. All rendering operations will be
	* output to this texture.
	*/
	Renderer2D(Ogre::RenderTexture* texture);

	/**
	* This constructor requires a viewport.  Typically the viewport will belong to a RenderWindow,
	* but it can also belong to a RenderTexture.
	*/
	Renderer2D(Ogre::Viewport* viewport);

	/**
	* Destructor.
	*/
	~Renderer2D();

	/**
	* Begins a queue of Rects that will be batched into 1 draw operation. All queued Rects
	* must use the same source image. Note that different sections of the image can be queued for
	* drawing.  This is useful for drawing glyph images embedded in a large font image or Texture Atlas, for example.
	* NOTE: Passing in the empty string will not use any image for queued rects, instead they will be a solid color.
	*/
	void beginRectQueue(std::string imageFile = "", std::string resourceGroupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	/**
	* Clears the RenderTarget with the color given.
	*/
	void clear(Ogre::ColourValue cv = Ogre::ColourValue::ZERO);

	/**
	* Draws a 2D line on the RenderTarget.
	* NOTE: Its important that the frameStarted variable be set to indicate whether the current viewport is being rendered to.
	*       If you are calling this method from an update loop, chances are the viewport is not currently being rendered, so frameStarted should be false.
	*       If you are calling this method in response to a RenderGroup event, chances are the viewport is currently being rendered to, so frameStarted should be true.
	* NOTE: Performing a Rendering operation will alter the RenderSystem configuration. Please see _configureRenderSystem for configuration changes.
	*/
	void drawLine(bool frameStarted, Ogre::Vector2 p1, Ogre::Vector2 p2, Ogre::ColourValue cv = Ogre::ColourValue::White, unsigned int thickness = 1);
	/**
	* Draws an image onto the RenderTarget, stretching it to meet the destination Rect area.
	* NOTE: Its important that the frameStarted variable be set to indicate whether the current viewport is being rendered to.
	*       If you are calling this method from an update loop, chances are the viewport is not currently being rendered, so frameStarted should be false.
	*       If you are calling this method in response to a RenderGroup event, chances are the viewport is currently being rendered to, so frameStarted should be true.
	* NOTE: Performing a Rendering operation will alter the RenderSystem configuration. Please see _configureRenderSystem for configuration changes.
	*/
	void drawImage(bool frameStarted, const std::string& imageFile, std::string resourceGroupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::Rect dest = Ogre::Rect(), Ogre::ColourValue cv = Ogre::ColourValue::White, Ogre::Rect source = Ogre::Rect(), int degRotation = 0, Ogre::Vector2 origin = Ogre::Vector2(-1,-1));
	/**
	* Draws a Solid rectangle onto the RenderTarget, with the color given.
	* NOTE: Its important that the frameStarted variable be set to indicate whether the current viewport is being rendered to.
	*       If you are calling this method from an update loop, chances are the viewport is not currently being rendered, so frameStarted should be false.
	*       If you are calling this method in response to a RenderGroup event, chances are the viewport is currently being rendered to, so frameStarted should be true.
	* NOTE: Performing a Rendering operation will alter the RenderSystem configuration. Please see _configureRenderSystem for configuration changes.
	*/
	void drawRect(bool frameStarted, Ogre::Rect dest = Ogre::Rect(), Ogre::ColourValue cv = Ogre::ColourValue::White, int degRotation = 0, Ogre::Vector2 origin = Ogre::Vector2(-1,-1));
	/**
	* Draws an image onto the RenderTarget, tiling it across the destination Rect area, defined by the source dimensions.
	* NOTE: Its important that the frameStarted variable be set to indicate whether the current viewport is being rendered to.
	*       If you are calling this method from an update loop, chances are the viewport is not currently being rendered, so frameStarted should be false.
	*       If you are calling this method in response to a RenderGroup event, chances are the viewport is currently being rendered to, so frameStarted should be true.
	* NOTE: Performing a Rendering operation will alter the RenderSystem configuration. Please see _configureRenderSystem for configuration changes.
	*/
	void drawTiledImage(bool frameStarted, const std::string& imageFile, std::string resourceGroupName = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::Rect dest = Ogre::Rect(), Ogre::ColourValue cv = Ogre::ColourValue::White, Ogre::Rect source = Ogre::Rect(), int degRotation = 0, Ogre::Vector2 origin = Ogre::Vector2(-1,-1));

	/**
	* Renders all Rects that have been Queued for drawing.
	* NOTE: Its important that the frameStarted variable be set to indicate whether the current viewport is being rendered to.
	*       If you are calling this method from an update loop, chances are the viewport is not currently being rendered, so frameStarted should be false.
	*       If you are calling this method in response to a RenderGroup event, chances are the viewport is currently being rendered to, so frameStarted should be true.
	* NOTE: Performing a Rendering operation will alter the RenderSystem configuration. Please see _configureRenderSystem for configuration changes.
	*/
	void endRectQueue(bool frameStarted);

	/**
	* Gets the region outlining where rendering is allowed.
	*/
	Ogre::Rect getClippingBounds();
	/**
	* Returns true if beginRectQueue(...) has been called and endRectQueue(...) has not been called, false otherwise.
	*/
	bool getRectQueueStarted();
	/**
	* Gets the RenderTarget receiving rendering output.
	*/
	Ogre::RenderTarget* getRenderTarget();

	/**
	* Queues a Rect for drawing.
	*/
	void queueRect(Ogre::Rect dest = Ogre::Rect(), Ogre::ColourValue cv = Ogre::ColourValue::White, Ogre::Rect source = Ogre::Rect(), int degRotation = 0, Ogre::Vector2 origin = Ogre::Vector2(-1,-1));

	/**
	* Sets the region outlining where rendering is allowed.
	*/
	void setClippingBounds(const Ogre::Rect& r);
	/**
	* Changes the RenderTarget and Viewport to match the RenderTexture specified.
	*/
	void setRenderTexture(Ogre::RenderTexture* texture);
	/**
	* Changes the RenderTarget and Viewport to match the viewport specified.
	*/
	void setViewport(Ogre::Viewport* viewport);

	/**
	* Writes the RenderTarget contents out to an image file.
	*/
	void writeContentsToFile(const std::string& fileName);

protected:

	// The 2D area of a RenderTarget that is affected by rendering operations.
	Ogre::Viewport* mViewport;	
	// A RenderWindow or RenderTexture, used to receive rendering operations.
	Ogre::RenderTarget* mRenderTarget;	
	// Handle to the underlying RenderSystem.
	Ogre::RenderSystem* mRenderSystem;	
	float mHorizontalTexelOffset;	
	float mVerticalTexelOffset;
	// The area of the Viewport that will show rendering operations.
	Ogre::Rect mClippingBounds;
	// All Render operations are done using the Ogre::RenderOperation class and the RenderSystem.
	Ogre::RenderOperation* mRenderOperation;	
	// 2730 = 65536 bytes / 24 bytes per vertex. (~64kb allocated for vertex buffer)
	static const int VERTEX_COUNT = 2730;	
	// The Vertex Buffer stores the vertices we want to Render, and is provided to the RenderSystem
	// via the RenderOperation class.
	Ogre::HardwareVertexBuffer* mVertexBuffer;	
	void _createVertexBuffer();
	void _destroyVertexBuffer();	
	/*
	* Define the size and data types that form a *Vertex*, to be used in the VertexBuffer.
	*/
	void _declareVertexStructure();	
	/**
	* Configures the RenderSystem so that we don't use any vertex programs, fragment programs, or other settings
	* that would affect drawing.
	*/
	void _configureRenderSystem();
	/**
	* Converts a 2d pixel position to screen coordinates. [-1,1]
	*/
	void _localize(Ogre::Vector2& vertex);
	void _rotatePoint(Ogre::Vector2& v2, int rotationInDegrees, const Ogre::Vector2& origin);
	/**
	* This method is called once the VertexBuffer has been populated.
	*/
	void _performRenderOperation(bool frameStarted, const std::string& imageName, int vertexCount);
	// members related to Queuing draw operations
	bool mQueueStarted;
	std::string mQueueImage;
	std::string mQueueImageResourceGroup;
	Vertex* mVertexBufferIndex;
	unsigned int mVertexCount;
};

#endif
