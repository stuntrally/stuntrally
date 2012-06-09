#ifndef SH_PASSDEFINITION_H
#define SH_PASSDEFINITION_H

#include "PropertyBase.hpp"

namespace sh
{
	/**
	 * @brief
	 * A single pass that belongs to a \a MaterialInstance \n
	 * since this is just the "definition" and not the real \a Pass (provided by \a Platform),
	 * it is merely a placeholder for properties
	 */
	class PassDefinition : public PropertySetGet
	{
	};
};

#endif
