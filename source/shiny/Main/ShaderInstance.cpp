#include "ShaderInstance.hpp"

#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Preprocessor.hpp"
#include "Factory.hpp"
#include "ShaderSet.hpp"

namespace
{
	std::string convertLang (sh::Language lang)
	{
		if (lang == sh::Language_CG)
			return "SH_CG";
		else if (lang == sh::Language_HLSL)
			return "SH_HLSL";
		else if (lang == sh::Language_GLSL)
			return "SH_GLSL";
	}
}

namespace sh
{
	ShaderInstance::ShaderInstance (ShaderSet* parent, const std::string& name, PropertySetGet* properties)
		: mName(name)
		, mParent(parent)
		, mSupported(true)
	{
		std::string source = mParent->getSource();
		int type = mParent->getType();
		std::string basePath = mParent->getBasePath();

		std::vector<std::string> definitions;

		if (mParent->getType() == GPT_Vertex)
			definitions.push_back("SH_VERTEX_SHADER");
		else
			definitions.push_back("SH_FRAGMENT_SHADER");
		definitions.push_back(convertLang(Factory::getInstance().getCurrentLanguage()));

		// replace properties
		size_t pos;
		while (true)
		{
			pos =  source.find("@shProperty");
			if (pos == std::string::npos)
				break;
			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			std::string cmd = source.substr(pos+1, start-(pos+1));
			std::string replaceValue;
			if (cmd == "shPropertyBool")
			{
				std::string propertyName = source.substr(start+1, end-(start+1));
				PropertyValuePtr value = properties->getProperty(propertyName);
				bool val = retrieveValue<BooleanValue>(value, properties->getContext()).get();
				replaceValue = val ? "1" : "0";
			}
			else if (cmd == "shPropertyString")
			{
				std::string propertyName = source.substr(start+1, end-(start+1));
				PropertyValuePtr value = properties->getProperty(propertyName);
				replaceValue = retrieveValue<StringValue>(value, properties->getContext()).get();
			}
			else if (cmd == "shPropertyEqual")
			{
				size_t comma_start = source.find(",", pos);
				size_t comma_end = comma_start+1;
				// skip spaces
				while (source[comma_end] == ' ')
					++comma_end;
				std::string propertyName = source.substr(start+1, comma_start-(start+1));
				std::string comparedAgainst = source.substr(comma_end, end-comma_end);
				std::string value = retrieveValue<StringValue>(properties->getProperty(propertyName), properties->getContext()).get();
				replaceValue = (value == comparedAgainst) ? "1" : "0";
			}
			else
				throw std::runtime_error ("unknown command \"" + cmd + "\" in \"" + name + "\"");
			source.replace(pos, (end+1)-pos, replaceValue);
		}

		// replace global settings
		while (true)
		{
			pos =  source.find("@shGlobalSetting");
			if (pos == std::string::npos)
				break;

			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			std::string cmd = source.substr(pos+1, start-(pos+1));
			std::string replaceValue;
			if (cmd == "shGlobalSettingBool")
			{
				std::string settingName = source.substr(start+1, end-(start+1));
				std::string value = mParent->getCurrentGlobalSettings()->find(settingName)->second;
				replaceValue = (value == "true" || value == "1") ? "1" : "0";
			}
			if (cmd == "shGlobalSettingEqual")
			{
				size_t comma_start = source.find(",", pos);
				size_t comma_end = comma_start+1;
				// skip spaces
				while (source[comma_end] == ' ')
					++comma_end;
				std::string settingName = source.substr(start+1, comma_start-(start+1));
				std::string comparedAgainst = source.substr(comma_end, end-comma_end);
				std::string value = mParent->getCurrentGlobalSettings()->find(settingName)->second;
				replaceValue = (value == comparedAgainst) ? "1" : "0";
			}

			else
				throw std::runtime_error ("unknown command \"" + cmd + "\" in \"" + name + "\"");
			source.replace(pos, (end+1)-pos, replaceValue);
		}

		// parse foreach
		while (true)
		{
			pos = source.find("@shForeach");
			if (pos == std::string::npos)
				break;

			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			int num = boost::lexical_cast<int>(source.substr(start+1, end-(start+1)));

			assert(source.find("@shEndForeach", pos) != std::string::npos);
			size_t block_end = source.find("@shEndForeach", pos);

			// get the content of the inner block
			std::string content = source.substr(end+1, block_end - (end+1));

			// replace both outer and inner block with content of inner block num times
			std::string replaceStr;
			for (int i=0; i<num; ++i)
			{
				// replace @shIterator with the current iteration
				std::string addStr = content;
				boost::replace_all(addStr, "@shIteration", boost::lexical_cast<std::string>(i));
				replaceStr += addStr;
			}
			source.replace(pos, (block_end+std::string("@shEndForeach").length())-pos, replaceStr);
		}

		// why do we need our own preprocessor? there are several custom commands available in the shader files
		// (for example for binding uniforms to properties or auto constants) - more below. it is important that these
		// commands are _only executed if the specific code path actually "survives" the compilation.
		// thus, we run the code through a preprocessor first to remove the parts that are unused because of
		// unmet #if conditions (or other preprocessor directives).
		Preprocessor p;
		source = p.preprocess(source, basePath, definitions, name);

		// parse auto constants
		typedef std::map< std::string, std::pair<std::string, std::string> > AutoConstantMap;
		AutoConstantMap autoConstants;
		while (true)
		{
			pos = source.find("@shAutoConstant");
			if (pos == std::string::npos)
				break;

			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			size_t comma1 = source.find(",", pos);
			size_t comma2 = source.find(",", comma1+1);

			bool hasExtraData = (comma2 != std::string::npos) && (comma2 < end);
			std::string autoConstantName, uniformName;
			std::string extraData;
			if (!hasExtraData)
			{
				uniformName = source.substr(start+1, comma1-(start+1));
				// skip spaces
				++comma1;
				while (source[comma1] == ' ')
					++comma1;
				autoConstantName = source.substr(comma1, end-(comma1));
			}
			else
			{
				uniformName = source.substr(start+1, comma1-(start+1));
				// skip spaces
				++comma1;
				while (source[comma1] == ' ')
					++comma1;
				autoConstantName = source.substr(comma1, comma2-(comma1));
				// skip spaces
				++comma2;
				while (source[comma2] == ' ')
					++comma2;
				extraData = source.substr(comma2, end-comma2);
			}
			autoConstants[uniformName] = std::make_pair(autoConstantName, extraData);

			source.erase(pos, (end+1)-pos);
		}

		// parse uniform properties
		while (true)
		{
			pos = source.find("@shUniformProperty");
			if (pos == std::string::npos)
				break;

			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			std::string cmd = source.substr(pos, start-pos);
			ValueType vt;
			if (cmd == "@shUniformProperty4f")
				vt = VT_Vector4;
			else if (cmd == "@shUniformProperty3f")
				vt = VT_Vector3;
			else if (cmd == "@shUniformProperty2f")
				vt = VT_Vector2;
			else if (cmd == "@shUniformProperty1f")
				vt = VT_Float;
			else if (cmd == "@shUniformPropertyInt")
				vt = VT_Int;
			else
				throw std::runtime_error ("unsupported command \"" + cmd + "\"");

			size_t comma1 = source.find(",", pos);

			std::string propertyName, uniformName;
			uniformName = source.substr(start+1, comma1-(start+1));
			// skip spaces
			++comma1;
			while (source[comma1] == ' ')
				++comma1;
			propertyName = source.substr(comma1, end-(comma1));
			mUniformProperties[uniformName] = std::make_pair(propertyName, vt);

			source.erase(pos, (end+1)-pos);
		}

		while (true)
		{
			// can't use #version XYZ in the shader file itself because the preprocessor gets confused.
			// therefore use a @shGlslVersion macro, that gets replaced _after_ the preprocessing.
			pos = source.find("@shGlslVersion");
			if (pos == std::string::npos)
				break;

			size_t start = source.find("(", pos);
			size_t end = source.find(")", pos);
			std::string version = source.substr(start+1, end-(start+1));
			source.replace(pos, (end+1)-pos, "#version " + version);
		}

		Platform* platform = Factory::getInstance().getPlatform();

		std::string profile;
		if (Factory::getInstance ().getCurrentLanguage () == Language_CG)
			profile = mParent->getCgProfile ();
		else if (Factory::getInstance ().getCurrentLanguage () == Language_HLSL)
			profile = mParent->getHlslProfile ();

		if (type == GPT_Vertex)
			mProgram = boost::shared_ptr<GpuProgram>(platform->createGpuProgram(GPT_Vertex, "", mName, profile, source, Factory::getInstance().getCurrentLanguage()));
		else if (type == GPT_Fragment)
			mProgram = boost::shared_ptr<GpuProgram>(platform->createGpuProgram(GPT_Fragment, "", mName, profile, source, Factory::getInstance().getCurrentLanguage()));

		if (!mProgram->getSupported())
		{
			std::cout << "        Full source code below: \n" << source << std::endl;
			mSupported = false;
			return;
		}

		for (AutoConstantMap::iterator it = autoConstants.begin(); it != autoConstants.end(); ++it)
		{
			mProgram->setAutoConstant(it->first, it->second.first, it->second.second);
		}
	}

	std::string ShaderInstance::getName ()
	{
		return mName;
	}

	bool ShaderInstance::getSupported () const
	{
		return mSupported;
	}

	void ShaderInstance::setUniformParameters (boost::shared_ptr<Pass> pass, PropertySetGet* properties)
	{
		for (UniformMap::iterator it = mUniformProperties.begin(); it != mUniformProperties.end(); ++it)
		{
			pass->setGpuConstant(mParent->getType(), it->first, it->second.second, properties->getProperty(it->second.first), properties->getContext());
		}
	}

}
