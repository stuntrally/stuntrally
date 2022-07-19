#include "pch.h"
#include "Gui_Popup.h"
//#include "../Def_Str.h"
#include <stdio.h>

#include <MyGUI_Gui.h>
#include <MyGUI_OgrePlatform.h>
#include <MyGUI_Window.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_Button.h>
#include <MyGUI_InputManager.h>
using namespace MyGUI;


///----------------------------------------------------------------------------------------------------------------
GuiPopup::GuiPopup()
	:mGui(0), mWnd(0), mPlatform(0)
	,isVisible(false), btnResult(-1)
{	}

bool GuiPopup::Show(
	PopupDelegate* delegate, const UString& title, bool modal,
	const MyGUI::UString& text0, const MyGUI::UString& text1, const MyGUI::UString& text2, const MyGUI::UString& text3,
	const MyGUI::UString& edit0, const MyGUI::UString& edit1, const MyGUI::UString& edit2, const MyGUI::UString& edit3,
	const MyGUI::UString& button0, const MyGUI::UString& button1, const MyGUI::UString& button2, const MyGUI::UString& button3)
{
	if (isVisible)
		return false;
	isVisible = true;
	btnResult = -1;

	//  window    -----------------------
	int numEdits = 0;
	if (!text0.empty())  ++numEdits;  if (!text1.empty())  ++numEdits;
	if (!text2.empty())  ++numEdits;  if (!text3.empty())  ++numEdits;

	int scr_w = mPlatform->getRenderManagerPtr()->getViewSize().width, scr_h = mPlatform->getRenderManagerPtr()->getViewSize().height;
	int wnd_w = 392, wnd_h = 140 + numEdits * 80,
		wnd_x = (scr_w - wnd_w)/2, wnd_y = (scr_h - wnd_h)/2;  // center

	mWnd = mGui->createWidget<Window>("WindowC", wnd_x,wnd_y, wnd_w,wnd_h, Align::Center, "Popup", "WndPop");
	mWnd->setColour(Colour(0.8f,0.96f,1.f));
	mWnd->setAlpha(0.9f);
	mWnd->setCaption(title);

	mDelegates.clear();  // add delegate
	mDelegates += delegate;

	if (modal)
		InputManager::getInstance().addWidgetModal(mWnd);


	//  text, edit  -----------------------
	int y = 24,  yadd = 80;
	if (!text0.empty())  {
		TextBox* text = mWnd->createWidget<TextBox>("TextBox", 16,    y, 352,28, Align::Default, "PopText0");	text->setCaption(text0);
		EditBox* edit = mWnd->createWidget<EditBox>("EditBox", 16, 32+y, 352,28, Align::Default, "PopEdit0");	edit->setCaption(edit0);
		y += yadd;
	}
	if (!text1.empty())  {
		TextBox* text = mWnd->createWidget<TextBox>("TextBox", 16,    y, 352,28, Align::Default, "PopText1");	text->setCaption(text1);
		EditBox* edit = mWnd->createWidget<EditBox>("EditBox", 16, 32+y, 352,28, Align::Default, "PopEdit1");	edit->setCaption(edit1);
		y += yadd;
	}
	if (!text2.empty())  {
		TextBox* text = mWnd->createWidget<TextBox>("TextBox", 16,    y, 352,28, Align::Default, "PopText2");	text->setCaption(text2);
		EditBox* edit = mWnd->createWidget<EditBox>("EditBox", 16, 32+y, 352,28, Align::Default, "PopEdit2");	edit->setCaption(edit2);
		y += yadd;
	}
	if (!text3.empty())  {
		TextBox* text = mWnd->createWidget<TextBox>("TextBox", 16,    y, 352,28, Align::Default, "PopText3");	text->setCaption(text3);
		EditBox* edit = mWnd->createWidget<EditBox>("EditBox", 16, 32+y, 352,28, Align::Default, "PopEdit3");	edit->setCaption(edit3);
		y += yadd;
	}

	//  buttons  -----------------------
	y += 12;
	int numBtns = 0;
	if (!button0.empty())  ++numBtns;  if (!button1.empty())  ++numBtns;
	if (!button2.empty())  ++numBtns;  if (!button3.empty())  ++numBtns;

	if (numBtns == 0)  ++numBtns;  // at least 1
	int xmarg = 20, w = (wnd_w - xmarg*2) / numBtns;
	int x = wnd_w - xmarg - w;

	if (!button0.empty())  {
		Button* btn = mWnd->createWidget<Button>("Button",  x,y, w-16,36, Align::Default, "PopBtn0");  btn->setCaption(button0);
		btn->setColour(Colour(0.7f,0.85f,1.0f));
		btn->eventMouseButtonClick += newDelegate(this, &GuiPopup::ButtonClick);  x -= w;
	}
	if (!button1.empty())  {
		Button* btn = mWnd->createWidget<Button>("Button",  x,y, w-16,36, Align::Default, "PopBtn1");  btn->setCaption(button1);
		btn->setColour(Colour(0.6f,0.8f,1.0f));
		btn->eventMouseButtonClick += newDelegate(this, &GuiPopup::ButtonClick);  x -= w;
	}
	if (!button2.empty())  {
		Button* btn = mWnd->createWidget<Button>("Button",  x,y, w-16,36, Align::Default, "PopBtn2");  btn->setCaption(button2);
		btn->setColour(Colour(0.6f,0.8f,1.0f));
		btn->eventMouseButtonClick += newDelegate(this, &GuiPopup::ButtonClick);  x -= w;
	}
	if (!button3.empty())  {
		Button* btn = mWnd->createWidget<Button>("Button",  x,y, w-16,36, Align::Default, "PopBtn3");  btn->setCaption(button3);
		btn->setColour(Colour(0.6f,0.8f,1.0f));
		btn->eventMouseButtonClick += newDelegate(this, &GuiPopup::ButtonClick);  x -= w;
	}

	return true;
}
//----------------------------------------------------------------------------------------------------------------

void GuiPopup::Hide()
{
	isVisible = false;
	if (mWnd)
	{
		mWnd->setVisible(false);
		mGui->destroyWidget(mWnd);
		mWnd = 0;
	}
}

void GuiPopup::ButtonClick(MyGUI::WidgetPtr wp)
{
	//  get result button id
	btnResult = -1;
	sscanf(wp->getName().c_str(), "PopBtn%d", &btnResult);

	//  save return fields from edits
	EditBox* ed;
	ed = (EditBox*)mWnd->findWidget("PopEdit0");  if (ed)  edit0 = ed->getCaption();
	ed = (EditBox*)mWnd->findWidget("PopEdit1");  if (ed)  edit1 = ed->getCaption();
	ed = (EditBox*)mWnd->findWidget("PopEdit2");  if (ed)  edit2 = ed->getCaption();
	ed = (EditBox*)mWnd->findWidget("PopEdit3");  if (ed)  edit3 = ed->getCaption();

	//Hide();  // done in delegate
	
	mDelegates();
}
