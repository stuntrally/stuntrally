#include "pch.h"
#include "MessageBoxStyle.h"
#include "MyGUI_Macros.h"

namespace MyGUI
{

	size_t MessageBoxStyle::getIconIndex()
	{
		size_t index = 0;
		int num = value >> _IndexIcon1;

		while (num != 0)
		{
			if ((num & 1) == 1)
				return index;

			++index;
			num >>= 1;
		}

		return ITEM_NONE;
	}


	size_t MessageBoxStyle::getButtonIndex()
	{
		size_t index = 0;
		int num = value;

		while (num != 0)
		{
			if ((num & 1) == 1)
				return index;

			++index;
			num >>= 1;
		}

		return ITEM_NONE;
	}


	std::vector<MessageBoxStyle> MessageBoxStyle::getButtons()
	{
		std::vector<MessageBoxStyle> buttons;

		size_t index = 0;
		int num = value;
		while (index < _IndexIcon1)
		{
			if ((num & 1) == 1)
			{
				buttons.push_back(MessageBoxStyle::Enum( MYGUI_FLAG(index)));
			}

			++index;
			num >>= 1;
		}

		return buttons;
	}

}
