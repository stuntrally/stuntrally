/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "MyGUI_Precompiled.h"
#include "MyGUI_ResourceManualFont.h"
#include "MyGUI_SkinManager.h"
#include "MyGUI_RenderManager.h"
#include "MyGUI_TextureUtility.h"

namespace MyGUI
{

	ResourceManualFont::ResourceManualFont() :
		mDefaultHeight(0),
		mSubstituteGlyphInfo(nullptr),
		mTexture(nullptr)
	{
	}

	const GlyphInfo* ResourceManualFont::getGlyphInfo(Char _id) const
	{
		CharMap::const_iterator iter = mCharMap.find(_id);

		if (iter != mCharMap.end())
			return &iter->second;

		return mSubstituteGlyphInfo;
	}

	void ResourceManualFont::loadTexture()
	{
		if (mTexture == nullptr)
		{
			RenderManager& render = RenderManager::getInstance();
			mTexture = render.getTexture(mSource);
			if (mTexture == nullptr)
			{
				mTexture = render.createTexture(mSource);
				if (mTexture != nullptr)
					mTexture->loadFromFile(mSource);
			}
		}
	}

	void ResourceManualFont::deserialization(xml::ElementPtr _node, Version _version)
	{
		Base::deserialization(_node, _version);

		xml::ElementEnumerator node = _node->getElementEnumerator();
		while (node.next())
		{
			if (node->getName() == "Property")
			{
				const std::string& key = node->findAttribute("key");
				const std::string& value = node->findAttribute("value");
				if (key == "Source") mSource = value;
				else if (key == "DefaultHeight") mDefaultHeight = utility::parseInt(value);
				else if (key == "Shader") mShader = value;
			}
		}

		loadTexture();

		if (mTexture != nullptr)
		{
			if (!mShader.empty())
				mTexture->setShader(mShader);
			int textureWidth = mTexture->getWidth();
			int textureHeight = mTexture->getHeight();

			node = _node->getElementEnumerator();
			while (node.next())
			{
				if (node->getName() == "Codes")
				{
					xml::ElementEnumerator element = node->getElementEnumerator();
					while (element.next("Code"))
					{
						std::string value;
						// описане глифов
						if (element->findAttribute("index", value))
						{
							Char id = 0;
							if (value == "cursor")
								id = static_cast<Char>(FontCodeType::Cursor);
							else if (value == "selected")
								id = static_cast<Char>(FontCodeType::Selected);
							else if (value == "selected_back")
								id = static_cast<Char>(FontCodeType::SelectedBack);
							else if (value == "substitute")
								id = static_cast<Char>(FontCodeType::NotDefined);
							else
								id = utility::parseUInt(value);

							float advance(utility::parseValue<float>(element->findAttribute("advance")));
							FloatPoint bearing(utility::parseValue<FloatPoint>(element->findAttribute("bearing")));

							// texture coordinates
							FloatCoord coord(utility::parseValue<FloatCoord>(element->findAttribute("coord")));

							// glyph size, default to texture coordinate size
							std::string sizeString;
							FloatSize size(coord.width, coord.height);
							if (element->findAttribute("size", sizeString))
							{
								size = utility::parseValue<FloatSize>(sizeString);
							}

							if (advance == 0.0f)
								advance = size.width;

							GlyphInfo& glyphInfo = mCharMap.insert(CharMap::value_type(id, GlyphInfo(
								id,
								size.width,
								size.height,
								advance,
								bearing.left,
								bearing.top,
								FloatRect(
									coord.left / textureWidth,
									coord.top / textureHeight,
									coord.right() / textureWidth,
									coord.bottom() / textureHeight)
								))).first->second;

							if (id == FontCodeType::NotDefined)
								mSubstituteGlyphInfo = &glyphInfo;
						}
					}
				}
			}
		}
	}

	ITexture* ResourceManualFont::getTextureFont() const
	{
		return mTexture;
	}

	int ResourceManualFont::getDefaultHeight() const
	{
		return mDefaultHeight;
	}

	void ResourceManualFont::setSource(const std::string& value)
	{
		mTexture = nullptr;
		mSource = value;
		loadTexture();
	}

	void ResourceManualFont::setShader(const std::string& value)
	{
		mShader = value;
		if (mTexture != nullptr)
			mTexture->setShader(mShader);
	}

	void ResourceManualFont::setTexture(ITexture *texture)
	{
		mTexture = texture;
		mSource.clear();
	}

	void ResourceManualFont::setDefaultHeight(int value)
	{
		mDefaultHeight = value;
	}

	void ResourceManualFont::addGlyphInfo(Char id, const GlyphInfo& info)
	{
		GlyphInfo& inserted = mCharMap.insert(CharMap::value_type(id, info)).first->second;

		if (id == FontCodeType::NotDefined)
			mSubstituteGlyphInfo = &inserted;
	}

} // namespace MyGUI
