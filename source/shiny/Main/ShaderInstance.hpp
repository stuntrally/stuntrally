#ifndef SH_SHADERINSTANCE_H
#define SH_SHADERINSTANCE_H

#include <vector>

#include "Platform.hpp"

namespace sh
{
	class ShaderSet;
/// \todo add the global settings here (and consider them in buildHash)
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
		boost::shared_ptr<GpuProgram> mProgram;
		std::string mName;
		ShaderSet* mParent;
		bool mSupported; ///< shader compilation was sucessful?

		std::map<std::string, ValueType> mUniformProperties;
		///< uniform properties that this depends on, and their value-types
		/// @note this lists shared uniform parameters as well
	};
}

#endif
