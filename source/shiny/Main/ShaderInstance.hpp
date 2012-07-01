#ifndef SH_SHADERINSTANCE_H
#define SH_SHADERINSTANCE_H

#include "Platform.hpp"

namespace sh
{
	/**
	 * @brief A specific instance of a \a ShaderSet with a deterministic shader source
	 */
	class ShaderInstance
	{
	public:
		ShaderInstance (const std::string& name, std::string source, const std::string& basePath, PropertySetGet* properties);

		std::string getName();

	private:
		boost::shared_ptr<Program> mProgram;
		std::string mName;
	};
}

#endif
