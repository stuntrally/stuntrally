#ifndef SH_SHADERINSTANCE_H
#define SH_SHADERINSTANCE_H

#include "Platform.hpp"

namespace sh
{
	class ShaderSet;

	/**
	 * @brief A specific instance of a \a ShaderSet with a deterministic shader source
	 */
	class ShaderInstance
	{
	public:
		ShaderInstance (ShaderSet* parent, const std::string& name, PropertySetGet* properties);

		std::string getName();

		bool getSupported () const;

	private:
		boost::shared_ptr<Program> mProgram;
		std::string mName;
		ShaderSet* mParent;
		bool mSupported; ///< shader compilation was sucessful?
	};
}

#endif
