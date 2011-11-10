#ifndef _Gui_Def_h_
#define _Gui_Def_h_


const static float res = 1000000.f;  //float slider int res
#define Fmt  sprintf


//  Gui Shortcuts: find control, assign event, set value
//------------------------------------------------------------------------
//ButtonPtr btn, bchk;  ComboBoxPtr combo;
//HScrollPtr sl;  size_t v;
//TODO: make App methods of these..

#define Slv(name, vset)  \
	sl = mGUI->findWidget<ScrollBar>(#name);  \
	if (sl)  sl->eventScrollChangePosition += newDelegate(this, &App::sl##name);  \
	val##name = (StaticTextPtr)(mWndOpts->findWidget(#name"Val"));  \
	v = vset*res;  if (sl)  sl->setScrollPosition(v);	sl##name(sl, v);

#define Btn(name, event)  \
	btn = mGUI->findWidget<Button>(name);  \
	if (btn)  btn->eventMouseButtonClick += newDelegate(this, &App::event);

#define Chk(name, event, var)  \
	bchk = mGUI->findWidget<Button>(name);  \
	if (bchk)  {  bchk->eventMouseButtonClick += newDelegate(this, &App::event);  \
		bchk->setStateSelected(var);  }

#define Edt(edit, name, event)  \
	edit = (EditPtr)mWndOpts->findWidget(name);  \
	if (edit)  edit->eventEditTextChange += newDelegate(this, &App::event);		

#define Ed(name, evt)  Edt(ed##name, #name, evt)
	
#define Cmb(cmb, name, event)  \
	cmb = mGUI->findWidget<ComboBox>(name);  \
	cmb->eventComboChangePosition += newDelegate(this, &App::event);

#define Tab(tab, name, event)  \
	tab = mGUI->findWidget<Tab>(name);  \
	tab->eventTabChangeSelect += newDelegate(this, &App::event);
		
		
//  checkboxes event
//------------------------------------------------------------------------
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateSelected(pSet->var);  }


#endif
