#ifndef SH_MATERIALINSTANCE_H
#define SH_MATERIALINSTANCE_H

#include <vector>

#include "PropertyBase.hpp"
#include "Platform.hpp"
#include "MaterialInstancePass.hpp"

namespace sh
{
	class Factory;

	typedef std::vector<MaterialInstancePass> PassVector;

	/**
	 * @brief
	 * a specific material instance, which has all required properties set
	 * (for example the diffuse & normal map, ambient/diffuse/specular values) \n
	 * Depending on these properties, the factory will automatically select a shader permutation
	 * that suits these and create the backend materials / passes (provided by the \a Platform class)
	 */
	class MaterialInstance : public PropertySetGet
	{
	public:
		MaterialInstance (const std::string& name, Factory* f);

		MaterialInstancePass* createPass ();
		PassVector getPasses(); ///< gets the passes of the top-most parent

		Material* getMaterial();

	private:
		void setParentInstance (const std::string& name);
		std::string getParentInstance ();

		void create (Platform* platform);
		void createForConfiguration (Platform* platform, const std::string& configuration);

		void markDirty (const std::string& configuration); ///< force recreating the technique/shaders when it's next used

		void setShadersEnabled (bool enabled);

		friend class Factory;


	private:
		std::string mParentInstance;
		///< this is only used during the file-loading phase. an instance could be loaded before its parent is loaded,
		/// so initially only the parent's name is written to this member.
		/// once all instances are loaded, the actual mParent pointer (from PropertySetGet class) can be set

		PassVector mPasses;

		std::string mName;

		boost::shared_ptr<Material> mMaterial;

		bool mShadersEnabled;

		Factory* mFactory;
	};
}

#endif
