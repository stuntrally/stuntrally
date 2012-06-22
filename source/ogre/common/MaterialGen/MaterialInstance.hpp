#ifndef SH_MATERIALINSTANCE_H
#define SH_MATERIALINSTANCE_H

#include "PropertyBase.hpp"
#include "Platform.hpp"

namespace sh
{
	/**
	 * @brief
	 * A specific instance of a material definition, which has all required properties set.
	 * (for example the diffuse & normal map, ambient/diffuse/specular values)
	 * Depending on these properties, the factory will automatically select a shader permutation
	 * that suits these and create the backend materials / passes (provided by the \a Platform class)
	 */
	class MaterialInstance : public PropertySetGet
	{
	public:
		MaterialInstance (const std::string& name);
		MaterialInstance ();

		void _setParentInstance (const std::string& name);
		std::string _getParentInstance ();

		void _create (Platform* platform);
		void _createForConfiguration (Platform* platform, const std::string& configuration);

		Material* getMaterial();

	private:
		std::string mParentInstance;
		///< this is only used during the file-loading phase. an instance could be loaded before its parent is loaded,
		/// so initially only the parent's name is written to this member.
		/// once all instances are loaded, the actual mParent pointer (from PropertySetGet class) can be set

		std::string mName;

		boost::shared_ptr<Material> mMaterial;
	};
}

#endif
