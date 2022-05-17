#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/GuiCom.h"
#include "../vdrift/game.h"
#include "CGame.h"
#include "CGui.h"
#include <OgreRoot.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Button.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_TabItem.h>
#include <MyGUI_ComboBox.h>
using namespace MyGUI;
using namespace Ogre;


std::string GetKeyName(SDL_Keycode key, bool omit = false)
{
	if (key == SDLK_UNKNOWN)
		return omit ? "" : TR("#{InputKeyUnassigned}");
	else
		return std::string(SDL_GetKeyName(key));
}


///  Input caption  ---------------------
void CGui::UpdateInputButton(Button* button, const InputAction& action, EBind bind)
{
	std::string s, sAssign = TR("#FFA030#{InputAssignKey}");  // caption

	SDL_Keycode decKey = action.mICS->getKeyBinding(action.mControl, ICS::Control::DECREASE);
	SDL_Keycode incKey = action.mICS->getKeyBinding(action.mControl, ICS::Control::INCREASE);

	//  B_First:  "<Assign>"
	//  B_Second: "Key1, <Assign>"
	//  B_Done:   "Key1, Key2"

	if (action.mType == InputAction::Axis)
	{
		if (bind == B_First)
			s = sAssign;
		else
		{	s += GetKeyName(decKey,true);
			if (!s.empty())  s += " , ";
			if (bind == B_Second)
				s += sAssign;
			else
				s += GetKeyName(incKey,true);
		}
	}else
	{	if (bind == B_First)
			s = sAssign;
		else
			s += GetKeyName(incKey, action.mType & InputAction::Axis);
	}

	if (bind == B_Done)
	{
		for (int j=0; j < SDL_NumJoysticks(); ++j)
		{
			int axis = action.mICS->getJoystickAxisBinding(action.mControl, j, ICS::Control::INCREASE);
			if (axis != ICS::InputControlSystem::UNASSIGNED)
			{
				if (!s.empty())  s += " / ";
				s += "J"+toStr(j) + ".Axis " + toStr(axis);
			}
			int btn = action.mICS->getJoystickButtonBinding(action.mControl, j, ICS::Control::INCREASE);
			if (btn != ICS::InputControlSystem::UNASSIGNED)
			{
				if (!s.empty())  s += " / ";
				s += "J"+toStr(j) + ".Button " + toStr(btn);
			}
		}
	}
	if (s.empty())  s = TR("#{InputKeyUnassigned}");
	button->setCaption(s);
}


///  Gui Init - Input tabs
//----------------------------------------------------------------------------------------------------------------------------------
void CGui::CreateInputTab(int iTab, bool player,
	const std::vector<InputAction>& actions, ICS::InputControlSystem* ICS)
{
	if (!tabInput)  return;
	TabItemPtr tabitem = tabInput->getItemAt(iTab);
	std::string sPlr = toStr(iTab);

	//  dimensions
	const int sx = 150, sy = 24,  // button size
		//  columns positon x  txt,btn,bar,>
		x0 = 16, x1 = 160, x2 = 325, x3 = 464,
		yHdr0 = 12, yHdr = yHdr0+6,  // header start
		yAdd = 14,  // y add with new row
		s0 = x1-x0-5;  // descr size x

	#define CreateText(x,y, w,h, name, text)  {  Txt txt =  \
		tabitem->createWidget<TextBox>("TextBox", x,y+2, w,h, Align::Default, name);  \
		gcom->setOrigPos(txt, "OptionsWnd");  \
		txt->setCaption(text);  }
	

	///  Headers  action, binding, value
	CreateText(x0,yHdr0, sx,sy, "hdrTxt1_"+sPlr, TR("#90B0F0#{InputHeaderTxt1}"));
	CreateText(x1,yHdr0, sx,sy, "hdrTxt2_"+sPlr, TR("#A0C0FF#{InputHeaderTxt2}"));
	if (player)  {
		CreateText(x2,yHdr0, sx,sy, "hdrTxt3_"+sPlr, TR("#90B0F0#{InputHeaderTxt3}"));
		CreateText(x3,yHdr0, sx,sy, "hdrTxt4_"+sPlr, TR("#80A0E0#{InputHeaderTxt4}"));  }

	//  spacing for add y
	static std::map <std::string, int> yRow;
	int y = 2;
	if (yRow.empty())
	{	//  player
		yRow["Throttle"]=y;   y+=2;     yRow["Brake"]=y;       y+=2;
		yRow["Steering"]=y;   y+=2 +1;  yRow["HandBrake"]=y;   y+=2;
		yRow["Boost"]=y;      y+=2;     yRow["Flip"]=y;        y+=2;
		yRow["Rewind"]=y;     y+=2 +2;
		yRow["NextCamera"]=y; y+=2;     yRow["PrevCamera"]=y;  y+=2 +1;
		yRow["ShiftUp"]=y;    y+=2;     yRow["ShiftDown"]=y;   y+=2 +1;
		yRow["LastChk"]=y;    y+=2;
		//  general
		y = 2;
		yRow["ShowOptions"]=y; y+=2 +1;
		yRow["RestartGame"]=y; y+=2;    yRow["ResetGame"]=y;  y+=2 +1;
		yRow["Screenshot"]=y;  y+=2 +1;
		yRow["PrevTab"]=y;     y+=2;    yRow["NextTab"]=y;    y+=2 +2;
	}

	///  Actions  ------------------------------------------------
	int i = 0;
	for (auto it = actions.begin(); it != actions.end(); ++it)
	{
		std::string name = it->mName;
		y = yHdr + yRow[name] * yAdd;

		//  description label  ----------------
		Txt desc = tabitem->createWidget<TextBox>("TextBox",
			x0, y+5, s0, sy,  Align::Default);
		gcom->setOrigPos(desc, "OptionsWnd");
		desc->setCaption( TR("#{InputMap" + name + "}") );
		desc->setTextColour( !player ?
			(i==0||i==3||i==4 ? Colour(0.86f,0.93f,1.f) : Colour(0.6f,0.8f,0.95f)) :  // general

			i==11 ? Colour(0.8f,0.6f,1.f) : // rewind
			i==10 ? Colour(0.55f,0.5f,0.6f) : // last chk-
			(i==4||i==5) ? Colour(0.4f,1.f,1.f) : // boost,flip
			(i==6||i==7) ? Colour(0.7f,0.7f,0.7f) : // gear
			(i>=8 ? Colour(0.6f,0.75f,1.f) : // camera
			Colour(0.75f,0.88f,1.f)) );  // car steer
		//desc->setTextShadow(true);

		//  bind info
		bool analog = it->mType & InputAction::Axis;
		bool twosided = it->mType == InputAction::Axis;

		//  binding button  ----------------
		Btn btn1 = tabitem->createWidget<Button>("Button",
			x1, y, sx, sy,  Align::Default);
		gcom->setOrigPos(btn1, "OptionsWnd");
		UpdateInputButton(btn1, *it);
		btn1->eventMouseButtonClick += newDelegate(this, &CGui::inputBindBtnClicked);
		btn1->eventMouseButtonPressed += newDelegate(this, &CGui::inputBindBtn2);
		btn1->setUserData(*it);
		Colour clr =    !player ? Colour(0.7f,0.9f,1.f) :
			(analog ? (twosided ? Colour(0.7f,0.7f,1.f) : Colour(0.5f,0.75f,1.f)) : Colour(0.6f,0.9f,1.f));
		Colour txclr =  !player ? Colour(0.8f,1.f,1.f) :
			(analog ? (twosided ? Colour(0.9f,0.9f,1.f) : Colour(0.85f,0.93f,1.f)) : Colour(0.8f,1.f,1.f));
		btn1->setColour(clr);
		btn1->setTextColour(txclr);

		//  value bar  --------------
		if (player)
		{
			Img bar = tabitem->createWidget<ImageBox>("ImageBox",
				x2 + (twosided ? 0 : 64), y+4, twosided ? 128 : 64, 16, Align::Default,
				"bar_" + toStr(i) + "_" + sPlr);
			gcom->setOrigPos(bar, "OptionsWnd");
			bar->setUserData(*it);
			bar->setImageTexture(String("input_bar.png"));  bar->setImageCoord(IntCoord(0,0,128,16));
		}

		//  detail btn  ----------------
		if (analog)
		{	btn1 = tabitem->createWidget<Button>("Button",
				x3, y, 32, sy,  Align::Default,
				"inputdetail_" + toStr(i) + "_" + sPlr + "_1");
			gcom->setOrigPos(btn1, "OptionsWnd");
			btn1->setCaption(">");
			btn1->setTextColour(Colour(0.6f,0.7f,0.8f));
			btn1->setColour(Colour(0.6f,0.8f,1.0f));
			btn1->setUserData(*it);
			btn1->eventMouseButtonClick += newDelegate(this, &CGui::inputDetailBtn);
		}
		
		//  icon  --------------
		#define CrtImg(x,y, ux,uy)  {  Img img =  \
			tabitem->createWidget<ImageBox>("ImageBox", x-32,y, 28,28, Align::Default);  \
			gcom->setOrigPos(img, "OptionsWnd");  \
			img->setAlpha(0.9f);  img->setImageCoord(IntCoord(ux,uy,128,128));  \
			img->setImageTexture("gui_icons.png");  }

		if (player)  switch (i)
		{	//case 0:  CrtImg(x1,y, 128,256);  break;  // gauge
			case 2:  CrtImg(x1,y, 128,384);  break;  // steer
			case 3:  CrtImg(x1,y, 768,512);  break;  // handbrake

			case 6:  CrtImg(x1,y, 128,512);  break;  // gear
			case 9:  CrtImg(x1,y, 0,512);  break;    // camera

			case 4:  CrtImg(x1,y, 512,0);  break;    // boost
			case 5:  CrtImg(x1,y, 512,128);  break;  // flip
			
			//case 7:  CrtImg(x1,y, 512,256);  break;  // damage
			case 11:  CrtImg(x1,y, 512,384);  break; // rewind
		}	++i;
	}

	if (player)
	{	y = yHdr + 32 * yAdd;
		CreateText(x1,y, 500,24, "txtunb" + sPlr, TR("#80B0F0#{InputUnbind}"));
	}
}

void CGui::InitInputGui()
{
	app->input->LoadInputDefaults();


	txtInpDetail = fTxt("InputDetail");
	panInputDetail = fWP("PanInputDetail");

	Tab(tabInput, "SubTabInput", tabInputChg);
	if (!tabInput)  return;

	//  details edits
	Btn btn, bchk;
	Btn("InputInv", btnInputInv);  //Ed(InputMul, editInput);
	Ed(InputIncrease, editInput);  //Ed(InputReturn, editInput);
	Chk("OneAxisThrBrk", chkOneAxis, false);  chOneAxis = bchk;

	//  key emul presets combo
	Cmb cmb;
	Cmb(cmb, "CmbInputKeysAllPreset", comboInputKeyAllPreset);
	if (cmb)
	{	cmb->removeAllItems();  cmb->addItem("");
		cmb->addItem(TR("#{InpSet_Slow}"));
		cmb->addItem(TR("#{InpSet_Medium}"));
		cmb->addItem(TR("#{InpSet_Fast}"));
	}
	
	///  fill global and 4 players tabs
	CreateInputTab(4, false, app->input->mInputActions, app->mInputCtrl);
	for (int i=0; i < 4; ++i)
		CreateInputTab(i, true, app->input->mInputActionsPlayer[i], app->mInputCtrlPlayer[i]);
}


///  Bind Input
//----------------------------------------------------------------------------------------------------------------------------------

void CGui::inputBindBtn2(WP sender, int, int, MouseButton mb)
{
	if (mb == MouseButton::Right)
		inputUnbind(sender);
}

void CGui::inputBindBtnClicked(WP sender)
{
	sender->castType<Button>()->setCaption( TR("#FFA030#{InputAssignKey}"));

	InputAction* action = sender->getUserData<InputAction>();
	mBindingAction = action;
	mBindingSender = sender->castType<Button>();

	if (mBindingAction->mType == InputAction::Axis)
	{	// bind decrease (ie left) first
		action->mICS->enableDetectingBindingState(action->mControl, ICS::Control::DECREASE);
	}else
		action->mICS->enableDetectingBindingState(action->mControl, ICS::Control::INCREASE);

	UpdateInputButton(mBindingSender, *action, B_First);

	// activate key capture mode
	app->bAssignKey = true;
	app->hideMouse();
}

void CGui::notifyInputActionBound(bool complete)
{	
	UpdateInputButton(mBindingSender, *mBindingAction, complete ? B_Done : B_Second);
	if (complete)
	{	app->bAssignKey = false;

		// If a key was assigned that used to belong to another control, it will now be unassigned,
		// so we need to force-update button labels
		TabControl* inputTab = fTab("SubTabInput");  if (!inputTab)  return;
		TabItem* current = inputTab->getItemSelected();
		for (int i=0; i < current->getChildCount(); ++i)
		{
			Button* button = current->getChildAt(i)->castType<Button>(false);
			if (!button || button->getCaption() == ">") // HACK: we don't want the detail buttons
				continue;
			if (button->getUserData<InputAction>() != mBindingAction)
				UpdateInputButton(button, *button->getUserData<InputAction>());
		}
	}
}

void CGui::inputUnbind(WP sender)
{
	InputAction* action = sender->getUserData<InputAction>();
	mBindingAction = action;
	mBindingSender = sender->castType<Button>();

	SDL_Keycode key = action->mICS->getKeyBinding(action->mControl, ICS::Control::INCREASE);
	action->mICS->removeKeyBinding(key);

	key = action->mICS->getKeyBinding(action->mControl, ICS::Control::DECREASE);
	action->mICS->removeKeyBinding(key);

	for (int j=0; j < SDL_NumJoysticks(); ++j)
	{
		int axis = action->mICS->getJoystickAxisBinding(action->mControl, j, ICS::Control::INCREASE);
		if (axis != ICS::InputControlSystem::UNASSIGNED)
			action->mICS->removeJoystickAxisBinding(j, axis);
		
		int btn = action->mICS->getJoystickButtonBinding(action->mControl, j, ICS::Control::INCREASE);
		if (btn != ICS::InputControlSystem::UNASSIGNED)
			action->mICS->removeJoystickButtonBinding(j, btn);
	}
	UpdateInputButton(mBindingSender, *action);
}


///  edit details
//-------------------------------------------------------------------------------
void CGui::inputDetailBtn(WP sender)
{
	const InputAction& action = *sender->getUserData<InputAction>();
	if (txtInpDetail)
		txtInpDetail->setCaptionWithReplacing( TR("#{InputDetailsFor}")+":  #{InputMap"+action.mName+"}");

	mBindingAction = sender->getUserData<InputAction>();
	if (panInputDetail)
		panInputDetail->setVisible(false);

	Button* btnInputInv = fBtn("InputInv");
	if (btnInputInv)
		btnInputInv->setStateSelected( mBindingAction->mControl->getInverted());
	if (edInputIncrease)
		edInputIncrease->setCaption( toStr(action.mControl->getStepSize() * action.mControl->getStepsPerSeconds()));
}

void CGui::editInput(EditPtr ed)
{
	Real vInc = s2r(edInputIncrease->getCaption());
	mBindingAction->mControl->setStepSize(0.1);
	mBindingAction->mControl->setStepsPerSeconds(vInc*10);
}

void CGui::btnInputInv(WP wp)
{
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(!chk->getStateSelected());
	mBindingAction->mControl->setInverted(chk->getStateSelected());
}

void CGui::chkOneAxis(WP wp)
{
	int id=0;  if (!TabInputId(&id))  return;
	ButtonPtr chk = wp->castType<Button>();
	bool b = !app->mInputCtrlPlayer[id]->mbOneAxisThrottleBrake;
	app->mInputCtrlPlayer[id]->mbOneAxisThrottleBrake = b;
    chk->setStateSelected(b);
}

void CGui::tabInputChg(TabPtr tab, size_t val)
{
	int id=0;  bool vis = TabInputId(&id);
	chOneAxis->setVisible(vis);
	//txtInpDetail;  panInputDetail;
	//edInputIncrease;
	if (vis)
	{
		bool b = app->mInputCtrlPlayer[id]->mbOneAxisThrottleBrake;
		chOneAxis->setStateSelected(b);
	}
}

//  returns player id 0..3, false if not player tab
bool CGui::TabInputId(int* pId)
{
	if (!tabInput)  return false;
	int id = tabInput->getIndexSelected();
	if (id > 3)  return false;
	*pId = id;  return true;
}

void CGui::comboInputKeyAllPreset(ComboBoxPtr cmb, size_t val)
{
	if (val == 0)  return;  cmb->setIndexSelected(0);
	int id=0;  if (!TabInputId(&id))  return;

	const int numActs = 6;  // these actions have key emul params (analog)
	int keyActs[numActs] = {A_Boost, A_Brake, A_Flip, A_HandBrake, A_Steering, A_Throttle};
	const Real speeds[3] = {2.f, 3.f, 4.f};
	Real vInc = speeds[val-1];

	for (int i=0; i < numActs; ++i)
	{
		ICS::Control* control = app->mInputCtrlPlayer[id]->getControl(keyActs[i]);

		control->setStepSize(0.1f);
		control->setStepsPerSeconds(vInc*10.f);
	}
	if (edInputIncrease)  edInputIncrease->setCaption(toStr(vInc));
}


///  update input bars vis,dbg
//-------------------------------------------------------------------------------
void CGui::UpdateInputBars()
{
	TabControl* inputTab = fTab("SubTabInput");  if (!inputTab)  return;
	TabItem* current = inputTab->getItemSelected();
	for (int i=0; i<current->getChildCount(); ++i)
	{
		ImageBox* image = current->getChildAt(i)->castType<ImageBox>(false);
		if (!image)  continue;
		InputAction* ia = image->getUserData<InputAction>(false);
		if (!ia)  continue;
		
		const InputAction& action = *ia;
		float val = action.mICS->getChannel(action.mId)->getValue();

		const int wf = 128, w = 256;
		int v = -val * 128, vf = -(val*2-1) * 64, s=512, s0=s/2;

		bool full = action.mType == InputAction::Axis;

		if (full)	image->setImageCoord(IntCoord(std::max(0, std::min(s-wf, vf + s0 -wf/2)), 0, wf, 16));
		else		image->setImageCoord(IntCoord(std::max(0, std::min(s-w,  v  + s0))      , 0, w, 16));
	}
}

//  Bind Key   . . . . .
void CGui::keyBindingDetected(
	ICS::InputControlSystem* pICS, ICS::Control* control,
	SDL_Keycode key,
	ICS::Control::ControlChangingDirection direction)
{
	ICS::DetectingBindingListener::keyBindingDetected(pICS, control, key, direction);
	
	if (direction == ICS::Control::DECREASE)
	{
		pICS->enableDetectingBindingState(control, ICS::Control::INCREASE);
		notifyInputActionBound(false);  //  second key still needs binding
	}else
		notifyInputActionBound(true);  //  done
}

//  Bind Joy Axis   . . . . .
void CGui::joystickAxisBindingDetected(
	ICS::InputControlSystem* pICS, ICS::Control* control,
	int deviceId, int axis,
	ICS::Control::ControlChangingDirection direction)
{
	ICS::DetectingBindingListener::joystickAxisBindingDetected(
		pICS, control, deviceId, axis, ICS::Control::INCREASE);

	std::string s = control->getName();
	//LogO("Control "+s);

	///  inverted throttle and brake by default
	bool inv = s != "2" && s != "5";  //  only steering and flip normal
	control->setInverted(inv);
	
	notifyInputActionBound(true);
}

//  Bind Joy Button  . . . . .
void CGui::joystickButtonBindingDetected(
	ICS::InputControlSystem* pICS, ICS::Control* control,
	int deviceId, unsigned int button,
	ICS::Control::ControlChangingDirection direction)
{
	//  2-sided axis can't be bound with a JS button
	if (mBindingAction->mType == InputAction::Axis)
		return;

	ICS::DetectingBindingListener::joystickButtonBindingDetected(
		pICS, control, deviceId, button, ICS::Control::INCREASE);
	
	notifyInputActionBound(true);
}
