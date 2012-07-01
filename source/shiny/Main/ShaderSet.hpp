#ifndef SH_SHADERSET_H
#define SH_SHADERSET_H

#include <string>
#include <vector>
#include <map>

#include "ShaderInstance.hpp"

namespace sh
{
	class PropertySetGet;

	typedef std::map<size_t, ShaderInstance> ShaderInstanceMap;

	/**
	 * @brief Contains possible shader permutations of a single uber-shader (represented by one source file)
	 */
	class ShaderSet
	{
	public:
		enum Type
		{
			Type_Vertex,
			Type_Fragment
			//Type_Geometry
		};

		ShaderSet (Type type, const std::string& sourceFile, const std::string& basePath, const std::string& name);
		ShaderSet (const std::string& type, const std::string& sourceFile, const std::string& basePath, const std::string& name);

		/// retrieve a shader instance for the given properties. \n
		/// if a \a ShaderInstance with the same properties exists already, simply returns this instance. \n
		/// otherwise, creates a new \a ShaderInstance (i.e. compiles a new shader) \n
		/// @note only the properties that actually affect the shader source are taken into consideration here,
		/// so it does not matter if you pass any extra properties that the shader does not care about.
		ShaderInstance* getInstance (PropertySetGet* properties);

	private:
		Type mType;
		std::string mSource;
		std::string mBasePath;
		std::string mName;

		std::vector <std::string> mGlobalSettings; ///< names of the global settings that affect the shader source
		std::vector <std::string> mProperties; ///< names of the per-material properties that affect the shader source

		ShaderInstanceMap mInstances; ///< maps permutation ID (generated from the properties) to \a ShaderInstance

		void parse(); ///< find out which properties and global settings affect the shader source

		size_t buildHash (PropertySetGet* properties);
	};
}

#endif
