#pragma once

//  Gui Shortcuts: find control, assign event, set value
//------------------------------------------------------------------------
//ButtonPtr btn, bchk;  ComboBoxPtr cmb;
//Slider* sl;
//TODO: make classes with methods for these..

#define Slv(name, vset)  \
	sl = app->mGUI->findWidget<Slider>(#name);  \
	if (sl && sl->eventValueChanged.empty())  sl->eventValueChanged += newDelegate(this, &CGui::sl##name);  \
	val##name = app->mGUI->findWidget<StaticText>(#name"Val",false);  \
	if (sl)  sl->setValue(vset);  sl##name(sl, vset);

#define Btn(name, event)  \
	btn = app->mGUI->findWidget<Button>(name);  \
	if (btn && btn->eventMouseButtonClick.empty())  btn->eventMouseButtonClick += newDelegate(this, &CGui::event);

#define Chk(name, event, var)  \
	bchk = app->mGUI->findWidget<Button>(name);  \
	if (bchk && bchk->eventMouseButtonClick.empty())  {  bchk->eventMouseButtonClick += newDelegate(this, &CGui::event);  }  \
	if (bchk)  bchk->setStateSelected(var);

#define Edt(edit, name, event)  \
	edit = app->mGUI->findWidget<EditBox>(name);  \
	if (edit && edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &CGui::event);		

#define Ed(name, evt)  Edt(ed##name, #name, evt)
	
#define Cmb(cmb, name, event)  \
	cmb = app->mGUI->findWidget<ComboBox>(name);  \
	if (cmb && cmb->eventComboChangePosition.empty())  cmb->eventComboChangePosition += newDelegate(this, &CGui::event);

#define Tab(tab, name, event)  \
	tab = app->mGUI->findWidget<Tab>(name);  \
	if (tab && tab->eventTabChangeSelect.empty())  tab->eventTabChangeSelect += newDelegate(this, &CGui::event);
		
		
//  checkboxes event
//------------------------------------------------------------------------
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateSelected(pSet->var);  }


//  slider event and its text field for value
#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
#define SL   MyGUI::Slider* wp, float val     // slider event args
#define CMB  MyGUI::ComboBox* wp, size_t val  // combobox event args
#define TAB  MyGUI::Tab* tab, size_t id       //  tab event args

namespace MyGUI {  class Widget;  }
typedef MyGUI::Widget* WP;

typedef std::list <std::string> strlist;
