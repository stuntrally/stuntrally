#include "pch.h"
#include "DynamicLines.h"

#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareBufferManager.h>
#include <OgreCamera.h>

#include <cassert>
#include <cmath>

using namespace Ogre;


//  DynamicRenderable
//=============================================================================================

DynamicRenderable::DynamicRenderable() :
	mVertexBufferCapacity(0), mIndexBufferCapacity(0)
{
}
DynamicRenderable::~DynamicRenderable()
{
	delete mRenderOp.vertexData;
	delete mRenderOp.indexData;
}

//------------------------------------------------------------------------------------------------
void DynamicRenderable::initialize(RenderOperation::OperationType operationType, bool useIndices)
{
	// Initialize render operation
	mRenderOp.operationType = operationType;
	mRenderOp.useIndexes = useIndices;
	mRenderOp.vertexData = new VertexData;
	if (mRenderOp.useIndexes)
	mRenderOp.indexData = new IndexData;

	// Reset buffer capacities
	mVertexBufferCapacity = 0;
	mIndexBufferCapacity = 0;

	// Create vertex declaration
	createVertexDeclaration();
}

//------------------------------------------------------------------------------------------------
void DynamicRenderable::prepareHardwareBuffers(size_t vertexCount, size_t indexCount)
{
	// Prepare vertex buffer
	size_t newVertCapacity = mVertexBufferCapacity;
	if ((vertexCount > mVertexBufferCapacity) ||
		(!mVertexBufferCapacity))
	{
		// vertexCount exceeds current capacity!
		// It is necessary to reallocate the buffer.

		// Check if this is the first call
		if (!newVertCapacity)
			newVertCapacity = 1;

		// Make capacity the next power of two
		while (newVertCapacity < vertexCount)
			newVertCapacity <<= 1;
	}else
	if (vertexCount < mVertexBufferCapacity>>1)  {
		// Make capacity the previous power of two
		while (vertexCount < newVertCapacity>>1)
			newVertCapacity >>= 1;	}

	if (newVertCapacity != mVertexBufferCapacity) 
	{
		mVertexBufferCapacity = newVertCapacity;
		// Create new vertex buffer
		HardwareVertexBufferSharedPtr vbuf =
			HardwareBufferManager::getSingleton().createVertexBuffer(
			mRenderOp.vertexData->vertexDeclaration->getVertexSize(0)*2,
			mVertexBufferCapacity,
			HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // TODO: Custom HBU_?

		// Bind buffer
		mRenderOp.vertexData->vertexBufferBinding->setBinding(0, vbuf);
	}
	
	// Update vertex count in the render operation
	mRenderOp.vertexData->vertexCount = vertexCount;

	if (mRenderOp.useIndexes)
	{
		OgreAssert(indexCount <= std::numeric_limits<unsigned short>::max(), "indexCount exceeds 16 bit");

		size_t newIndexCapacity = mIndexBufferCapacity;
		// Prepare index buffer
		if ((indexCount > newIndexCapacity) ||
			(!newIndexCapacity))
		{
			// indexCount exceeds current capacity!
			// It is necessary to reallocate the buffer.

			// Check if this is the first call
			if (!newIndexCapacity)
			newIndexCapacity = 1;

			// Make capacity the next power of two
			while (newIndexCapacity < indexCount)
				newIndexCapacity <<= 1;

		}
		else if (indexCount < newIndexCapacity>>1) 
		{
			// Make capacity the previous power of two
			while (indexCount < newIndexCapacity>>1)
				newIndexCapacity >>= 1;
		}

		if (newIndexCapacity != mIndexBufferCapacity)
		{
			mIndexBufferCapacity = newIndexCapacity;
			// Create new index buffer
			mRenderOp.indexData->indexBuffer =
			HardwareBufferManager::getSingleton().createIndexBuffer(
				HardwareIndexBuffer::IT_16BIT, mIndexBufferCapacity,
				HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // TODO: Custom HBU_?
		}

		// Update index count in the render operation
		mRenderOp.indexData->indexCount = indexCount;
	}
}
//------------------------------------------------------------------------------------------------

Real DynamicRenderable::getBoundingRadius(void) const
{
	return Math::Sqrt(std::max(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
}

Real DynamicRenderable::getSquaredViewDepth(const Camera* cam) const
{
	 Vector3 vMin, vMax, vMid, vDist;
	 vMin = mBox.getMinimum();
	 vMax = mBox.getMaximum();
	 vMid = ((vMax - vMin) * 0.5) + vMin;
	 vDist = cam->getDerivedPosition() - vMid;

	 return vDist.squaredLength();
}


//  DynamicRenderable
//=============================================================================================

DynamicLines::DynamicLines(RenderOperation::OperationType opType)
{
	initialize(opType, false);
	mDirty = true;

	iBufferSize = 2;
	prepareHardwareBuffers(iBufferSize, 0);

	//	Temp fix to remove clipping
	Vector3 vaabMin = Vector3(-1000, -1000, -1000);
	Vector3 vaabMax = Vector3(1000, 1000, 1000);
	mBox.setExtents(vaabMin, vaabMax);
}

DynamicLines::~DynamicLines()
{
}

void DynamicLines::setOperationType(RenderOperation::OperationType opType)
{
	mRenderOp.operationType = opType;
}

RenderOperation::OperationType DynamicLines::getOperationType() const
{
	return mRenderOp.operationType;
}

//---------------------------------------------------
void DynamicLines::addLine(const Vector3 &vectStart, const Vector3 &vectEnd, const ColourValue &colStart, const ColourValue &colEnd)
{
	mPoints.push_back(vectStart);
	mPoints.push_back(vectEnd);
	mColors.push_back(colStart);
	mColors.push_back(colEnd);
	mDirty = true;
}

void DynamicLines::addLine(const Vector3 &vectStart, const Vector3 &vectEnd, const ColourValue &col)
{
	mPoints.push_back(vectStart);
	mPoints.push_back(vectEnd);
	mColors.push_back(col);
	mColors.push_back(col);
	mDirty = true;
}


void DynamicLines::clear()
{
	mPoints.clear();
	mColors.clear();
	mDirty = true;
}

void DynamicLines::update()
{
	if(mDirty)
		fillHardwareBuffers();
}


void DynamicLines::createVertexDeclaration()
{
	VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
	decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
	decl->addElement(0, Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3), VET_COLOUR, VES_DIFFUSE);
}

void DynamicLines::fillHardwareBuffers()
{
	int size = mPoints.size();

	//	Double buffer size if we have reach the maximum size
	if(iBufferSize < size)
	{
		iBufferSize = size * 2;
		prepareHardwareBuffers(iBufferSize, 0);
	}
  
	unsigned char *pVert;
	float *pFloat;
	Ogre::ARGB *pCol;

	HardwareVertexBufferSharedPtr vbuf =
		mRenderOp.vertexData->vertexBufferBinding->getBuffer(0);

	pVert = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	Ogre::VertexDeclaration::VertexElementList elems = mRenderOp.vertexData->vertexDeclaration->findElementsBySource(0);
	Ogre::VertexDeclaration::VertexElementList::const_iterator elemItr = elems.begin(), elemEnd = elems.end();

	for(int i = 0; i < iBufferSize/*size*/; ++i)
	{
		for (elemItr = elems.begin(); elemItr != elemEnd; ++elemItr)
		{
			const Ogre::VertexElement& elem = *elemItr;
			switch (elem.getSemantic())
			{
				case Ogre::VES_POSITION:
					elem.baseVertexPointerToElement(pVert, &pFloat);
					if(i < size)
					{
						*pFloat++ = mPoints[i].x;
						*pFloat++ = mPoints[i].y;
						*pFloat++ = mPoints[i].z;
					} else
					{
						*pFloat++ = 0.0f;
						*pFloat++ = 0.0f;
						*pFloat++ = 0.0f;
					}
				break;
				case Ogre::VES_DIFFUSE:
					elem.baseVertexPointerToElement(pVert, &pCol);
					{
						if(i < size)
							*pCol++ = Ogre::VertexElement::convertColourValue(Ogre::ColourValue(mColors[i].r, mColors[i].g, mColors[i].b, mColors[i].a), Ogre::VET_COLOUR_ARGB);
						else
							*pCol++ = Ogre::VertexElement::convertColourValue(Ogre::ColourValue(0.0, 0.0, 0.0, 0.0), Ogre::VET_COLOUR_ARGB);
					}
				break;
			}
		}
		pVert += vbuf->getVertexSize();
	}
	vbuf->unlock();
	mDirty = false;
	clear();
}
