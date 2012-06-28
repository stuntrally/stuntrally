#include "MaterialInstance.hpp"

namespace sh
{
	MaterialInstance::MaterialInstance (const std::string& name)
		: mName(name)
	{
	}

	MaterialInstance::MaterialInstance ()
	{
	}

	void MaterialInstance::_setParentInstance (const std::string& name)
	{
		mParentInstance = name;
	}

	std::string MaterialInstance::_getParentInstance ()
	{
		return mParentInstance;
	}

	void MaterialInstance::_create (Platform* platform)
	{
		mMaterial = platform->createMaterial(mName);
	}

	void MaterialInstance::_createForConfiguration (Platform* platform, const std::string& configuration)
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
		}
	}

	Material* MaterialInstance::getMaterial ()
	{
		return mMaterial.get();
	}

	MaterialInstancePass* MaterialInstance::createPass ()
	{
		mPasses.push_back (MaterialInstancePass());
		return &mPasses.back();
	}

	PassVector MaterialInstance::getPasses()
	{
		if (mParent)
			return static_cast<MaterialInstance*>(mParent)->getPasses();
		else
			return mPasses;
	}
}
