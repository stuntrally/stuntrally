#ifndef SH_MATERIALINSTANCETEXTUREUNIT_H
#define SH_MATERIALINSTANCETEXTUREUNIT_H

#include "PropertyBase.hpp"

namespace sh
{
	/**
	 * @brief
	 * A single texture unit state that belongs to a \a MaterialInstancePass \n
	 * this is not the real "backend" \a TextureUnitState (provided by \a Platform),
	 * it is merely a placeholder for properties \n
	 * also note that the backend \a TextureUnitState will only be created if this texture unit is
	 * actually used (i.e. has a texture set)
	 */
	class MaterialInstanceTextureUnit : public PropertySetGet
	{
	public:
		MaterialInstanceTextureUnit (const std::string& name);
	private:
		std::string mName;
	};
};

#endif
