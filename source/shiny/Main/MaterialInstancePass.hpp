#ifndef SH_MATERIALINSTANCEPASS_H
#define SH_MATERIALINSTANCEPASS_H

#include "PropertyBase.hpp"
#include "MaterialInstanceTextureUnit.hpp"

namespace sh
{
	/**
	 * @brief
	 * Holds properties of a single texture unit in a \a MaterialInstancePass. \n
	 * No inheritance here for now.
	 */
	class MaterialInstancePass : public PropertySetGet
	{
	public:
		MaterialInstanceTextureUnit* createTextureUnit (const std::string& name);

		std::map <std::string, MaterialInstanceTextureUnit> getTexUnits ();
	private:
		std::map <std::string, MaterialInstanceTextureUnit> mTexUnits;
	};
}

#endif
