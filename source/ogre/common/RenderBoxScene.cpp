#include "pch.h"
#include <Ogre.h>
#include <MyGUI.h>
#include "RenderBoxScene.h"

namespace wraps
{
	using namespace Ogre;

	
	void RenderBoxScene::createScene()
	{
		mScene = Root::getSingleton().createSceneManager("DefaultSceneManager", MyGUI::utility::toString(this, "_SceneManagerRenderBox"));

		mNode = mScene->getRootSceneNode()->createChildSceneNode();

		///  setup light
		/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
		mScene->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
		mScene->setShadowTechnique(SHADOWTYPE_NONE);
		Vector3 dir(-1.2, -2, -0.5);
		dir.normalise();
		Light* light = mScene->createLight(MyGUI::utility::toString(this, "_LightRenderBox"));
		light->setType(Light::LT_DIRECTIONAL);
		light->setDirection(dir);
		light->setDiffuseColour( ColourValue(0.7, 0.7, 0.7));
		light->setSpecularColour(ColourValue(0.6, 0.6, 0.6));

		std::string camera(MyGUI::utility::toString(this, "_CameraRenderBox"));
		mCamera = mScene->createCamera(camera);
		mCamera->setNearClipDistance(0.1f);
		mCamera->pitch(Degree(8)); //* lower

		mCameraNode = mScene->getRootSceneNode()->createChildSceneNode(camera);
		mCameraNode->attachObject(mCamera);

		if (mCanvas->getHeight() == 0)
			mCamera->setAspectRatio(1);
		else
			mCamera->setAspectRatio( float(mCanvas->getWidth()) / float(mCanvas->getHeight()) );

		setViewport(mCamera);
	}


	void RenderBoxScene::updateViewport()
	{
		if (mCanvas->getWidth() <= 1 || mCanvas->getHeight() <= 1)
			return;

		if (mEntity != nullptr && mCamera != nullptr)
		{
			mCamera->setAspectRatio((float)mCanvas->getWidth() / (float)mCanvas->getHeight());

			AxisAlignedBox box;
			const Vector3& dpos = mEntity->getParentSceneNode()->_getDerivedPosition();
			box.merge(mEntity->getBoundingBox().getMinimum() + dpos);
			box.merge(mEntity->getBoundingBox().getMaximum() + dpos);
			if (box.isNull()) return;

			Vector3 vec = box.getSize();
			float width = sqrt(vec.x * vec.x + vec.z * vec.z), height = vec.y;
			float len2 = width / mCamera->getAspectRatio(), len1 = height;
			if (len1 < len2)  len1 = len2;
			len1 /= 0.86;  // [sqrt(3)/2] for 60 degrees field of view

			Vector3 pos = box.getCenter();
			pos.z += vec.z / 2 + len1 + 1/* min dist*/;
			pos += Vector3(0, height * 0.9f/* pitch*/, len1 * 0.1f);
			pos *= 0.85f;  //* closer
			Vector3 look = Vector3(0, box.getCenter().y * 0.8f, 0);

			mCameraNode->setPosition(pos);
			mCameraNode->lookAt(look, Node::TS_WORLD);
		}
	}

}
