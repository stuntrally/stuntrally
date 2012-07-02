#include "ShaderSet.hpp"

#include <fstream>
#include <sstream>
#include <iostream> //temp

#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

namespace sh
{
	ShaderSet::ShaderSet (const std::string& type, const std::string& profile, const std::string& sourceFile, const std::string& basePath,
						  const std::string& name, std::map <std::string, std::string>* globalSettingsPtr)
		: mBasePath(basePath)
		, mName(name)
		, mCurrentGlobalSettings(globalSettingsPtr)
		, mProfile(profile)
	{
		if (type == "vertex")
			mType = Type_Vertex;
		else // if (type == "fragment")
			mType = Type_Fragment;

		std::ifstream stream(sourceFile.c_str(), std::ifstream::in);
		std::stringstream buffer;

		if (mType == Type_Vertex)
			buffer << "#define SH_VERTEX_SHADER" << std::endl;
		else if (mType == Type_Fragment)
			buffer << "#define SH_FRAGMENT_SHADER" << std::endl;
		buffer << stream.rdbuf();
		stream.close();
		mSource = buffer.str();
		parse();
	}

	void ShaderSet::parse()
	{
		std::string currentToken;
		bool tokenIsRecognized = false;
		bool isInBraces = false;
		for (std::string::const_iterator it = mSource.begin(); it != mSource.end(); ++it)
		{
			char c = *it;
			if (((c == ' ') && !isInBraces) || (c == '\n') ||
					(   ((c == '(') || (c == ')'))
						  && !tokenIsRecognized))
			{
				if (tokenIsRecognized)
				{
					if (boost::starts_with(currentToken, "@shGlobalSetting"))
					{
						assert ((currentToken.find('(') != std::string::npos) && (currentToken.find(')') != std::string::npos));
						size_t start = currentToken.find('(')+1;
						mGlobalSettings.push_back(currentToken.substr(start, currentToken.find(')')-start));
					}
					else if (boost::starts_with(currentToken, "@shPropertyEqual"))
					{
						assert ((currentToken.find('(') != std::string::npos) && (currentToken.find(')') != std::string::npos)
								&& (currentToken.find(',') != std::string::npos));
						size_t start = currentToken.find('(')+1;
						size_t end = currentToken.find(',');
						mProperties.push_back(currentToken.substr(start, end-start));
					}
					else if (boost::starts_with(currentToken, "@shProperty"))
					{
						assert ((currentToken.find('(') != std::string::npos) && (currentToken.find(')') != std::string::npos));
						size_t start = currentToken.find('(')+1;
						mProperties.push_back(currentToken.substr(start, currentToken.find(')')-start));
					}
				}

				currentToken = "";
			}
			else
			{
				if (currentToken == "")
				{
					if (c == '@')
						tokenIsRecognized = true;
					else
						tokenIsRecognized = false;
				}

				if (c == '(' && tokenIsRecognized)
					isInBraces = true;
				else if (c == ')' && tokenIsRecognized)
					isInBraces = false;

				currentToken += c;

			}
		}
	}

	ShaderInstance* ShaderSet::getInstance (PropertySetGet* properties)
	{
		size_t h = buildHash (properties);
		if (std::find(mFailedToCompile.begin(), mFailedToCompile.end(), h) != mFailedToCompile.end())
			return NULL;
		if (mInstances.find(h) == mInstances.end())
		{
			ShaderInstance newInstance(this, mName + "_" + boost::lexical_cast<std::string>(h), properties);
			if (!newInstance.getSupported())
			{
				mFailedToCompile.push_back(h);
				return NULL;
			}
			mInstances.insert(std::make_pair(h, newInstance));
		}
		return &mInstances.find(h)->second;
	}

	size_t ShaderSet::buildHash (PropertySetGet* properties)
	{
		size_t seed = 0;
		for (std::vector<std::string>::iterator it = mProperties.begin(); it != mProperties.end(); ++it)
		{
			std::string v = retrieveValue<StringValue>(properties->getProperty(*it), properties->getContext()).get();
			boost::hash_combine(seed, v);
		}
	}

	std::map <std::string, std::string>* ShaderSet::getCurrentGlobalSettings() const
	{
		return mCurrentGlobalSettings;
	}

	std::string ShaderSet::getBasePath() const
	{
		return mBasePath;
	}

	std::string ShaderSet::getSource() const
	{
		return mSource;
	}

	std::string ShaderSet::getProfile() const
	{
		return mProfile;
	}

	int ShaderSet::getType() const
	{
		return mType;
	}
}
