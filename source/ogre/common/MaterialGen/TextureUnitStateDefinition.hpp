#ifndef SH_TEXTUREUNITSTATEDEFINITION_H
#define SH_TEXTUREUNITSTATEDEFINITION_H

#include "PropertyBase.hpp"

namespace sh
{
	/**
	 * @brief
	 * A single texture unit state that belongs to a \a PassDefinition \n
	 * since this is just the "definition" and not the real \a TextureUnitState (provided by \a Platform),
	 * it is merely a placeholder for properties
	 */
	class TextureUnitStateDefinition : public PropertySetGet
	{
	public:
		void setName (const std::string& name);
	private:
		std::string mName;
	};
};

#endif
