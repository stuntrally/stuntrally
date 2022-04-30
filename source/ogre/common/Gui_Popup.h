#pragma once

#include <OgrePlatform.h>

#include <MyGUI_Delegate.h>
#include <MyGUI_UString.h>

namespace MyGUI {  class OgreD3D11Platform;  class OgrePlatform;
	class Widget;  class Window;  class Gui;  }


typedef MyGUI::delegates::CMultiDelegate0 PopupDelegates;
typedef MyGUI::delegates::CMultiDelegate0::IDelegate PopupDelegate;


///  gui popup  ---------------------------------------
//     - message box with edits (and buttons) up to 4

class GuiPopup
{
public:
	bool isVisible;
	int btnResult;  // index for the button pressed 0..3,  0 is most right  -1 invalid
	MyGUI::UString edit0,edit1,edit2,edit3;  // return captions from edits   // btnResName

	bool Show(							// returns true if shown (false if is already)
		PopupDelegate* delegate,		// delegate for callback method when closing popup
		const MyGUI::UString& title,	// window title
		bool modal = true,				// if true will be the only window clickable

		// textboxes captions (above each editbox)
		const MyGUI::UString& text0 = "", const MyGUI::UString& text1 = "",
		const MyGUI::UString& text2 = "", const MyGUI::UString& text3 = "",

		// editboxes captions at start
		const MyGUI::UString& edit0 = "", const MyGUI::UString& edit1 = "",
		const MyGUI::UString& edit2 = "", const MyGUI::UString& edit3 = "",

		// buttons captions
		const MyGUI::UString& button0 = "", const MyGUI::UString& button1 = "",
		const MyGUI::UString& button2 = "",	const MyGUI::UString& button3 = "");
	
	void Hide();		// call this in callback to close popup

	GuiPopup();
	
	MyGUI::Gui* mGui;

	MyGUI::OgrePlatform* mPlatform;
	
protected:
	void ButtonClick(MyGUI::Widget*);
	MyGUI::Window* mWnd;
	PopupDelegates mDelegates;
};
