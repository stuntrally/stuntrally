#ifndef SH_MATERIALDEFINITION_H
#define SH_MATERIALDEFINITION_H

#include <vector>

#include "PassDefinition.hpp"

namespace sh
{
	/**
	 * @brief
	 * Serves as a basic template for a number of MaterialInstances.
	 * It can contain multiple passes.
	 * Each of these passes selects an uber-shader to use.
	 * Then, it can set properties which can be accessed through macros or through uniforms in the shader.
	 */
	class MaterialDefinition
	{
	public:
		PassDefinition* createPassDefinition ();

	private:
		std::vector <PassDefinition> mPasses;
	};
};

#endif
