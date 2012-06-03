#ifndef SH_PLATFORM_H
#define SH_PLATFORM_H

#include <string>

#include "Language.hpp"

namespace sh
{
	class Factory;

	// These classes are supposed to be filled by the platform implementation
	class VertexProgram
	{
	};

	class FragmentProgram
	{
	};

	class GeometryProgram
	{
	};

	class TextureUnitState
	{
	};

	class Pass
	{
		virtual TextureUnitState& createTextureUnitState () = 0;
		virtual void assignVertexProgram (const VertexProgram& program) = 0;
		virtual void assignFragmentProgram (const FragmentProgram& program) = 0;
		virtual void assignGeometryProgram(const GeometryProgram& program) = 0;
	};

	class Material
	{
		virtual Pass& createPass () = 0;
	};

	class Platform
	{
	public:
		Platform ();
		virtual ~Platform ();

		virtual Material& createMaterial (const std::string& name) = 0;
		virtual VertexProgram& createVertexProgram (const std::string& name, const std::string& source, Language lang) = 0;
		virtual FragmentProgram& createFragmentProgram (const std::string& name, const std::string& source, Language lang) = 0;
		virtual GeometryProgram& createGeometryProgram (const std::string& name, const std::string& source, Language lang) = 0;

		/// internal use only
		void setFactory(Factory* factory);

		friend class Factory;
	protected:
		/**
		 * this will be \a true if the platform supports a listener that notifies the system
		 * whenever a material is requested for rendering. if this is supported, shaders can be
		 * compiled on-demand when needed (and not earlier)
		 */
		virtual bool supportsMaterialQueuedListener ();

		/**
		 * fire event: material requested for rendering
		 * @param name material name
		 */
		void fireMaterialRequested (const std::string& name);

	private:
		Factory* mFactory;
	};
}

#endif
