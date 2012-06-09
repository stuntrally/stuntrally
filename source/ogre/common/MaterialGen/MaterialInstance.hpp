#ifndef SH_MATERIALINSTANCE_H
#define SH_MATERIALINSTANCE_H

namespace sh
{
	/**
	 * @brief
	 * A specific instance of a material definition, which has all required properties set.
	 * (for example the diffuse & normal map, ambient/diffuse/specular values)
	 * Depending on these properties, the factory will automatically select a shader permutation
	 * that suits these and create the backend materials / passes (provided by the \a Platform class)
	 */
	class MaterialInstance
	{
	};
}

#endif
