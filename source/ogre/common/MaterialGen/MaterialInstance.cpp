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

		/// \todo delete old

		// accumulate passes from all parents
		PassVector passes = getPasses();
		for (PassVector::iterator it = passes.begin(); it != passes.end(); ++it)
		{
			boost::shared_ptr<Pass> pass = mMaterial->createPass (configuration);
			it->copyAll (pass.get());
			PropertyValuePtr p = makeProperty<StringValue>(new StringValue("dsgfdgd"));
			pass->setProperty("test", p);
		}
	}

	Material* MaterialInstance::getMaterial ()
	{
		return mMaterial.get();
	}

	MaterialInstancePass* MaterialInstance::createPass ()
	{
		std::cout << "created a pass for " << mName << std::endl;
		mPasses.push_back (MaterialInstancePass());
		return &mPasses.back();
	}

	PassVector MaterialInstance::getPasses()
	{
		PassVector result = mPasses;
		if (mParent)
		{
			PassVector append = static_cast<MaterialInstance*>(mParent)->getPasses();
			result.insert(result.end(), append.begin(), append.end());
		}
		return result;
	}
}
