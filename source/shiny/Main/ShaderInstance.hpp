#ifndef SH_SHADERINSTANCE_H
#define SH_SHADERINSTANCE_H

#include <vector>

#include "Platform.hpp"

namespace sh
{
	class ShaderSet;

	typedef std::map< std::string, std::pair<std::string, ValueType > > UniformMap;

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

		void setUniformParameters (boost::shared_ptr<Pass> pass, PropertySetGet* properties);

	private:
		boost::shared_ptr<GpuProgram> mProgram;
		std::string mName;
		ShaderSet* mParent;
		bool mSupported; ///< shader compilation was sucessful?

		UniformMap mUniformProperties;
		///< uniforms that this depends on, and their property names / value-types
		/// @note this lists shared uniform parameters as well
	};
}

#endif
