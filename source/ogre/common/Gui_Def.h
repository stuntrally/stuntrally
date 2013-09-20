#pragma once

//  forward declare
namespace MyGUI
{
	class Gui;
	class Widget;  class Window;  class TabControl;  class TabItem;  class ComboBox;
	class Button;  class TextBox;  class ImageBox;  class EditBox;  class Slider;
	class ListBox; class MultiListBox;  class MultiList2;
	class Canvas;  class ScrollView;  class Message;
}

//  Base Gui
class BGui
{
public:
	#define TD(w,n)  typedef MyGUI::w* n
	//  short Widget names
	TD(Widget, WP);  TD(Window, Wnd);  TD(TabControl, Tab);  TD(TabItem, Tbi);  TD(ComboBox, Cmb);
	TD(Button, Btn);  TD(TextBox, Txt);  TD(ImageBox, Img);  TD(EditBox, Ed);  TD(Slider, Sl);
	TD(ListBox, Li);  TD(MultiListBox, Mli);  TD(MultiList2, Mli2);
	TD(Canvas, Can);  TD(ScrollView, Scv);
};

//  short Arguments for events
//  slider event and its text field for value
#define SLV(name)  void sl##name(SL);  Txt val##name;  //old!
#define SL   Sl wp, float val     // slider event args  //old!

#define CMB  Cmb wp, size_t val  // combobox event args
#define TAB  Tab tab, size_t id       //  tab event args

//  short Finding widgets
#define fWnd(s)  mGui->findWidget<Window>(s)
#define fTxt(s)  mGui->findWidget<TextBox>(s)
#define fImg(s)  mGui->findWidget<ImageBox>(s)
#define fEd(s)   mGui->findWidget<EditBox>(s)
#define fTab(s)  mGui->findWidget<TabControl>(s)


//  Gui Shortcuts: find control, assign event, set value
//------------------------------------------------------------------------
//ButtonPtr btn, bchk;  ComboBoxPtr cmb;
//Slider* sl;
//TODO: make classes with methods for these..

#define Slv(name, vset)  \
	sl = app->mGui->findWidget<Slider>(#name);  \
	if (sl && sl->eventValueChanged.empty())  sl->eventValueChanged += newDelegate(this, &CGui::sl##name);  \
	val##name = app->mGui->findWidget<StaticText>(#name"Val",false);  \
	if (sl)  sl->setValue(vset);  sl##name(sl, vset);

#define Btn(name, event)  \
	btn = app->mGui->findWidget<Button>(name);  \
	if (btn && btn->eventMouseButtonClick.empty())  btn->eventMouseButtonClick += newDelegate(this, &CGui::event);

#define Chk(name, event, var)  \
	bchk = app->mGui->findWidget<Button>(name);  \
	if (bchk && bchk->eventMouseButtonClick.empty())  {  bchk->eventMouseButtonClick += newDelegate(this, &CGui::event);  }  \
	if (bchk)  bchk->setStateSelected(var);

#define Edt(edit, name, event)  \
	edit = app->mGui->findWidget<EditBox>(name);  \
	if (edit && edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &CGui::event);		

#define Ed(name, evt)  Edt(ed##name, #name, evt)
	
#define Cmb(cmb, name, event)  \
	cmb = app->mGui->findWidget<ComboBox>(name);  \
	if (cmb && cmb->eventComboChangePosition.empty())  cmb->eventComboChangePosition += newDelegate(this, &CGui::event);

#define Tab(tab, name, event)  \
	tab = app->mGui->findWidget<TabControl>(name);  \
	if (tab && tab->eventTabChangeSelect.empty())  tab->eventTabChangeSelect += newDelegate(this, &CGui::event);
		
		
//  checkbox event
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateSelected(pSet->var);  }
