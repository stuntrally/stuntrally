#ifndef SH_OGREMATERIALSERIALIZER_H
#define SH_OGREMATERIALSERIALIZER_H

#include <OgrePrerequisites.h>

namespace Ogre
{
    struct MaterialScriptContext;
    /// Function def for material attribute parser; return value determines if the next line should be {
    typedef bool (*ATTRIBUTE_PARSER)(String& params, MaterialScriptContext& context);
}

namespace sh
{
	/**
	 * @brief This class handles the pass & texture unit properties
	 */
	class OgreMaterialSerializer
	{
	public:
        OgreMaterialSerializer();

		bool setPassProperty (const std::string& param, std::string value, Ogre::Pass* pass);
		bool setTextureUnitProperty (const std::string& param, std::string value, Ogre::TextureUnitState* t);
		bool setMaterialProperty (const std::string& param, std::string value, Ogre::MaterialPtr m);

	private:
        /// Keyword-mapped attribute parsers.
        typedef std::map<Ogre::String, Ogre::ATTRIBUTE_PARSER> AttribParserList;
        /// Parsers for the pass section of a script
        AttribParserList mPassAttribParsers;
        /// Parsers for the texture unit section of a script
        AttribParserList mTextureUnitAttribParsers;
        /// Parsers for the material section of a script
        AttribParserList mMaterialAttribParsers;
	};

}

#endif
