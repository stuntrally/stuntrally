#ifndef _Dynamic_Lines_H_
#define _Dynamic_Lines_H_

//#include "OgreSimpleRenderable.h"
//#include "OgreCamera.h"
//#include "OgreHardwareBufferManager.h"
//#include "OgreMaterialManager.h"
//#include "OgreTechnique.h"
//#include "OgrePass.h"
//#include "OgreLogManager.h"
#include <vector>

using namespace Ogre;


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
   typedef Ogre::Vector3 Vector3;
   typedef Ogre::Quaternion Quaternion;
   typedef Ogre::Camera Camera;
   typedef Ogre::Real Real;
   typedef Ogre::RenderOperation::OperationType OperationType;

private:
   std::vector<Vector3> mPoints;
   std::vector<ColourValue> mColors;
   bool mDirty;

public:
   DynamicLines(OperationType opType = Ogre::RenderOperation::OT_LINE_STRIP);
   virtual ~DynamicLines();

   void addLine(Vector3 &vectStart, Vector3 &vectEnd, ColourValue &colStart, ColourValue &colEnd);
   void addLine(Vector3 &vectStart, Vector3 &vectEnd, ColourValue &col);
   void clear();
   void update();  //  Call this to update the hardware buffer after making changes.  

   void setOperationType(OperationType opType);
   OperationType getOperationType() const;

protected:
   virtual void createVertexDeclaration();
   virtual void fillHardwareBuffers();
};

#endif