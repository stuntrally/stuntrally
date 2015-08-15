#include "pch.h"
#include "ScriptLoader.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <exception>
#include <fstream>

#include <boost/filesystem.hpp>

namespace sh
{
	void ScriptLoader::loadAllFiles(ScriptLoader* c, const std::string& path)
	{
		for ( boost::filesystem::recursive_directory_iterator end, dir(path); dir != end; ++dir )
		{
			boost::filesystem::path p(*dir);
			if(p.extension() == c->mFileEnding)
			{
				c->mCurrentFileName = (*dir).path().string();
				std::ifstream in((*dir).path().string().c_str(), std::ios::binary);
				c->parseScript(in);
			}
		}
	}

	ScriptLoader::ScriptLoader(const std::string& fileEnding)
		: mLoadOrder(0)
		, mToken(TOKEN_NewLine)
		, mLastToken(TOKEN_NewLine)

	{
		mFileEnding = fileEnding;
	}

	ScriptLoader::~ScriptLoader()
	{
		clearScriptList();
	}

	void ScriptLoader::clearScriptList()
	{
		std::map <std::string, ScriptNode *>::iterator i;
		for (i = m_scriptList.begin(); i != m_scriptList.end(); ++i)
		{
			delete i->second;
		}
		m_scriptList.clear();
	}

	ScriptNode *ScriptLoader::getConfigScript(const std::string &name)
	{
		std::map <std::string, ScriptNode*>::iterator i;

		std::string key = name;
		i = m_scriptList.find(key);

		//If found..
		if (i != m_scriptList.end())
		{
			return i->second;
		}
		else
		{
			return NULL;
		}
	}

	std::map <std::string, ScriptNode*> ScriptLoader::getAllConfigScripts ()
	{
		return m_scriptList;
	}

	void ScriptLoader::parseScript(std::ifstream &stream)
	{
		//Get first token
		_nextToken(stream);
		if (mToken == TOKEN_EOF)
		{
			stream.close();
			return;
		}

		//Parse the script
		_parseNodes(stream, 0);

		stream.close();
	}

	void ScriptLoader::_nextToken(std::ifstream &stream)
	{
		//EOF token
		if (!stream.good())
		{
			mToken = TOKEN_EOF;
			return;
		}

		//(Get next character)
		int ch = stream.get();

		while ((ch == ' ' || ch == 9) && !stream.eof())
		{    //Skip leading spaces / tabs
			ch = stream.get();
		}

		if (!stream.good())
		{
			mToken = TOKEN_EOF;
			return;
		}

		//Newline token
		if (ch == '\r' || ch == '\n')
		{
			do
			{
				ch = stream.get();
			} while ((ch == '\r' || ch == '\n') && !stream.eof());

			stream.unget();

			mToken = TOKEN_NewLine;
			return;
		}

		//Open brace token
		else if (ch == '{')
		{
			mToken = TOKEN_OpenBrace;
			return;
		}

		//Close brace token
		else if (ch == '}')
		{
			mToken = TOKEN_CloseBrace;
			return;
		}

		//Text token
		if (ch < 32 || ch > 122)    //Verify valid char
		{
			throw std::runtime_error("Parse Error: Invalid character, ConfigLoader::load()");
		}

		mTokenValue = "";
		mToken = TOKEN_Text;
		do
		{
			//Skip comments
			if (ch == '/')
			{
				int ch2 = stream.peek();

				//C++ style comment (//)
				if (ch2 == '/')
				{
					stream.get();
					do
					{
						ch = stream.get();
					} while (ch != '\r' && ch != '\n' && !stream.eof());

					mToken = TOKEN_NewLine;
					return;
				}
			}

			//Add valid char to tokVal
			mTokenValue += (char)ch;

			//Next char
			ch = stream.get();

		} while (ch > 32 && ch <= 122 && !stream.eof());

		stream.unget();

		return;
	}

	void ScriptLoader::_skipNewLines(std::ifstream &stream)
	{
		while (mToken == TOKEN_NewLine)
		{
			_nextToken(stream);
		}
	}

	void ScriptLoader::_parseNodes(std::ifstream &stream, ScriptNode *parent)
	{
		typedef std::pair<std::string, ScriptNode*> ScriptItem;

		while (true)
		{
			switch (mToken)
			{
				//Node
				case TOKEN_Text:
				{
					//Add the new node
					ScriptNode *newNode;
					if (parent)
					{
						newNode = parent->addChild(mTokenValue);
					}
					else
					{
						newNode = new ScriptNode(0, mTokenValue);
					}

					//Get values
					_nextToken(stream);
					std::string valueStr;
					int i=0;
					while (mToken == TOKEN_Text)
					{
						if (i == 0)
							valueStr += mTokenValue;
						else
							valueStr += " " + mTokenValue;
						_nextToken(stream);
						++i;
					}
					newNode->setValue(valueStr);

					_skipNewLines(stream);

					//Add any sub-nodes
					if (mToken == TOKEN_OpenBrace)
					{
						//Parse nodes
						_nextToken(stream);
						_parseNodes(stream, newNode);
						//Check for matching closing brace
						if (mToken != TOKEN_CloseBrace)
						{
							throw std::runtime_error("Parse Error: Expecting closing brace");
						}
						_nextToken(stream);
						_skipNewLines(stream);
					}

					newNode->mFileName = mCurrentFileName;

					//Add root nodes to scriptList
					if (!parent)
					{
						std::string key;

						if (newNode->getValue() == "")
								throw std::runtime_error("Root node must have a name (\"" + newNode->getName() + "\")");
						key = newNode->getValue();

						if (!m_scriptList.insert(ScriptItem(key, newNode)).second)
						{
							std::cout << "Script node '" << key << "' already exists" << std::endl;
							delete newNode;
							newNode = NULL;
						}
					}

					break;
				}

				//Out of place brace
				case TOKEN_OpenBrace:
					throw std::runtime_error("Parse Error: Opening brace out of plane");
					break;

				//Return if end of nodes have been reached
				case TOKEN_CloseBrace:
					return;

				//Return if reached end of file
				case TOKEN_EOF:
					return;

				case TOKEN_NewLine:
					_nextToken(stream);
					break;
			}
		};
	}

	ScriptNode::ScriptNode(ScriptNode *parent, const std::string &name)
	{
		mName = name;
		mParent = parent;
	}

	ScriptNode::~ScriptNode()
	{
		//Delete all children
		std::vector<ScriptNode*>::iterator i;
		for (i = mChildren.begin(); i != mChildren.end(); ++i)
		{
			ScriptNode *node = *i;
			delete node;
		}
		mChildren.clear();
	}

	ScriptNode *ScriptNode::addChild(const std::string &name)
	{
		ScriptNode* node = new ScriptNode(this, name);
		mChildren.push_back(node);
		return node;
	}

	ScriptNode *ScriptNode::findChild(const std::string &name)
	{
		int childCount = (int)mChildren.size();

		//Search for the node from start to finish
		for (int indx = 0; indx < childCount; ++indx){
			ScriptNode *node = mChildren[indx];
			if (node->mName == name)
				return node;
		}

		//Not found anywhere
		return NULL;
	}
}
