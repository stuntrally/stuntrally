#ifndef SH_MATERIALINSTANCE_H
#define SH_MATERIALINSTANCE_H

#include "PropertyBase.hpp"

namespace sh
{
	/**
	 * @brief
	 * Holds properties of a single pass in a \a MaterialInstance. \n
	 * there are 2 types of inheritance involved here: \n
	 * - the matching MaterialInstancePass of the parent MaterialInstance (if present) \n
	 * - the PassDefinition this is based on (must be there)
	 */
	class MaterialInstancePass : public PropertySetGet
	{
	public:
		MaterialInstancePass ();

		void _setPassDefinition (PassDefinition* p);

	private:
		PassDefinition* mParentPassDefinition;
	};
}

#endif
