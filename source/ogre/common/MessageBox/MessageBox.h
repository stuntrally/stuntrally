/*!
	@file
	@author		Albert Semenov
	@date		12/2010
*/
#ifndef __MESSAGE_BOX_H__
#define __MESSAGE_BOX_H__

//#include <MyGUI.h>
#include <MyGUI_Prerequest.h>

#include "MessageBoxStyle.h"
#include "BaseLayout.h"

namespace MyGUI
{
	class Message;

	typedef delegates::CMultiDelegate2<Message*, MessageBoxStyle> EventHandle_MessageBoxPtrMessageStyle;

	class Message :
		public wraps::BaseLayout
	{
	public:
		Message();

		virtual ~Message();

		/** Set caption text*/
		void setCaption(const UString& _value);

		/** Set message text*/
		void setMessageText(const UString& _value);

		/** Create button with specific name*/
		MessageBoxStyle addButtonName(const UString& _name);

		/** Set smooth message showing*/
		void setSmoothShow(bool _value);

		/** Set message icon*/
		void setMessageIcon(MessageBoxStyle _value);

		void endMessage(MessageBoxStyle _result);

		void endMessage();

		/** Create button using MessageBoxStyle*/
		void setMessageButton(MessageBoxStyle _value);

		/** Set message style (button and icon)*/
		void setMessageStyle(MessageBoxStyle _value);

		void setMessageModal(bool _value);

		/** Static method for creating message with one command
			@param
				_modal if true all other GUI elements will be blocked untill message is closed
			@param
				_style any combination of flags from ViewValueInfo
			@param
				_button1 ... _button4 specific buttons names
		*/
		static Message* createMessageBox(
			const UString& _skinName,
			const UString& _caption,
			const UString& _message,
			MessageBoxStyle _style = MessageBoxStyle::Ok | MessageBoxStyle::IconDefault,
			const std::string& _layer = "",
			bool _modal = true,
			const std::string& _button1 = "",
			const std::string& _button2 = "",
			const std::string& _button3 = "",
			const std::string& _button4 = "");

	/*events:*/
		/** Event : button on message window pressed.\n
			signature : void method(tools::Message* _sender, MessageBoxStyle _result)\n
			@param _sender widget that called this event
			@param _result - id of pressed button
		*/
		EventHandle_MessageBoxPtrMessageStyle
			eventMessageBoxResult;

	protected:
		void updateSize();

		void notifyButtonClick(Widget* _sender);

		void clearButton();

		void onKeyButtonPressed(KeyCode _key, Char _char);

		void _destroyMessage(MessageBoxStyle _result);
		
		UString getButtonName(MessageBoxStyle _style) const;

		const char* getIconName(size_t _index) const;

		const char* getButtonName(size_t _index) const;

		const char* getButtonTag(size_t _index) const;

	private:
		IntSize mOffsetText;
		TextBox* mWidgetText;

		std::string mButtonSkin;
		std::string mButtonType;
		IntSize mButtonSize;
		IntSize mButtonOffset;

		std::vector<Button*> mVectorButton;
		MessageBoxStyle mInfoOk;
		MessageBoxStyle mInfoCancel;
		bool mSmoothShow;

		std::string mDefaultCaption;
		ImageBox* mIcon;
		int mLeftOffset1;
		int mLeftOffset2;
	};

}

#endif
