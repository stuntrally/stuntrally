#pragma once

//  forward declare
namespace MyGUI
{
	class Gui;
	class Widget;  class Window;  class TabControl;  class TabItem;  class ComboBox;
	class Button;  class TextBox;  class ImageBox;  class EditBox;  class Slider;
	class ListBox; class MultiListBox;  class MultiList2;
	class Canvas;  class ScrollView;  class Message;  class RotatingSkin;
}
class SliderValue;  class Check;


//  Base Gui
class BGui
{
public:
	#define TD(w,n)  typedef MyGUI::w* n
	///  short Widget names
	TD(Widget, WP);  TD(Window, Wnd);  TD(TabControl, Tab);  TD(TabItem, Tbi);  TD(ComboBox, Cmb);
	TD(Button, Btn);  TD(TextBox, Txt);  TD(ImageBox, Img);  TD(EditBox, Ed);  TD(Slider, Sl);
	TD(ListBox, Li);  TD(MultiListBox, Mli);  TD(MultiList2, Mli2);
	TD(Canvas, Can);  TD(ScrollView, Scv);
	typedef SliderValue SV;  typedef Check Ck;
};


///  short Arguments for events
#define CMB  Cmb wp,  size_t val
#define TAB  Tab tab, size_t id


//  declare  slider and its event  in .h
#define SlV(a)  SV sv##a;  void sl##a(SV*)

//  slider set event
#define Sev(ev)   if (sv->event.empty())  sv->event += newDelegate(this, &CGui::sl##ev)
#define SevC(ev)  if (sv->event.empty())  sv->event += newDelegate(this, &CGuiCom::sl##ev)

//  declare  check and its event  in .h
#define CK(a)   Ck ck##a;  void chk##a(Ck*)

//  check set event
#define Cev(ev)   if (ck->event.empty())  ck->event += newDelegate(this, &CGui::chk##ev)
#define CevC(ev)  if (ck->event.empty())  ck->event += newDelegate(this, &CGuiCom::chk##ev)


///  short Finding widgets
#define fWP(s)   mGui->findWidget<Widget>(s)
#define fWnd(s)  mGui->findWidget<Window>(s)
#define fBtn(s)  mGui->findWidget<Button>(s)

#define fTxt(s)  mGui->findWidget<TextBox>(s,false)
#define fImg(s)  mGui->findWidget<ImageBox>(s)

#define fEd(s)   mGui->findWidget<EditBox>(s)
#define fCmb(s)  mGui->findWidget<ComboBox>(s)

#define fLi(s)   mGui->findWidget<List>(s)
#define fMli(s)  mGui->findWidget<MultiList>(s)

#define fTab(s)  mGui->findWidget<TabControl>(s)
#define fTbi(s)  mGui->findWidget<TabItem>(s)

#define Tev(tb, evt)  tb->eventTabChangeSelect += newDelegate(this, &CGui::tab##evt)
#define Lev(li, evt)  li->eventListChangePosition += newDelegate(this, &CGui::list##evt)

#define fTabW(s)  tab = fTab(s); \
	tab->setIndexSelected(1);  tab->setSmoothShow(false); \
	tab->eventTabChangeSelect += newDelegate(this, &CGui::tabMainMenu);


///  find control, assign event, set value (old)
//------------------------------------------------------------------------
//Btn btn, bchk;  Cmb cmb;  Sl* sl;

//  button
#define Btn(name, event)  \
	btn = fBtn(name);  \
	if (btn && btn->eventMouseButtonClick.empty())  btn->eventMouseButtonClick += newDelegate(this, &CGui::event);

#define BtnC(name, event)  \
	btn = fBtn(name);  \
	if (btn && btn->eventMouseButtonClick.empty())  btn->eventMouseButtonClick += newDelegate(this, &CGuiCom::event);

//  img btn
#define ImgB(img, name, event)  \
	img = fImg(name);  \
	if (img && img->eventMouseButtonClick.empty())  img->eventMouseButtonClick += newDelegate(this, &CGui::event);


//  check
#define Chk(name, event, var)  \
	bchk = fBtn(name);  \
	if (bchk && bchk->eventMouseButtonClick.empty())  {  bchk->eventMouseButtonClick += newDelegate(this, &CGui::event);  }  \
	if (bchk)  bchk->setStateSelected(var);

#define ChkC(name, event, var)  \
	bchk = fBtn(name);  \
	if (bchk && bchk->eventMouseButtonClick.empty())  {  bchk->eventMouseButtonClick += newDelegate(this, &CGuiCom::event);  }  \
	if (bchk)  bchk->setStateSelected(var);


//  edit
#define Edt(edit, name, event)  \
	edit = fEd(name);  \
	if (edit && edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &CGui::event);

#define Ed(name, event)  Edt(ed##name, #name, event)

#define EdC(edit, name, event)  \
	edit = fEd(name);  \
	if (edit && edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &CGuiCom::event);

	
//  combo
#define Cmb(cmb, name, event)  \
	cmb = fCmb(name);  \
	if (cmb && cmb->eventComboChangePosition.empty())  cmb->eventComboChangePosition += newDelegate(this, &CGui::event);

#define CmbC(cmb, name, event)  \
	cmb = fCmb(name);  \
	if (cmb && cmb->eventComboChangePosition.empty())  cmb->eventComboChangePosition += newDelegate(this, &CGuiCom::event);


//  tab
#define Tab(tab, name, event)  \
	tab = fTab(name);  \
	if (tab && tab->eventTabChangeSelect.empty())  tab->eventTabChangeSelect += newDelegate(this, &CGui::event);


#define Radio2(bR1,bR2, b)  bR1->setStateSelected(b);  bR2->setStateSelected(!b);

		
//old  -----

//  declare slider event and its text
#define SL   Sl wp, float val
#define SLV(name)  void sl##name(SL);  Txt val##name =0;

//  slider  //old
#define Slv(name, vset)  \
	sl = mGui->findWidget<Slider>(#name);  \
	if (sl && sl->eventValueChanged.empty())  sl->eventValueChanged += newDelegate(this, &CGui::sl##name);  \
	val##name = fTxt(#name"Val");  \
	if (sl)  sl->setValue(vset);  sl##name(sl, vset);

//  check event, toggle  //old
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	Btn chk = wp->castType<Button>(); \
    chk->setStateSelected(pSet->var);  }
