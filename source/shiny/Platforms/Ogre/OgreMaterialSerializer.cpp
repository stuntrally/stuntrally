#include "pch.h"
#include "OgreMaterialSerializer.hpp"

#include <OgrePass.h>

#include <OgreStringConverter.h>
#include <OgreLogManager.h>

//#if 0  // for Ogre 1.9 only
#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 12, 0)
namespace Ogre
{
    template <typename T> class MapIterator;
    /** Struct for holding the script context while parsing. */
    struct MaterialScriptContext
    {
        MaterialPtr material;
        Pass* pass;
        TextureUnitState* textureUnit;
        AliasTextureNamePairList textureAliases;
    };
}
#endif

#include <OgreLodStrategyManager.h>
#include <OgreDistanceLodStrategy.h>

//-----------------------------------------------------------------------
// Internal parser methods
//-----------------------------------------------------------------------
namespace 
{
    using namespace Ogre;

    void logParseError(const String& error, const MaterialScriptContext& context)
    {
        // log material name only if filename not specified
        if (context.material.get())
        {
            LogManager::getSingleton().logMessage(
                "Error in material " + context.material->getName() +
                " : " + error, LML_CRITICAL);
        }
        else
        {
            LogManager::getSingleton().logMessage("Error: "+error, LML_CRITICAL);
        }
    }
    void trimAndUnquoteWord(String& val)
    {
        StringUtil::trim(val);
        if(val.size() >= 2 && val[0] == '\"' && val[val.size() - 1] == '\"')
            val = val.substr(1, val.size() - 2);
    }
    ColourValue _parseColourValue(StringVector& vecparams)
    {
        return ColourValue(
            StringConverter::parseReal(vecparams[0]) ,
            StringConverter::parseReal(vecparams[1]) ,
            StringConverter::parseReal(vecparams[2]) ,
            (vecparams.size()==4) ? StringConverter::parseReal(vecparams[3]) : 1.0f ) ;
    }
    //-----------------------------------------------------------------------
    FilterOptions convertFiltering(const String& s)
    {
        if (s == "none")
        {
            return FO_NONE;
        }
        else if (s == "point")
        {
            return FO_POINT;
        }
        else if (s == "linear")
        {
            return FO_LINEAR;
        }
        else if (s == "anisotropic")
        {
            return FO_ANISOTROPIC;
        }

        return FO_POINT;
    }
    bool parseLodValues(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");

        // iterate over the parameters and parse values out of them
        Material::LodValueList lodList;
        StringVector::iterator i, iend;
        iend = vecparams.end();
        for (i = vecparams.begin(); i != iend; ++i)
        {
            lodList.push_back(StringConverter::parseReal(*i));
        }

        context.material->setLodLevels(lodList);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodStrategy(String& params, MaterialScriptContext& context)
    {
        LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(params);
        
        if (strategy == 0)
            logParseError(
            "Bad lod_strategy attribute, available LOD strategy name expected.",
            context);

        context.material->setLodStrategy(strategy);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodDistances(String& params, MaterialScriptContext& context)
    {
        // Set to distance strategy
        context.material->setLodStrategy(DistanceLodSphereStrategy::getSingletonPtr());

        StringVector vecparams = StringUtil::split(params, " \t");

        // iterate over the parameters and parse values out of them
        Material::LodValueList lodList;
        StringVector::iterator i, iend;
        iend = vecparams.end();
        for (i = vecparams.begin(); i != iend; ++i)
        {
            lodList.push_back(StringConverter::parseReal(*i));
        }

        context.material->setLodLevels(lodList);

        return false;
    }
    bool parseReceiveShadows(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.material->setReceiveShadows(true);
        else if (params == "off")
            context.material->setReceiveShadows(false);
        else
            logParseError(
            "Bad receive_shadows attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;

    }
    bool parseTransparencyCastsShadows(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.material->setTransparencyCastsShadows(true);
        else if (params == "off")
            context.material->setTransparencyCastsShadows(false);
        else
            logParseError(
            "Bad transparency_casts_shadows attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;
    }
    bool parseSetTextureAlias(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Wrong number of parameters for texture_alias, expected 2", context);
            return false;
        }
        // first parameter is alias name and second parameter is texture name
        context.textureAliases[vecparams[0]] = vecparams[1];

        return false;
    }
    bool parseAmbient(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_AMBIENT);
            } else {
                logParseError(
                    "Bad ambient attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setAmbient( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_AMBIENT);
        }
        else
        {
            logParseError(
                "Bad ambient attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }
        return false;
    }
   //-----------------------------------------------------------------------
    bool parseDiffuse(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_DIFFUSE);
            } else {
                logParseError(
                    "Bad diffuse attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setDiffuse( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_DIFFUSE);
        }
        else
        {
            logParseError(
                "Bad diffuse attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }        return false;
    }
    //-----------------------------------------------------------------------
    bool parseSpecular(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 2, 4 or 5 parameters
        if(vecparams.size() == 2)
        {
            if(vecparams[0] == "vertexcolour") {
                context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_SPECULAR);
                context.pass->setShininess(StringConverter::parseReal(vecparams[1]) );
            }
            else
            {
                logParseError(
                    "Bad specular attribute, double parameter statement must be 'vertexcolour <shininess>'",
                    context);
            }
        }
        else if(vecparams.size() == 4 || vecparams.size() == 5)
        {
            context.pass->setSpecular(
                StringConverter::parseReal(vecparams[0]),
                StringConverter::parseReal(vecparams[1]),
                StringConverter::parseReal(vecparams[2]),
                vecparams.size() == 5?
                    StringConverter::parseReal(vecparams[3]) : 1.0f);
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_SPECULAR);
            context.pass->setShininess(
                StringConverter::parseReal(vecparams[vecparams.size() - 1]) );
        }
        else
        {
            logParseError(
                "Bad specular attribute, wrong number of parameters (expected 2, 4 or 5)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEmissive(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_EMISSIVE);
            } else {
                logParseError(
                    "Bad emissive attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setSelfIllumination( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_EMISSIVE);
        }
        else
        {
            logParseError(
                "Bad emissive attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor convertBlendFactor(const String& param)
    {
        if (param == "one")
            return SBF_ONE;
        else if (param == "zero")
            return SBF_ZERO;
        else if (param == "dest_colour")
            return SBF_DEST_COLOUR;
        else if (param == "src_colour")
            return SBF_SOURCE_COLOUR;
        else if (param == "one_minus_dest_colour")
            return SBF_ONE_MINUS_DEST_COLOUR;
        else if (param == "one_minus_src_colour")
            return SBF_ONE_MINUS_SOURCE_COLOUR;
        else if (param == "dest_alpha")
            return SBF_DEST_ALPHA;
        else if (param == "src_alpha")
            return SBF_SOURCE_ALPHA;
        else if (param == "one_minus_dest_alpha")
            return SBF_ONE_MINUS_DEST_ALPHA;
        else if (param == "one_minus_src_alpha")
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend factor.", "convertBlendFactor");
        }


    }
    //-----------------------------------------------------------------------
    bool parseSceneBlend(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Should be 1 or 2 params
        if (vecparams.size() == 1)
        {
            //simple
            SceneBlendType stype;
            if (vecparams[0] == "add")
                stype = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stype = SBT_MODULATE;
            else if (vecparams[0] == "colour_blend")
                stype = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stype = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad scene_blend attribute, unrecognised parameter '" + vecparams[0] + "'",
                    context);
                return false;
            }
            context.pass->setSceneBlending(stype);

        }
        else if (vecparams.size() == 2)
        {
            //src/dest
            SceneBlendFactor src, dest;

            try {
                src = convertBlendFactor(vecparams[0]);
                dest = convertBlendFactor(vecparams[1]);
                context.pass->setSceneBlending(src,dest);
            }
            catch (Exception& e)
            {
                logParseError("Bad scene_blend attribute, " + e.getDescription(), context);
            }

        }
        else
        {
            logParseError(
                "Bad scene_blend attribute, wrong number of parameters (expected 1 or 2)",
                context);
        }

        return false;

    }
    //-----------------------------------------------------------------------
    bool parseSeparateSceneBlend(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Should be 2 or 4 params
        if (vecparams.size() == 2)
        {
            //simple
            SceneBlendType stype;
            if (vecparams[0] == "add")
                stype = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stype = SBT_MODULATE;
            else if (vecparams[0] == "colour_blend")
                stype = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stype = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad separate_scene_blend attribute, unrecognised parameter '" + vecparams[0] + "'",
                    context);
                return false;
            }

            SceneBlendType stypea;
            if (vecparams[0] == "add")
                stypea = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stypea = SBT_MODULATE;
            else if (vecparams[0] == "colour_blend")
                stypea = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stypea = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad separate_scene_blend attribute, unrecognised parameter '" + vecparams[1] + "'",
                    context);
                return false;
            }
            
            context.pass->setSeparateSceneBlending(stype, stypea);
        }
        else if (vecparams.size() == 4)
        {
            //src/dest
            SceneBlendFactor src, dest;
            SceneBlendFactor srca, desta;

            try {
                src = convertBlendFactor(vecparams[0]);
                dest = convertBlendFactor(vecparams[1]);
                srca = convertBlendFactor(vecparams[2]);
                desta = convertBlendFactor(vecparams[3]);
                context.pass->setSeparateSceneBlending(src,dest,srca,desta);
            }
            catch (Exception& e)
            {
                logParseError("Bad separate_scene_blend attribute, " + e.getDescription(), context);
            }

        }
        else
        {
            logParseError(
                "Bad separate_scene_blend attribute, wrong number of parameters (expected 2 or 4)",
                context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    CompareFunction convertCompareFunction(const String& param)
    {
        if (param == "always_fail")
            return CMPF_ALWAYS_FAIL;
        else if (param == "always_pass")
            return CMPF_ALWAYS_PASS;
        else if (param == "less")
            return CMPF_LESS;
        else if (param == "less_equal")
            return CMPF_LESS_EQUAL;
        else if (param == "equal")
            return CMPF_EQUAL;
        else if (param == "not_equal")
            return CMPF_NOT_EQUAL;
        else if (param == "greater_equal")
            return CMPF_GREATER_EQUAL;
        else if (param == "greater")
            return CMPF_GREATER;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare function", "convertCompareFunction");

    }
    //-----------------------------------------------------------------------
    bool parseDepthCheck(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthCheckEnabled(true);
        else if (params == "off")
            context.pass->setDepthCheckEnabled(false);
        else
            logParseError(
            "Bad depth_check attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseDepthWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthWriteEnabled(true);
        else if (params == "off")
            context.pass->setDepthWriteEnabled(false);
        else
            logParseError(
                "Bad depth_write attribute, valid parameters are 'on' or 'off'.",
                context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLightScissor(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setLightScissoringEnabled(true);
        else if (params == "off")
            context.pass->setLightScissoringEnabled(false);
        else
            logParseError(
            "Bad light_scissor attribute, valid parameters are 'on' or 'off'.",
            context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLightClip(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setLightClipPlanesEnabled(true);
        else if (params == "off")
            context.pass->setLightClipPlanesEnabled(false);
        else
            logParseError(
            "Bad light_clip_planes attribute, valid parameters are 'on' or 'off'.",
            context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseDepthFunc(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try {
            CompareFunction func = convertCompareFunction(params);
            context.pass->setDepthFunction(func);
        }
        catch (...)
        {
            logParseError("Bad depth_func attribute, invalid function parameter.", context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseNormaliseNormals(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setNormaliseNormals(true);
        else if (params == "off")
            context.pass->setNormaliseNormals(false);
        else
            logParseError(
            "Bad normalise_normals attribute, valid parameters are 'on' or 'off'.",
            context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setColourWriteEnabled(true);
        else if (params == "off")
            context.pass->setColourWriteEnabled(false);
        else
            logParseError(
                "Bad colour_write attribute, valid parameters are 'on' or 'off'.",
                context);
        return false;
    }

    //-----------------------------------------------------------------------
    bool parseCullHardware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setCullingMode(CULL_NONE);
        else if (params=="anticlockwise")
            context.pass->setCullingMode(CULL_ANTICLOCKWISE);
        else if (params=="clockwise")
            context.pass->setCullingMode(CULL_CLOCKWISE);
        else
            logParseError(
                "Bad cull_hardware attribute, valid parameters are "
                "'none', 'clockwise' or 'anticlockwise'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseCullSoftware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setManualCullingMode(MANUAL_CULL_NONE);
        else if (params=="back")
            context.pass->setManualCullingMode(MANUAL_CULL_BACK);
        else if (params=="front")
            context.pass->setManualCullingMode(MANUAL_CULL_FRONT);
        else
            logParseError(
                "Bad cull_software attribute, valid parameters are 'none', "
                "'front' or 'back'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLighting(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="on")
            context.pass->setLightingEnabled(true);
        else if (params=="off")
            context.pass->setLightingEnabled(false);
        else
            logParseError(
                "Bad lighting attribute, valid parameters are 'on' or 'off'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMaxLights(String& params, MaterialScriptContext& context)
    {
        context.pass->setMaxSimultaneousLights((ushort)StringConverter::parseInt(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseStartLight(String& params, MaterialScriptContext& context)
    {
        context.pass->setStartLight((ushort)StringConverter::parseInt(params));
        return false;
    }
    //-----------------------------------------------------------------------
    void parseIterationLightTypes(String& params, MaterialScriptContext& context)
    {
        // Parse light type
        if (params == "directional")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
        }
        else if (params == "point")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_POINT);
        }
        else if (params == "spot")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
        }
        else
        {
            logParseError("Bad iteration attribute, valid values for light type parameter "
                "are 'point' or 'directional' or 'spot'.", context);
        }

    }
    //-----------------------------------------------------------------------
    bool parseIteration(String& params, MaterialScriptContext& context)
    {
        // we could have more than one parameter
        /** combinations could be:
            iteration once
            iteration once_per_light [light type]
            iteration <number>
            iteration <number> [per_light] [light type]
            iteration <number> [per_n_lights] <num_lights> [light type]
        */
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 1 || vecparams.size() > 4)
        {
            logParseError("Bad iteration attribute, expected 1 to 3 parameters.", context);
            return false;
        }

        if (vecparams[0]=="once")
            context.pass->setIteratePerLight(false, false);
        else if (vecparams[0]=="once_per_light")
        {
            if (vecparams.size() == 2)
            {
                parseIterationLightTypes(vecparams[1], context);
            }
            else
            {
                context.pass->setIteratePerLight(true, false);
            }

        }
        else // could be using form: <number> [per_light] [light type]
        {
            int passIterationCount = StringConverter::parseInt(vecparams[0]);
            if (passIterationCount > 0)
            {
                context.pass->setPassIterationCount(passIterationCount);
                if (vecparams.size() > 1)
                {
                    if (vecparams[1] == "per_light")
                    {
                        if (vecparams.size() == 3)
                        {
                            parseIterationLightTypes(vecparams[2], context);
                        }
                        else
                        {
                            context.pass->setIteratePerLight(true, false);
                        }
                    }
                    else if (vecparams[1] == "per_n_lights")
                    {
                        if (vecparams.size() < 3)
                        {
                            logParseError(
                                "Bad iteration attribute, expected number of lights.", 
                                context);
                        }
                        else
                        {
                            // Parse num lights
                            context.pass->setLightCountPerIteration(
                                (ushort)StringConverter::parseInt(vecparams[2]));
                            // Light type
                            if (vecparams.size() == 4)
                            {
                                parseIterationLightTypes(vecparams[3], context);
                            }
                            else
                            {
                                context.pass->setIteratePerLight(true, false);
                            }
                        }
                    }
                    else
                        logParseError(
                            "Bad iteration attribute, valid parameters are <number> [per_light|per_n_lights <num_lights>] [light type].", context);
                }
            }
            else
                logParseError(
                    "Bad iteration attribute, valid parameters are 'once' or 'once_per_light' or <number> [per_light|per_n_lights <num_lights>] [light type].", context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSize(String& params, MaterialScriptContext& context)
    {
        context.pass->setPointSize(StringConverter::parseReal(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSprites(String& params, MaterialScriptContext& context)
    {
        if (params=="on")
            context.pass->setPointSpritesEnabled(true);
        else if (params=="off")
            context.pass->setPointSpritesEnabled(false);
        else
            logParseError(
                "Bad point_sprites attribute, valid parameters are 'on' or 'off'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointAttenuation(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 1 && vecparams.size() != 4)
        {
            logParseError("Bad point_size_attenuation attribute, 1 or 4 parameters expected", context);
            return false;
        }
        if (vecparams[0] == "off")
        {
            context.pass->setPointAttenuation(false);
        }
        else if (vecparams[0] == "on")
        {
            if (vecparams.size() == 4)
            {
                context.pass->setPointAttenuation(true,
                    StringConverter::parseReal(vecparams[1]),
                    StringConverter::parseReal(vecparams[2]),
                    StringConverter::parseReal(vecparams[3]));
            }
            else
            {
                context.pass->setPointAttenuation(true);
            }
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSizeMin(String& params, MaterialScriptContext& context)
    {
        context.pass->setPointMinSize(
            StringConverter::parseReal(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSizeMax(String& params, MaterialScriptContext& context)
    {
        context.pass->setPointMaxSize(
            StringConverter::parseReal(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseFogging(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams[0]=="true")
        {
            // if true, we need to see if they supplied all arguments, or just the 1... if just the one,
            // Assume they want to disable the default fog from effecting this material.
            if( vecparams.size() == 8 )
            {
                FogMode mFogtype;
                if( vecparams[1] == "none" )
                    mFogtype = FOG_NONE;
                else if( vecparams[1] == "linear" )
                    mFogtype = FOG_LINEAR;
                else if( vecparams[1] == "exp" )
                    mFogtype = FOG_EXP;
                else if( vecparams[1] == "exp2" )
                    mFogtype = FOG_EXP2;
                else
                {
                    logParseError(
                        "Bad fogging attribute, valid parameters are "
                        "'none', 'linear', 'exp', or 'exp2'.", context);
                    return false;
                }

                context.pass->setFog(
                    true,
                    mFogtype,
                    ColourValue(
                    StringConverter::parseReal(vecparams[2]),
                    StringConverter::parseReal(vecparams[3]),
                    StringConverter::parseReal(vecparams[4])),
                    StringConverter::parseReal(vecparams[5]),
                    StringConverter::parseReal(vecparams[6]),
                    StringConverter::parseReal(vecparams[7])
                    );
            }
            else
            {
                context.pass->setFog(true);
            }
        }
        else if (vecparams[0]=="false")
            context.pass->setFog(false);
        else
            logParseError(
                "Bad fog_override attribute, valid parameters are 'true' or 'false'.",
                context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseShading(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="flat")
            context.pass->setShadingMode(SO_FLAT);
        else if (params=="gouraud")
            context.pass->setShadingMode(SO_GOURAUD);
        else if (params=="phong")
            context.pass->setShadingMode(SO_PHONG);
        else
            logParseError("Bad shading attribute, valid parameters are 'flat', "
                "'gouraud' or 'phong'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePolygonMode(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="solid")
            context.pass->setPolygonMode(PM_SOLID);
        else if (params=="wireframe")
            context.pass->setPolygonMode(PM_WIREFRAME);
        else if (params=="points")
            context.pass->setPolygonMode(PM_POINTS);
        else
            logParseError("Bad polygon_mode attribute, valid parameters are 'solid', "
            "'wireframe' or 'points'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePolygonModeOverrideable(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);

        context.pass->setPolygonModeOverrideable(
            StringConverter::parseBool(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseFiltering(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1 or 3 parameters
        if (vecparams.size() == 1)
        {
            // Simple format
            if (vecparams[0]=="none")
                context.textureUnit->setTextureFiltering(TFO_NONE);
            else if (vecparams[0]=="bilinear")
                context.textureUnit->setTextureFiltering(TFO_BILINEAR);
            else if (vecparams[0]=="trilinear")
                context.textureUnit->setTextureFiltering(TFO_TRILINEAR);
            else if (vecparams[0]=="anisotropic")
                context.textureUnit->setTextureFiltering(TFO_ANISOTROPIC);
            else
            {
                logParseError("Bad filtering attribute, valid parameters for simple format are "
                    "'none', 'bilinear', 'trilinear' or 'anisotropic'.", context);
                return false;
            }
        }
        else if (vecparams.size() == 3)
        {
            // Complex format
            context.textureUnit->setTextureFiltering(
                convertFiltering(vecparams[0]),
                convertFiltering(vecparams[1]),
                convertFiltering(vecparams[2]));
        }
        else
        {
            logParseError(
                "Bad filtering attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseCompareTest(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try 
        {
            if(params == "on")
            {
                context.textureUnit->setTextureCompareEnabled(true);
            }
            else if(params == "off")
            {
                context.textureUnit->setTextureCompareEnabled(false);
            }
            else
            {
                  OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare setting", "parseCompareEnabled");
            }
        }
        catch (...)
        {
            logParseError("Bad compare_test attribute, invalid function parameter.", context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseCompareFunction(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try {
            CompareFunction func = convertCompareFunction(params);
            context.textureUnit->setTextureCompareFunction(func);
        }
        catch (...)
        {
            logParseError("Bad compare_func attribute, invalid function parameter.", context);
        }

        return false;
    }
    bool parseAlphaRejection(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError(
                "Bad alpha_rejection attribute, wrong number of parameters (expected 2)",
                context);
            return false;
        }

        CompareFunction cmp;
        try {
            cmp = convertCompareFunction(vecparams[0]);
        }
        catch (...)
        {
            logParseError("Bad alpha_rejection attribute, invalid compare function.", context);
            return false;
        }

        context.pass->setAlphaRejectSettings(cmp, (unsigned char)StringConverter::parseInt(vecparams[1]));

        return false;
    }
    //---------------------------------------------------------------------
    bool parseAlphaToCoverage(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setAlphaToCoverageEnabled(true);
        else if (params == "off")
            context.pass->setAlphaToCoverageEnabled(false);
        else
            logParseError(
            "Bad alpha_to_coverage attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTransparentSorting(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setTransparentSortingEnabled(true);
        else if (params == "off")
            context.pass->setTransparentSortingEnabled(false);
        else if (params == "force")
            context.pass->setTransparentSortingForced(true);
        else
            logParseError(
            "Bad transparent_sorting attribute, valid parameters are 'on', 'off' or 'force'.",
            context);

        return false;
    }
    bool parseDepthBias(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");

        float constantBias = static_cast<float>(StringConverter::parseReal(vecparams[0]));
        float slopeScaleBias = 0.0f;
        if (vecparams.size() > 1)
        {
            slopeScaleBias = static_cast<float>(StringConverter::parseReal(vecparams[1]));
        }
        context.pass->setDepthBias(constantBias, slopeScaleBias);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseIterationDepthBias(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");

        float bias = static_cast<float>(StringConverter::parseReal(vecparams[0]));
        context.pass->setIterationDepthBias(bias);

        return false;
    }
    bool parseIlluminationStage(String& params, MaterialScriptContext& context)
    {
        if (params == "ambient")
        {
            context.pass->setIlluminationStage(IS_AMBIENT);
        }
        else if (params == "per_light")
        {
            context.pass->setIlluminationStage(IS_PER_LIGHT);
        }
        else if (params == "decal")
        {
            context.pass->setIlluminationStage(IS_DECAL);
        }
        else
        {
            logParseError("Invalid illumination_stage specified.", context);
        }
        return false;
    }
    // Texture layer attributes
    bool parseTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::tokenise(params, " \t");
        const size_t numParams = vecparams.size();
        if (numParams > 5)
        {
            logParseError("Invalid texture attribute - expected only up to 5 parameters.",
                context);
        }
        TextureType tt = TEX_TYPE_2D;
        int mipmaps = MIP_DEFAULT; // When passed to TextureManager::load, this means default to default number of mipmaps
        bool isAlpha = false;
        bool hwGamma = false;
        PixelFormat desiredFormat = PF_UNKNOWN;
        for (size_t p = 1; p < numParams; ++p)
        {
            StringUtil::toLowerCase(vecparams[p]);
            if (vecparams[p] == "1d")
            {
                tt = TEX_TYPE_1D;
            }
            else if (vecparams[p] == "2d")
            {
                tt = TEX_TYPE_2D;
            }
            else if (vecparams[p] == "3d")
            {
                tt = TEX_TYPE_3D;
            }
            else if (vecparams[p] == "cubic")
            {
                tt = TEX_TYPE_CUBE_MAP;
            }
            else if (vecparams[p] == "unlimited")
            {
                mipmaps = MIP_UNLIMITED;
            }
            else if (StringConverter::isNumber(vecparams[p]))
            {
                mipmaps = StringConverter::parseInt(vecparams[p]);
            }
            else if (vecparams[p] == "alpha")
            {
                isAlpha = true;
            }
            else if (vecparams[p] == "gamma")
            {
                hwGamma = true;
            }
            else if ((desiredFormat = PixelUtil::getFormatFromName(vecparams[p], true)) != PF_UNKNOWN)
            {
                // nothing to do here
            }
            else
            {
                logParseError("Invalid texture option - "+vecparams[p]+".",
                context);
            }
        }

        context.textureUnit->setTextureName(vecparams[0], tt);
        context.textureUnit->setNumMipmaps(mipmaps);
        context.textureUnit->setIsAlpha(isAlpha);
        context.textureUnit->setDesiredFormat(desiredFormat);
        context.textureUnit->setHardwareGammaEnabled(hwGamma);
        return false;
    }
    bool parseAnimTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::tokenise(params, " \t");
        size_t numParams = vecparams.size();
        // Determine which form it is
        // Must have at least 3 params though
        if (numParams < 3)
        {
            logParseError("Bad anim_texture attribute, wrong number of parameters "
                "(expected at least 3)", context);
            return false;
        }
        if (numParams == 3 && StringConverter::parseInt(vecparams[1]) != 0 )
        {
            // First form using base name & number of frames
            context.textureUnit->setAnimatedTextureName(
                vecparams[0],
                StringConverter::parseInt(vecparams[1]),
                StringConverter::parseReal(vecparams[2]));
        }
        else
        {
            // Second form using individual names
            context.textureUnit->setAnimatedTextureName(
                (String*)&vecparams[0],
                static_cast<unsigned int>(numParams-1),
                StringConverter::parseReal(vecparams[numParams-1]));
        }
        return false;

    }
    //-----------------------------------------------------------------------
    bool parseCubicTexture(String& params, MaterialScriptContext& context)
    {

        StringVector vecparams = StringUtil::tokenise(params, " \t");
        size_t numParams = vecparams.size();

        // Get final param
        bool useUVW;
        String& uvOpt = vecparams[numParams-1];
        StringUtil::toLowerCase(uvOpt);
        if (uvOpt == "combineduvw")
            useUVW = true;
        else if (uvOpt == "separateuv")
            useUVW = false;
        else
        {
            logParseError("Bad cubic_texture attribute, final parameter must be "
                "'combinedUVW' or 'separateUV'.", context);
            return false;
        }
        // Determine which form it is
        if (numParams == 2)
        {
            // First form using base name
            context.textureUnit->setCubicTextureName(vecparams[0], useUVW);
        }
        else if (numParams == 7)
        {
            // Second form using individual names
            // Can use vecparams[0] as array start point
            context.textureUnit->setCubicTextureName((String*)&vecparams[0], useUVW);
        }
        else
        {
            logParseError(
                "Bad cubic_texture attribute, wrong number of parameters (expected 2 or 7)",
                context);
            return false;
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexCoord(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureCoordSet(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
    TextureUnitState::TextureAddressingMode convTexAddressMode(const String& params, MaterialScriptContext& context)
    {
        if (params=="wrap")
            return TextureUnitState::TAM_WRAP;
        else if (params=="mirror")
            return TextureUnitState::TAM_MIRROR;
        else if (params=="clamp")
            return TextureUnitState::TAM_CLAMP;
        else if (params=="border")
            return TextureUnitState::TAM_BORDER;
        else
            logParseError("Bad tex_address_mode attribute, valid parameters are "
                "'wrap', 'mirror', 'clamp' or 'border'.", context);
        // default
        return TextureUnitState::TAM_WRAP;
    }
    //-----------------------------------------------------------------------
    bool parseTexAddressMode(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);

        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        if (numParams > 3 || numParams < 1)
        {
            logParseError("Invalid number of parameters to tex_address_mode"
                    " - must be between 1 and 3", context);
        }
        if (numParams == 1)
        {
            // Single-parameter option
            context.textureUnit->setTextureAddressingMode(
                convTexAddressMode(vecparams[0], context));
        }
        else
        {
            // 2-3 parameter option
            TextureUnitState::UVWAddressingMode uvw;
            uvw.u = convTexAddressMode(vecparams[0], context);
            uvw.v = convTexAddressMode(vecparams[1], context);
            if (numParams == 3)
            {
                // w
                uvw.w = convTexAddressMode(vecparams[2], context);
            }
            else
            {
                uvw.w = TextureUnitState::TAM_WRAP;
            }
            context.textureUnit->setTextureAddressingMode(uvw);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexBorderColour(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 3 or 4 parameters
        if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.textureUnit->setTextureBorderColour( _parseColourValue(vecparams) );
        }
        else
        {
            logParseError(
                "Bad tex_border_colour attribute, wrong number of parameters (expected 3 or 4)",
                context);
        }
        return false;
    }
    bool parseColourOp(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="replace")
            context.textureUnit->setColourOperation(LBO_REPLACE);
        else if (params=="add")
            context.textureUnit->setColourOperation(LBO_ADD);
        else if (params=="modulate")
            context.textureUnit->setColourOperation(LBO_MODULATE);
        else if (params=="alpha_blend")
            context.textureUnit->setColourOperation(LBO_ALPHA_BLEND);
        else
            logParseError("Bad colour_op attribute, valid parameters are "
                "'replace', 'add', 'modulate' or 'alpha_blend'.", context);

        return false;
    }
    LayerBlendOperationEx convertBlendOpEx(const String& param)
    {
        if (param == "source1")
            return LBX_SOURCE1;
        else if (param == "source2")
            return LBX_SOURCE2;
        else if (param == "modulate")
            return LBX_MODULATE;
        else if (param == "modulate_x2")
            return LBX_MODULATE_X2;
        else if (param == "modulate_x4")
            return LBX_MODULATE_X4;
        else if (param == "add")
            return LBX_ADD;
        else if (param == "add_signed")
            return LBX_ADD_SIGNED;
        else if (param == "add_smooth")
            return LBX_ADD_SMOOTH;
        else if (param == "subtract")
            return LBX_SUBTRACT;
        else if (param == "blend_diffuse_colour")
            return LBX_BLEND_DIFFUSE_COLOUR;
        else if (param == "blend_diffuse_alpha")
            return LBX_BLEND_DIFFUSE_ALPHA;
        else if (param == "blend_texture_alpha")
            return LBX_BLEND_TEXTURE_ALPHA;
        else if (param == "blend_current_alpha")
            return LBX_BLEND_CURRENT_ALPHA;
        else if (param == "blend_manual")
            return LBX_BLEND_MANUAL;
        else if (param == "dotproduct")
            return LBX_DOTPRODUCT;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend function", "convertBlendOpEx");
    }
    //-----------------------------------------------------------------------
    LayerBlendSource convertBlendSource(const String& param)
    {
        if (param == "src_current")
            return LBS_CURRENT;
        else if (param == "src_texture")
            return LBS_TEXTURE;
        else if (param == "src_diffuse")
            return LBS_DIFFUSE;
        else if (param == "src_specular")
            return LBS_SPECULAR;
        else if (param == "src_manual")
            return LBS_MANUAL;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend source", "convertBlendSource");
    }
    //-----------------------------------------------------------------------
    bool parseColourOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        if (numParams < 3 || numParams > 10)
        {
            logParseError(
                "Bad colour_op_ex attribute, wrong number of parameters (expected 3 to 10)",
                context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        ColourValue colSrc1 = ColourValue::White;
        ColourValue colSrc2 = ColourValue::White;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);

            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams < 4)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }

            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc1.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc1.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc1.a = 1.0f;
                }
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex += 3;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc2.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc2.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc2.a = 1.0f;
                }
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_ex attribute, " + e.getDescription(), context);
            return false;
        }

        context.textureUnit->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourOpFallback(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, wrong number "
                "of parameters (expected 2)", context);
            return false;
        }

        //src/dest
        SceneBlendFactor src, dest;

        try {
            src = convertBlendFactor(vecparams[0]);
            dest = convertBlendFactor(vecparams[1]);
            context.textureUnit->setColourOpMultipassFallback(src,dest);
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, "
                + e.getDescription(), context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAlphaOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();
        if (numParams < 3 || numParams > 6)
        {
            logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                "(expected 3 to 6)", context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        Real arg1 = 1.0, arg2 = 1.0;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);
            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams != 4)
                {
                    logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }
            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters (expected " +
                        StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg1 = StringConverter::parseReal(vecparams[parIndex]);
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg2 = StringConverter::parseReal(vecparams[parIndex]);
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad alpha_op_ex attribute, " + e.getDescription(), context);
            return false;
        }

        context.textureUnit->setAlphaOperation(op, src1, src2, arg1, arg2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEnvMap(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="off")
            context.textureUnit->setEnvironmentMap(false);
        else if (params=="spherical")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
        else if (params=="planar")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
        else if (params=="cubic_reflection")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
        else if (params=="cubic_normal")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
        else
            logParseError("Bad env_map attribute, valid parameters are 'off', "
                "'spherical', 'planar', 'cubic_reflection' and 'cubic_normal'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScroll(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScroll(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));


        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScrollAnim(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll_anim attribute, wrong number of "
                "parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setScrollAnimation(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotate(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureRotate(
            StringConverter::parseAngle(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotateAnim(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setRotateAnimation(
            StringConverter::parseReal(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScale(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scale attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScale(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseWaveXform(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");

        if (vecparams.size() != 6)
        {
            logParseError("Bad wave_xform attribute, wrong number of parameters "
                "(expected 6)", context);
            return false;
        }
        TextureUnitState::TextureTransformType ttype;
        WaveformType waveType;
        // Check transform type
        if (vecparams[0]=="scroll_x")
            ttype = TextureUnitState::TT_TRANSLATE_U;
        else if (vecparams[0]=="scroll_y")
            ttype = TextureUnitState::TT_TRANSLATE_V;
        else if (vecparams[0]=="rotate")
            ttype = TextureUnitState::TT_ROTATE;
        else if (vecparams[0]=="scale_x")
            ttype = TextureUnitState::TT_SCALE_U;
        else if (vecparams[0]=="scale_y")
            ttype = TextureUnitState::TT_SCALE_V;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 1 must be 'scroll_x', "
                "'scroll_y', 'rotate', 'scale_x' or 'scale_y'", context);
            return false;
        }
        // Check wave type
        if (vecparams[1]=="sine")
            waveType = WFT_SINE;
        else if (vecparams[1]=="triangle")
            waveType = WFT_TRIANGLE;
        else if (vecparams[1]=="square")
            waveType = WFT_SQUARE;
        else if (vecparams[1]=="sawtooth")
            waveType = WFT_SAWTOOTH;
        else if (vecparams[1]=="inverse_sawtooth")
            waveType = WFT_INVERSE_SAWTOOTH;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 2 must be 'sine', "
                "'triangle', 'square', 'sawtooth' or 'inverse_sawtooth'", context);
            return false;
        }

        context.textureUnit->setTransformAnimation(
            ttype,
            waveType,
            StringConverter::parseReal(vecparams[2]),
            StringConverter::parseReal(vecparams[3]),
            StringConverter::parseReal(vecparams[4]),
            StringConverter::parseReal(vecparams[5]) );

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTransform(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 16)
        {
            logParseError("Bad transform attribute, wrong number of parameters (expected 16)", context);
            return false;
        }
        Matrix4 xform(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]),
            StringConverter::parseReal(vecparams[2]),
            StringConverter::parseReal(vecparams[3]),
            StringConverter::parseReal(vecparams[4]),
            StringConverter::parseReal(vecparams[5]),
            StringConverter::parseReal(vecparams[6]),
            StringConverter::parseReal(vecparams[7]),
            StringConverter::parseReal(vecparams[8]),
            StringConverter::parseReal(vecparams[9]),
            StringConverter::parseReal(vecparams[10]),
            StringConverter::parseReal(vecparams[11]),
            StringConverter::parseReal(vecparams[12]),
            StringConverter::parseReal(vecparams[13]),
            StringConverter::parseReal(vecparams[14]),
            StringConverter::parseReal(vecparams[15]) );
        context.textureUnit->setTextureTransform(xform);


        return false;
    }
    bool parseAnisotropy(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureAnisotropy(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTextureAlias(String& params, MaterialScriptContext& context)
    {
        trimAndUnquoteWord(params);
        context.textureUnit->setTextureNameAlias(params);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMipmapBias(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureMipmapBias(
            (float)StringConverter::parseReal(params));

        return false;
    }
    bool parseContentType(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::tokenise(params, " \t");
        if (vecparams.empty())
        {
            logParseError("No content_type specified", context);
            return false;
        }
        String& paramType = vecparams[0];
        if (paramType == "named")
        {
            context.textureUnit->setContentType(TextureUnitState::CONTENT_NAMED);
        }
        else if (paramType == "shadow")
        {
            context.textureUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
        }
        else if (paramType == "compositor")
        {
            context.textureUnit->setContentType(TextureUnitState::CONTENT_COMPOSITOR);
            if (vecparams.size() == 3)
            {
                context.textureUnit->setCompositorReference(vecparams[1], vecparams[2]);
            }
            else if (vecparams.size() == 4)
            {
                context.textureUnit->setCompositorReference(vecparams[1], vecparams[2], 
                    StringConverter::parseUnsignedInt(vecparams[3]));
            }
            else
            {
                logParseError("compositor content_type requires 2 or 3 extra params", context);
            }
        }
        else
        {
            logParseError("Invalid content_type specified : " + paramType, context);
        }
        return false;
    }
}

namespace sh
{
    OgreMaterialSerializer::OgreMaterialSerializer()
    {
        // Set up material attribute parsers
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_values", (ATTRIBUTE_PARSER)parseLodValues));
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_strategy", (ATTRIBUTE_PARSER)parseLodStrategy));
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_distances", (ATTRIBUTE_PARSER)parseLodDistances));
        mMaterialAttribParsers.insert(AttribParserList::value_type("receive_shadows", (ATTRIBUTE_PARSER)parseReceiveShadows));
        mMaterialAttribParsers.insert(AttribParserList::value_type("transparency_casts_shadows", (ATTRIBUTE_PARSER)parseTransparencyCastsShadows));
        // mMaterialAttribParsers.insert(AttribParserList::value_type("technique", (ATTRIBUTE_PARSER)parseTechnique));
        mMaterialAttribParsers.insert(AttribParserList::value_type("set_texture_alias", (ATTRIBUTE_PARSER)parseSetTextureAlias));

        // Set up pass attribute parsers
        mPassAttribParsers.insert(AttribParserList::value_type("ambient", (ATTRIBUTE_PARSER)parseAmbient));
        mPassAttribParsers.insert(AttribParserList::value_type("diffuse", (ATTRIBUTE_PARSER)parseDiffuse));
        mPassAttribParsers.insert(AttribParserList::value_type("specular", (ATTRIBUTE_PARSER)parseSpecular));
        mPassAttribParsers.insert(AttribParserList::value_type("emissive", (ATTRIBUTE_PARSER)parseEmissive));
        mPassAttribParsers.insert(AttribParserList::value_type("scene_blend", (ATTRIBUTE_PARSER)parseSceneBlend));
        mPassAttribParsers.insert(AttribParserList::value_type("separate_scene_blend", (ATTRIBUTE_PARSER)parseSeparateSceneBlend));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_check", (ATTRIBUTE_PARSER)parseDepthCheck));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_write", (ATTRIBUTE_PARSER)parseDepthWrite));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_func", (ATTRIBUTE_PARSER)parseDepthFunc));
        mPassAttribParsers.insert(AttribParserList::value_type("normalise_normals", (ATTRIBUTE_PARSER)parseNormaliseNormals));
        mPassAttribParsers.insert(AttribParserList::value_type("alpha_rejection", (ATTRIBUTE_PARSER)parseAlphaRejection));
        mPassAttribParsers.insert(AttribParserList::value_type("alpha_to_coverage", (ATTRIBUTE_PARSER)parseAlphaToCoverage));
        mPassAttribParsers.insert(AttribParserList::value_type("transparent_sorting", (ATTRIBUTE_PARSER)parseTransparentSorting));
        mPassAttribParsers.insert(AttribParserList::value_type("colour_write", (ATTRIBUTE_PARSER)parseColourWrite));
        mPassAttribParsers.insert(AttribParserList::value_type("light_scissor", (ATTRIBUTE_PARSER)parseLightScissor));
        mPassAttribParsers.insert(AttribParserList::value_type("light_clip_planes", (ATTRIBUTE_PARSER)parseLightClip));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_hardware", (ATTRIBUTE_PARSER)parseCullHardware));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_software", (ATTRIBUTE_PARSER)parseCullSoftware));
        mPassAttribParsers.insert(AttribParserList::value_type("lighting", (ATTRIBUTE_PARSER)parseLighting));
        mPassAttribParsers.insert(AttribParserList::value_type("fog_override", (ATTRIBUTE_PARSER)parseFogging));
        mPassAttribParsers.insert(AttribParserList::value_type("shading", (ATTRIBUTE_PARSER)parseShading));
        mPassAttribParsers.insert(AttribParserList::value_type("polygon_mode", (ATTRIBUTE_PARSER)parsePolygonMode));
        mPassAttribParsers.insert(AttribParserList::value_type("polygon_mode_overrideable", (ATTRIBUTE_PARSER)parsePolygonModeOverrideable));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_bias", (ATTRIBUTE_PARSER)parseDepthBias));
        mPassAttribParsers.insert(AttribParserList::value_type("iteration_depth_bias", (ATTRIBUTE_PARSER)parseIterationDepthBias));
        /*mPassAttribParsers.insert(AttribParserList::value_type("texture_unit", (ATTRIBUTE_PARSER)parseTextureUnit));
        mPassAttribParsers.insert(AttribParserList::value_type("vertex_program_ref", (ATTRIBUTE_PARSER)parseVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("geometry_program_ref", (ATTRIBUTE_PARSER)parseGeometryProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_caster_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowCasterVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_caster_fragment_program_ref", (ATTRIBUTE_PARSER)parseShadowCasterFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_receiver_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowReceiverVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_receiver_fragment_program_ref", (ATTRIBUTE_PARSER)parseShadowReceiverFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("fragment_program_ref", (ATTRIBUTE_PARSER)parseFragmentProgramRef));*/
        mPassAttribParsers.insert(AttribParserList::value_type("max_lights", (ATTRIBUTE_PARSER)parseMaxLights));
        mPassAttribParsers.insert(AttribParserList::value_type("start_light", (ATTRIBUTE_PARSER)parseStartLight));
        mPassAttribParsers.insert(AttribParserList::value_type("iteration", (ATTRIBUTE_PARSER)parseIteration));
        mPassAttribParsers.insert(AttribParserList::value_type("point_size", (ATTRIBUTE_PARSER)parsePointSize));
        mPassAttribParsers.insert(AttribParserList::value_type("point_sprites", (ATTRIBUTE_PARSER)parsePointSprites));
        mPassAttribParsers.insert(AttribParserList::value_type("point_size_attenuation", (ATTRIBUTE_PARSER)parsePointAttenuation));
        mPassAttribParsers.insert(AttribParserList::value_type("point_size_min", (ATTRIBUTE_PARSER)parsePointSizeMin));
        mPassAttribParsers.insert(AttribParserList::value_type("point_size_max", (ATTRIBUTE_PARSER)parsePointSizeMax));
        mPassAttribParsers.insert(AttribParserList::value_type("illumination_stage", (ATTRIBUTE_PARSER)parseIlluminationStage));

        // Set up texture unit attribute parsers
        // mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture_source", (ATTRIBUTE_PARSER)parseTextureSource));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture", (ATTRIBUTE_PARSER)parseTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("anim_texture", (ATTRIBUTE_PARSER)parseAnimTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("cubic_texture", (ATTRIBUTE_PARSER)parseCubicTexture));
        // mTextureUnitAttribParsers.insert(AttribParserList::value_type("binding_type", (ATTRIBUTE_PARSER)parseBindingType));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_coord_set", (ATTRIBUTE_PARSER)parseTexCoord));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_address_mode", (ATTRIBUTE_PARSER)parseTexAddressMode));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_border_colour", (ATTRIBUTE_PARSER)parseTexBorderColour));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op", (ATTRIBUTE_PARSER)parseColourOp));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_ex", (ATTRIBUTE_PARSER)parseColourOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_multipass_fallback", (ATTRIBUTE_PARSER)parseColourOpFallback));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("alpha_op_ex", (ATTRIBUTE_PARSER)parseAlphaOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("env_map", (ATTRIBUTE_PARSER)parseEnvMap));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll", (ATTRIBUTE_PARSER)parseScroll));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll_anim", (ATTRIBUTE_PARSER)parseScrollAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate", (ATTRIBUTE_PARSER)parseRotate));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate_anim", (ATTRIBUTE_PARSER)parseRotateAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scale", (ATTRIBUTE_PARSER)parseScale));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("wave_xform", (ATTRIBUTE_PARSER)parseWaveXform));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("transform", (ATTRIBUTE_PARSER)parseTransform));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("filtering", (ATTRIBUTE_PARSER)parseFiltering));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("compare_test", (ATTRIBUTE_PARSER)parseCompareTest));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("compare_func", (ATTRIBUTE_PARSER)parseCompareFunction));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("max_anisotropy", (ATTRIBUTE_PARSER)parseAnisotropy));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture_alias", (ATTRIBUTE_PARSER)parseTextureAlias));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("mipmap_bias", (ATTRIBUTE_PARSER)parseMipmapBias));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("content_type", (ATTRIBUTE_PARSER)parseContentType));
    }

	bool OgreMaterialSerializer::setPassProperty (const std::string& param, std::string value, Ogre::Pass* pass)
	{
		MaterialScriptContext mScriptContext;
		mScriptContext.pass = pass;

		if (mPassAttribParsers.find (param) == mPassAttribParsers.end())
			return false;
		else
		{
			mPassAttribParsers.find(param)->second(value, mScriptContext);
			return true;
		}
	}

	bool OgreMaterialSerializer::setTextureUnitProperty (const std::string& param, std::string value, Ogre::TextureUnitState* t)
	{
		// quick access to automip setting, without having to use 'texture' which doesn't like spaces in filenames
		if (param == "num_mipmaps")
		{
			t->setNumMipmaps(Ogre::StringConverter::parseInt(value));
			return true;
		}
		
		MaterialScriptContext mScriptContext;
		mScriptContext.textureUnit = t;

		if (mTextureUnitAttribParsers.find (param) == mTextureUnitAttribParsers.end())
			return false;
		else
		{
			mTextureUnitAttribParsers.find(param)->second(value, mScriptContext);
			return true;
		}
	}

	bool OgreMaterialSerializer::setMaterialProperty (const std::string& param, std::string value, Ogre::MaterialPtr m)
	{
        MaterialScriptContext mScriptContext;
		mScriptContext.material = m;

		if (mMaterialAttribParsers.find (param) == mMaterialAttribParsers.end())
			return false;
		else
		{
			mMaterialAttribParsers.find(param)->second(value, mScriptContext);
			return true;
		}
	}
}
