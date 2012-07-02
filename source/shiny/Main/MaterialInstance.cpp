#include "MaterialInstance.hpp"

#include "Factory.hpp"
#include "ShaderSet.hpp"

namespace sh
{
	MaterialInstance::MaterialInstance (const std::string& name, Factory* f)
		: mName(name)
		, mShadersEnabled(true)
		, mFactory(f)
	{
	}

	void MaterialInstance::setParentInstance (const std::string& name)
	{
		mParentInstance = name;
	}

	std::string MaterialInstance::getParentInstance ()
	{
		return mParentInstance;
	}

	void MaterialInstance::create (Platform* platform)
	{
		mMaterial = platform->createMaterial(mName);
	}

	void MaterialInstance::createForConfiguration (Platform* platform, const std::string& configuration)
	{
		mMaterial->createConfiguration(configuration);

		// get passes of the top-most parent
		PassVector passes = getPasses();
		for (PassVector::iterator it = passes.begin(); it != passes.end(); ++it)
		{
			boost::shared_ptr<Pass> pass = mMaterial->createPass (configuration);
			it->copyAll (pass.get(), this);

			// create all texture units
			/// \todo check usage in the shader and create only those that are necessary
			std::map<std::string, MaterialInstanceTextureUnit> texUnits = it->getTexUnits();
			for (std::map<std::string, MaterialInstanceTextureUnit>::iterator texIt = texUnits.begin(); texIt  != texUnits.end(); ++texIt )
			{
				boost::shared_ptr<TextureUnitState> texUnit = pass->createTextureUnitState ();
				texIt->second.copyAll (texUnit.get(), this);
			}

			// create or retrieve shaders
			it->setContext(this);
			ShaderSet* vertex = mFactory->getShaderSet(retrieveValue<StringValue>(it->getProperty("vertex_program"), this).get());
			ShaderInstance* v = vertex->getInstance(&*it);
			if (v)
			{
				pass->assignProgram (GPT_Vertex, v->getName());
				v->setUniformParameters (pass, &*it);
			}
			ShaderSet* fragment = mFactory->getShaderSet(retrieveValue<StringValue>(it->getProperty("fragment_program"), this).get());
			ShaderInstance* f = fragment->getInstance(&*it);
			if (f)
			{
				pass->assignProgram (GPT_Fragment, f->getName());
				f->setUniformParameters (pass, &*it);
			}
		}
	}

	void MaterialInstance::markDirty (const std::string& configuration)
	{
		mMaterial->removeConfiguration(configuration);
	}

	Material* MaterialInstance::getMaterial ()
	{
		return mMaterial.get();
	}

	MaterialInstancePass* MaterialInstance::createPass ()
	{
		mPasses.push_back (MaterialInstancePass());
		mPasses.back().setContext(this);
		return &mPasses.back();
	}

	PassVector MaterialInstance::getPasses()
	{
		if (mParent)
			return static_cast<MaterialInstance*>(mParent)->getPasses();
		else
			return mPasses;
	}

	void MaterialInstance::setShadersEnabled (bool enabled)
	{
		if (enabled == mShadersEnabled)
			return;
		mShadersEnabled = enabled;
		/// \todo
	}
}
