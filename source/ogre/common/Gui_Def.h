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
	sl = (HScrollPtr)mWndOpts->findWidget(#name);  \
	if (sl)  sl->eventScrollChangePosition += newDelegate(this, &App::sl##name);  \
	val##name = (StaticTextPtr)(mWndOpts->findWidget(#name"Val"));  \
	v = vset*res;  if (sl)  sl->setScrollPosition(v);	sl##name(sl, v);

#define Btn(name, event)  \
	btn = /*(ButtonPtr)mWndOpts->findWidget*/mGUI->findWidget<Button>(name);  \
	if (btn)  btn->eventMouseButtonClick += newDelegate(this, &App::event);

#define Chk(name, event, var)  \
	bchk = mGUI->findWidget<Button>(name);  \
	if (bchk)  {  bchk->eventMouseButtonClick += newDelegate(this, &App::event);  \
		bchk->setStateCheck(var);  }

#define Edt(edit, name, event)  \
	edit = (EditPtr)mWndOpts->findWidget(name);  \
	if (edit)  edit->eventEditTextChange += newDelegate(this, &App::event);		

#define Ed(name, evt)  Edt(ed##name, #name, evt)
	
#define Cmb(cmb, name, event)  \
	cmb = (ComboBoxPtr)mWndOpts->findWidget(name);  \
	cmb->eventComboChangePosition += newDelegate(this, &App::event);

#define Tab(tab, name, event)  \
	tab = (TabPtr)mWndOpts->findWidget(name);  \
	tab->eventTabChangeSelect += newDelegate(this, &App::event);
		
		
//  checkboxes event
//------------------------------------------------------------------------
#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateCheck(pSet->var);  }


#endif
