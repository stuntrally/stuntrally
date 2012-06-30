#ifndef SH_SHADERSET_H
#define SH_SHADERSET_H

#include <string>

namespace sh
{
	/**
	 * @brief Contains possible shader permutations of a single uber-shader
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

		ShaderSet (Type type, const std::string& sourceFile);
		ShaderSet (const std::string& type, const std::string& sourceFile);

	private:
		Type mType;
		std::string mSourceFile;
	};
}

#endif
