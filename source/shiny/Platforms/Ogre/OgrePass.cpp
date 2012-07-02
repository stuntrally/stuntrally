#include "OgrePass.hpp"

#include <OgrePass.h>
#include <OgreTechnique.h>

#include "OgreTextureUnitState.hpp"
#include "OgreGpuProgram.hpp"
#include "OgreMaterial.hpp"

namespace sh
{
	OgrePass::OgrePass (OgreMaterial* parent, const std::string& configuration)
		: Pass()
	{
		Ogre::Technique* t = parent->getOgreMaterial()->getTechnique(configuration);
		mPass = t->createPass();
	}

	boost::shared_ptr<TextureUnitState> OgrePass::createTextureUnitState ()
	{
		return boost::shared_ptr<TextureUnitState> (new OgreTextureUnitState (this));
	}

	void OgrePass::assignProgram (GpuProgramType type, const std::string& name)
	{
		if (type == GPT_Vertex)
			mPass->setVertexProgram (name);
		else if (type == GPT_Fragment)
			mPass->setFragmentProgram (name);
		else
			throw std::runtime_error("unsupported GpuProgramType");
	}

	Ogre::Pass* OgrePass::getOgrePass ()
	{
		return mPass;
	}

	bool OgrePass::setPropertyOverride (const std::string &name, PropertyValuePtr& value, PropertySetGet* context)
	{
		bool found = true;

		if (name == "depth_write")
			mPass->setDepthWriteEnabled(retrieveValue<BooleanValue>(value, context).get());
		else if (name == "depth_check")
			mPass->setDepthCheckEnabled(retrieveValue<BooleanValue>(value, context).get());
		else if (name == "colour_write")
			mPass->setColourWriteEnabled(retrieveValue<BooleanValue>(value, context).get());
		else if (name == "depth_bias")
		{
			Vector2 vec = retrieveValue<Vector2>(value, context);
			mPass->setDepthBias(vec.mX, vec.mY);
		}
		else if (name == "scene_blend")
		{
			std::string val = retrieveValue<StringValue>(value, context).get();
			if (val == "add")
				mPass->setSceneBlending(Ogre::SBT_ADD);
			else if (val == "modulate")
				mPass->setSceneBlending(Ogre::SBT_MODULATE);
			else if (val == "colour_blend")
				mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_COLOUR);
			else if (val == "alpha_blend")
				mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			else
				std::cerr << "sh::OgrePass: Warning: Invalid value for property \"scene_blend\"" << std::endl;
		}
		else if (name == "vertex_colour")
		{
			bool enabled = retrieveValue<BooleanValue>(value, context).get();
			// fixed-function vertex colour tracking
			mPass->setVertexColourTracking(enabled ? (Ogre::TVC_AMBIENT | Ogre::TVC_DIFFUSE | Ogre::TVC_SPECULAR) : Ogre::TVC_NONE);
		}
		else if (name == "diffuse")
		{
			Vector4 color = retrieveValue<Vector4>(value, context);
			mPass->setDiffuse(color.mX, color.mY, color.mZ, color.mW);
		}
		else if (name == "ambient")
		{
			Vector3 color = retrieveValue<Vector3>(value, context);
			mPass->setAmbient(color.mX, color.mY, color.mZ);
		}
		else if (name == "specular")
		{
			Vector4 color = retrieveValue<Vector4>(value, context);
			mPass->setSpecular(color.mX, color.mY, color.mZ, 1.0);
			mPass->setShininess(color.mW);
		}
		else
			found = false;

		return found;
	}

	void OgrePass::setGpuConstant (int type, const std::string& name, ValueType vt, PropertyValuePtr value, PropertySetGet* context)
	{
		Ogre::GpuProgramParametersSharedPtr params;
		if (type == GPT_Vertex)
			params = mPass->getVertexProgramParameters();
		else if (type == GPT_Fragment)
			params = mPass->getFragmentProgramParameters();

		if (vt == VT_Float)
			params->setNamedConstant (name, retrieveValue<FloatValue>(value, context).get());
		else if (vt == VT_Int)
			params->setNamedConstant (name, retrieveValue<IntValue>(value, context).get());
		else if (vt == VT_Vector4)
		{
			Vector4 v = retrieveValue<Vector4>(value, context);
			params->setNamedConstant (name, Ogre::Vector4(v.mX, v.mY, v.mZ, v.mW));
		}
		else if (vt == VT_Vector3)
		{
			Vector3 v = retrieveValue<Vector3>(value, context);
			params->setNamedConstant (name, Ogre::Vector4(v.mX, v.mY, v.mZ, 1.0));
		}
		else if (vt == VT_Vector2)
		{
			Vector2 v = retrieveValue<Vector2>(value, context);
			params->setNamedConstant (name, Ogre::Vector4(v.mX, v.mY, 1.0, 1.0));
		}
		else
			throw std::runtime_error ("unsupported constant type");
	}
}
