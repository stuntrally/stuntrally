#ifndef _Gui_Def_h_
#define _Gui_Def_h_


//  Gui Shortcuts: find control, assign event, set value
//------------------------------------------------------------------------
//ButtonPtr btn, bchk;  ComboBoxPtr combo;
//Slider* sl;  size_t v;
//TODO: make App methods of these..

#define Slv(name, vset)  \
	sl = mGUI->findWidget<Slider>(#name);  \
	if (sl && sl->eventValueChanged.empty())  sl->eventValueChanged += newDelegate(this, &App::sl##name);  \
	val##name = mGUI->findWidget<StaticText>(#name"Val",false);  \
	if (sl)  sl->setValue(vset);  sl##name(sl, vset);

#define Btn(name, event)  \
	btn = mGUI->findWidget<Button>(name);  \
	if (btn && btn->eventMouseButtonClick.empty())  btn->eventMouseButtonClick += newDelegate(this, &App::event);

#define Chk(name, event, var)  \
	bchk = mGUI->findWidget<Button>(name);  \
	if (bchk && bchk->eventMouseButtonClick.empty())  {  bchk->eventMouseButtonClick += newDelegate(this, &App::event);  \
		bchk->setStateSelected(var);  }

#define Edt(edit, name, event)  \
	edit = mGUI->findWidget<EditBox>(name);  \
	if (edit && edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &App::event);		

#define Ed(name, evt)  Edt(ed##name, #name, evt)
	
#define Cmb(cmb, name, event)  \
	cmb = mGUI->findWidget<ComboBox>(name);  \
	if (cmb && cmb->eventComboChangePosition.empty())  cmb->eventComboChangePosition += newDelegate(this, &App::event);

#define Tab(tab, name, event)  \
	tab = mGUI->findWidget<Tab>(name);  \
	if (tab && tab->eventTabChangeSelect.empty()) tab->eventTabChangeSelect += newDelegate(this, &App::event);
		
		
//  checkboxes event
//------------------------------------------------------------------------
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateSelected(pSet->var);  }


const float slHalf = 0.45f;  // added to int value sliders to their float value

#endif
