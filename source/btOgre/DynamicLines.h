#pragma once
#include <OgreRenderOperation.h>
#include <OgreSimpleRenderable.h>
#include <vector>


//----------------------------------------------------------------------------------------------

class DynamicRenderable : public Ogre::SimpleRenderable
{
public:
	DynamicRenderable();
	virtual ~DynamicRenderable();

	void initialize(Ogre::RenderOperation::OperationType operationType, bool useIndices);

	virtual Ogre::Real getBoundingRadius(void) const;
	virtual Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;

protected:
	size_t mVertexBufferCapacity;
	size_t mIndexBufferCapacity;

	virtual void createVertexDeclaration() = 0;
	void prepareHardwareBuffers(size_t vertexCount, size_t indexCount);
	virtual void fillHardwareBuffers() = 0;
};


//----------------------------------------------------------------------------------------------

class DynamicLines : public DynamicRenderable
{
   int iBufferSize;
private:
   std::vector<Ogre::Vector3> mPoints;
   std::vector<Ogre::ColourValue> mColors;
   bool mDirty;

public:
   DynamicLines(Ogre::RenderOperation::OperationType opType = Ogre::RenderOperation::OT_LINE_STRIP);
   virtual ~DynamicLines();

   void addLine(const Ogre::Vector3 &vectStart, const Ogre::Vector3 &vectEnd, const Ogre::ColourValue &colStart, const Ogre::ColourValue &colEnd);
   void addLine(const Ogre::Vector3 &vectStart, const Ogre::Vector3 &vectEnd, const Ogre::ColourValue &col);
   void clear();
   void update();  //  Call this to update the hardware buffer after making changes.  

   void setOperationType(Ogre::RenderOperation::OperationType opType);
   Ogre::RenderOperation::OperationType getOperationType() const;

protected:
   virtual void createVertexDeclaration();
   virtual void fillHardwareBuffers();
};
