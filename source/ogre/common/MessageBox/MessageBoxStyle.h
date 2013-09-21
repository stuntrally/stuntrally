/*!
	@file
	@author		Albert Semenov
	@date		10/2010
*/
#ifndef __MESSAGE_BOX_STYLE_H__
#define __MESSAGE_BOX_STYLE_H__
#include <vector>
#include <cstddef>
//#include <MyGUI_Prerequest.h>


namespace MyGUI
{

	struct MessageBoxStyle
	{
			#define MYGUI_FLAG_NONE  0
			#define MYGUI_FLAG(num)  (1<<(num))
		enum Enum
		{
			None = MYGUI_FLAG_NONE,
			Ok = MYGUI_FLAG(0),
			Yes = MYGUI_FLAG(1),
			No = MYGUI_FLAG(2),
			Abort = MYGUI_FLAG(3),
			Retry = MYGUI_FLAG(4),
			Ignore = MYGUI_FLAG(5),
			Cancel = MYGUI_FLAG(6),
			Try = MYGUI_FLAG(7),
			Continue = MYGUI_FLAG(8),

			_IndexUserButton1 = 9, // индекс первой кнопки юзера

			Button1 = MYGUI_FLAG(_IndexUserButton1),
			Button2 = MYGUI_FLAG(_IndexUserButton1 + 1),
			Button3 = MYGUI_FLAG(_IndexUserButton1 + 2),
			Button4 = MYGUI_FLAG(_IndexUserButton1 + 3),

			_CountUserButtons = 4, // колличество кнопок юзера
			_IndexIcon1 = _IndexUserButton1 + _CountUserButtons, // индекс первой иконки

			IconDefault = MYGUI_FLAG(_IndexIcon1),

			IconInfo = MYGUI_FLAG(_IndexIcon1),
			IconQuest = MYGUI_FLAG(_IndexIcon1 + 1),
			IconError = MYGUI_FLAG(_IndexIcon1 + 2),
			IconWarning = MYGUI_FLAG(_IndexIcon1 + 3),

			Icon1 = MYGUI_FLAG(_IndexIcon1),
			Icon2 = MYGUI_FLAG(_IndexIcon1 + 1),
			Icon3 = MYGUI_FLAG(_IndexIcon1 + 2),
			Icon4 = MYGUI_FLAG(_IndexIcon1 + 3),
			Icon5 = MYGUI_FLAG(_IndexIcon1 + 4),
			Icon6 = MYGUI_FLAG(_IndexIcon1 + 5),
			Icon7 = MYGUI_FLAG(_IndexIcon1 + 6),
			Icon8 = MYGUI_FLAG(_IndexIcon1 + 7)
		};

		MessageBoxStyle(Enum _value = None) :
			value(_value)
		{	}

		MessageBoxStyle& operator |= (MessageBoxStyle const& _other)
		{
			value = Enum(int(value) | int(_other.value));
			return *this;
		}

		friend MessageBoxStyle operator | (Enum const& a, Enum const& b)
		{
			return MessageBoxStyle(Enum(int(a) | int(b)));
		}

		MessageBoxStyle operator | (Enum const& a)
		{
			return MessageBoxStyle(Enum(int(value) | int(a)));
		}

		friend bool operator == (MessageBoxStyle const& a, MessageBoxStyle const& b)
		{
			return a.value == b.value;
		}

		friend bool operator != (MessageBoxStyle const& a, MessageBoxStyle const& b)
		{
			return a.value != b.value;
		}

		/*friend std::ostream& operator << (std::ostream& _stream, const MessageBoxStyle&  _value)
		{
			//_stream << _value.print();
			return _stream;
		}

		friend std::istream& operator >> (std::istream& _stream, MessageBoxStyle&  _value)
		{
			std::string value;
			_stream >> value;
			_value = parse(value);
			return _stream;
		}*/

		size_t getIconIndex();

		size_t getButtonIndex();

		std::vector<MessageBoxStyle> getButtons();

		//typedef std::map<std::string, int> MapAlign;

		//static MessageBoxStyle parse(const std::string& _value);

	//private:
		//const MapAlign& getValueNames();

	private:
		Enum value;
	};

} // namespace MyGUI

#endif // __MESSAGE_BOX_STYLE_H__
