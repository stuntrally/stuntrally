#include "pch.h"
#include <MyGUI_Prerequest.h>
#include "MessageBox.h"
#include "MessageBoxStyle.h"
#include "BaseLayout.h"


namespace MyGUI
{

	Message::Message() :
		wraps::BaseLayout("MessageBox.layout"),
		mWidgetText(nullptr),
		mInfoOk(MessageBoxStyle::None),
		mInfoCancel(MessageBoxStyle::None),
		mSmoothShow(false),
		mIcon(nullptr),
		mLeftOffset1(0),
		mLeftOffset2(0)
	{
		assignWidget(mWidgetText, "Text", false);
		if (mWidgetText != nullptr)
		{
			mOffsetText.set(mMainWidget->getClientCoord().width - mWidgetText->getWidth(), mMainWidget->getClientCoord().height - mWidgetText->getHeight());
			mLeftOffset2 = mLeftOffset1 = mWidgetText->getLeft();
		}

		assignWidget(mIcon, "Icon", false);
		if (mIcon != nullptr)
		{
			mLeftOffset2 = mIcon->getRight() + 3;
		}

		mButtonType = Button::getClassTypeName();

		if (mMainWidget->isUserString("ButtonSkin"))
			mButtonSkin = mMainWidget->getUserString("ButtonSkin");

		Widget* widget = nullptr;
		assignWidget(widget, "ButtonPlace", false);
		if (widget != nullptr)
		{
			mButtonOffset.set(widget->getLeft(), mMainWidget->getClientCoord().height - widget->getTop());
			widget->setVisible(false);
		}

		assignWidget(widget, "ButtonTemplate", false);
		if (widget != nullptr)
		{
			mButtonSize = widget->getSize();
		}
	}

	Message::~Message()
	{
		mWidgetText = nullptr;
		mIcon = nullptr;
	}

	void Message::setCaption(const UString& _value)
	{
		mMainWidget->castType<Window>()->setCaption(_value);
	}

	/** Set message text*/
	void Message::setMessageText(const UString& _value)
	{
		if (mWidgetText != nullptr)
			mWidgetText->setCaption(_value);
		updateSize();
	}

	/** Create button with specific name*/
	MessageBoxStyle Message::addButtonName(const UString& _name)
	{
		if (mVectorButton.size() >= MessageBoxStyle::_CountUserButtons)
		{
			MYGUI_LOG(Warning, "Too many buttons in message box, ignored");
			return MessageBoxStyle::None;
		}
		MessageBoxStyle info = MessageBoxStyle(MessageBoxStyle::Enum(MYGUI_FLAG(mVectorButton.size() + MessageBoxStyle::_IndexUserButton1)));

		if (mVectorButton.empty())
			mInfoOk = info;
		mInfoCancel = info;

		Widget* widget = mMainWidget->createWidgetT(mButtonType, mButtonSkin, IntCoord(), Align::Left | Align::Bottom);
		Button* button = widget->castType<Button>();
		button->eventMouseButtonClick += newDelegate(this, &Message::notifyButtonClick);
		button->setCaption(_name);
		button->_setInternalData(info);
		mVectorButton.push_back(button);

		updateSize();
		return info;
	}

	/** Set smooth message showing*/
	void Message::setSmoothShow(bool _value)
	{
		mSmoothShow = _value;
		if (mSmoothShow)
		{
			mMainWidget->setAlpha(ALPHA_MIN);
			mMainWidget->setVisible(true);
			mMainWidget->castType<Window>()->setVisibleSmooth(true);
		}
	}

	/** Set message icon*/
	void Message::setMessageIcon(MessageBoxStyle _value)
	{
		if (nullptr == mIcon)
			return;

		if (mIcon->getItemResource() != nullptr)
		{
			mIcon->setItemName(getIconName(_value.getIconIndex()));
		}
		else
		{
			mIcon->setImageIndex(_value.getIconIndex());
		}

		updateSize();
	}

	void Message::endMessage(MessageBoxStyle _result)
	{
		_destroyMessage(_result);
	}

	void Message::endMessage()
	{
		_destroyMessage(mInfoCancel);
	}

	/** Create button using MessageBoxStyle*/
	void Message::setMessageButton(MessageBoxStyle _value)
	{
		clearButton();

		std::vector<MessageBoxStyle> buttons = _value.getButtons();

		for (size_t index = 0; index < buttons.size(); ++index)
		{
			MessageBoxStyle info = buttons[index];

			addButtonName(getButtonName(info));

			mVectorButton.back()->_setInternalData(info);

			if (mVectorButton.size() == 1)
				mInfoOk = info;
			mInfoCancel = info;
		}

		updateSize();
	}

	/** Set message style (button and icon)*/
	void Message::setMessageStyle(MessageBoxStyle _value)
	{
		setMessageButton(_value);
		setMessageIcon(_value);
	}

	void Message::setMessageModal(bool _value)
	{
		if (_value)
			InputManager::getInstance().addWidgetModal(mMainWidget);
		else
			InputManager::getInstance().removeWidgetModal(mMainWidget);
	}

	/** Static method for creating message with one command */
	Message* Message::createMessageBox(
		const UString& _skinName,
		const UString& _caption,
		const UString& _message,
		MessageBoxStyle _style,
		const std::string& _layer,
		bool _modal,
		const std::string& _button1,
		const std::string& _button2,
		const std::string& _button3,
		const std::string& _button4)
	{
		Message* mess = new Message();

		mess->setCaption(_caption);
		mess->setMessageText(_message);

		mess->setSmoothShow(true);

		mess->setMessageStyle(_style);

		if (!_button1.empty())
		{
			mess->addButtonName(_button1);
			if (!_button2.empty())
			{
				mess->addButtonName(_button2);
				if (!_button3.empty())
				{
					mess->addButtonName(_button3);
				}
			}
		}

		if (_modal)
			InputManager::getInstance().addWidgetModal(mess->mMainWidget);

		return mess;
	}


	void Message::updateSize()
	{
		ISubWidgetText* text = nullptr;
		if (mWidgetText != nullptr)
			text = mWidgetText->getSubWidgetText();
		IntSize size = text == nullptr ? IntSize() : text->getTextSize();

		if ((nullptr != mIcon) && (mIcon->getImageIndex() != ITEM_NONE))
		{
			if (size.height < mIcon->getHeight())
				size.height = mIcon->getHeight();
			size.width += mIcon->getSize().width;
		}
		size += mOffsetText;
		size.width += 3;

		int width = ((int)mVectorButton.size() * mButtonSize.width) + (((int)mVectorButton.size() + 1) * mButtonOffset.width);
		if (size.width < width)
			size.width = width;

		int offset = (size.width - width) / 2;
		offset += mButtonOffset.width;

		size.width += mMainWidget->getWidth() - mMainWidget->getClientCoord().width;
		size.height += mMainWidget->getHeight() - mMainWidget->getClientCoord().height;

		const IntSize& view = RenderManager::getInstance().getViewSize();
		mMainWidget->setCoord((view.width - size.width) / 2, (view.height - size.height) / 2, size.width, size.height);

		if (nullptr != mIcon)
		{
			if (mWidgetText != nullptr)
			{
				if (mIcon->getImageIndex() != ITEM_NONE)
					mWidgetText->setCoord(mLeftOffset2, mWidgetText->getTop(), mWidgetText->getWidth(), mWidgetText->getHeight());
				else
					mWidgetText->setCoord(mLeftOffset1, mWidgetText->getTop(), mWidgetText->getWidth(), mWidgetText->getHeight());
			}
		}

		for (auto iter = mVectorButton.begin(); iter != mVectorButton.end(); ++iter)
		{
			(*iter)->setCoord(offset, mMainWidget->getClientCoord().height - mButtonOffset.height, mButtonSize.width, mButtonSize.height);
			offset += mButtonOffset.width + mButtonSize.width;
		}
	}

	void Message::notifyButtonClick(Widget* _sender)
	{
		_destroyMessage(*_sender->_getInternalData<MessageBoxStyle>());
	}

	void Message::clearButton()
	{
		for (auto iter = mVectorButton.begin(); iter != mVectorButton.end(); ++iter)
			WidgetManager::getInstance().destroyWidget(*iter);
		mVectorButton.clear();
	}

	void Message::onKeyButtonPressed(KeyCode _key, Char _char)
	{
		//Base::onKeyButtonPressed(_key, _char);

		if ((_key == KeyCode::Return) || (_key == KeyCode::NumpadEnter))
			_destroyMessage(mInfoOk);
		else if (_key == KeyCode::Escape)
			_destroyMessage(mInfoCancel);
	}

	void Message::_destroyMessage(MessageBoxStyle _result)
	{
		eventMessageBoxResult(this, _result);

		delete this;
	}

	UString Message::getButtonName(MessageBoxStyle _style) const
	{
		size_t index = _style.getButtonIndex();
		const char* tag = getButtonTag(index);
		UString result = LanguageManager::getInstance().replaceTags(utility::toString("#{", tag, "}"));
		if (result == tag)
			return getButtonName(index);
		return result;
	}

	const char* Message::getIconName(size_t _index) const
	{
		static const size_t CountIcons = 4;
		static const char* IconNames[CountIcons + 1] = { "Info", "Quest", "Error", "Warning", "" };
		if (_index >= CountIcons)
			return IconNames[CountIcons];
		return IconNames[_index];
	}

	const char* Message::getButtonName(size_t _index) const
	{
		static const size_t Count = 9;
		static const char * Names[Count + 1] = { "Ok", "Yes", "No", "Abort", "Retry", "Ignore", "Cancel", "Try", "Continue", "" };
		if (_index >= Count)
			return Names[Count];
		return Names[_index];
	}

	const char* Message::getButtonTag(size_t _index) const
	{
		static const size_t Count = 9;
		static const char* Names[Count + 1] = { "MessageBox_Ok", "MessageBox_Yes", "MessageBox_No", "MessageBox_Abort", "MessageBox_Retry", "MessageBox_Ignore", "MessageBox_Cancel", "MessageBox_Try", "MessageBox_Continue", "" };
		if (_index >= Count)
			return Names[Count];
		return Names[_index];
	}

}
