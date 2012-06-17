#ifndef SH_PASSDEFINITION_H
#define SH_PASSDEFINITION_H

#include <vector>

#include "PropertyBase.hpp"
#include "TextureUnitStateDefinition.hpp"

namespace sh
{
	/**
	 * @brief
	 * A single pass that belongs to a \a MaterialDefinition \n
	 * since this is just the "definition" and not the real \a Pass (provided by \a Platform),
	 * it is merely a placeholder for properties
	 */
	class PassDefinition : public PropertySetGet
	{
	public:
		TextureUnitStateDefinition* createTextureUnitStateDefinition ();

	private:
		std::vector <TextureUnitStateDefinition> mTextureUnitStates;
	};
};

#endif
