#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "common/Gui_Def.h"

#include <OgreRoot.h>
using namespace MyGUI;
using namespace Ogre;

#define ALIGN  Align::Default  // MyGUI 3.2 has no Align::Relative


///  Gui Init - input tab
//----------------------------------------------------------------------------------------------------------------------------------
/*
String App::GetInputName(const String& sName)
{
	Ogre::vector<String>::type vs = StringUtil::split(sName, "/");
	if (vs.size() != 2)
		return sName;

	if (vs[0] == "Keyboard")
		return StrFromKey(sName);
	else
		return vs[1];
}
*/

void App::InitInputGui()
{
	TabItemPtr inpTabAll = mGUI->findWidget<TabItem>("InputTabAll");  if (!inpTabAll)  return;
	TabPtr inputTab = mGUI->findWidget<Tab>("InputTab");  if (!inputTab)  return;

	///  controller selection combo (for bind name, when more)
	ComboBoxPtr cmbJoy = mGUI->findWidget<ComboBox>("CmbInputController");
	if (cmbJoy)
	{
		for (int i=0; i < mJoysticks.size(); ++i)
			cmbJoy->addItem( SDL_JoystickName(mJoysticks[i]) );

		cmbJoy->eventComboChangePosition += newDelegate(this, &App::cmbJoystick);
		if (mJoysticks.size() > 0)  {  cmbJoy->setIndexSelected(0);  cmbJoystick(cmbJoy, 0);  }
	}/*

	//  labels that print the last pressed joystick button / last moved axis
	txtJAxis = mGUI->findWidget<StaticText>("axisOutput");
	txtJBtn = mGUI->findWidget<StaticText>("buttonOutput");
	txtInpDetail = mGUI->findWidget<StaticText>("InputDetail");

	//  details edits
	Ed(InputMin, editInput);  Ed(InputMax, editInput);  Ed(InputMul, editInput);
	Ed(InputReturn, editInput);  Ed(InputIncrease, editInput);

	//  analog presets combo
	ComboBoxPtr combo;
	Cmb(combo, "CmbInputDetPreset", comboInputPreset);  cmbInpDetSet = combo;
	if (combo)
	{	combo->removeAllItems();  combo->addItem("");
		combo->addItem(TR("#{InpSet_KeyHalf}"));
		combo->addItem(TR("#{InpSet_KeyFull}"));
		combo->addItem(TR("#{InpSet_AxisHalf}"));
		combo->addItem(TR("#{InpSet_AxisHalfInv}"));
		combo->addItem(TR("#{InpSet_AxisFull}"));
		combo->addItem(TR("#{InpSet_AxisFullInv}"));
    }
	//  key emul presets combo
	Cmb(combo, "CmbInputKeysAllPreset", comboInputKeyAllPreset);
	if (combo)
	{	combo->removeAllItems();  combo->addItem("");
		combo->addItem(TR("#{InpSet_Slow}"));
		combo->addItem(TR("#{InpSet_Medium}"));
		combo->addItem(TR("#{InpSet_Fast}"));
	}


	#define CreateText(x,y, w,h, name, text)  {  StaticTextPtr txt =  \
		tabitem->createWidget<TextBox>("TextBox", x,y, w,h, ALIGN, name);  \
		setOrigPos(txt, "OptionsWnd");  \
		if (txt)  txt->setCaption(text);  }
	
	//  button size and columns positon
	const int sx = 130, sy = 24,  x0 = 20, x1 = 140, x2 = 285, x3 = 430,  yh = 20,  s0 = x1-x0-5;


	///  insert a tab item for every schema (4players,global)
	std::map<OISB::String, OISB::ActionSchema*> schemas = sys.mActionSchemas;  int i=0;
	for (std::map<OISB::String, OISB::ActionSchema*>::const_iterator it = schemas.begin(); it != schemas.end(); ++it,++i)
	{
		const OISB::String& sPlr = (*it).first;
		bool playerTab = Ogre::StringUtil::startsWith( sPlr, "player");
		TabItemPtr tabitem = inputTab->addItem(!playerTab ? TR("#80C0FF#{InputMapGeneral}") : ((i>1 ? "#FFF020" : TR("#FFF850#{Player} ")) + toStr(i)));

		///  Headers  action, binding, value
		CreateText(x0,yh, sx,sy, "hdrTxt1_"+sPlr, TR("#90B0F0#{InputHeaderTxt1}"));
		CreateText(x1,yh, sx,sy, "hdrTxt2_"+sPlr, TR("#A0C0FF#{InputHeaderTxt2}"));
		if (playerTab)  {
			CreateText(x2,yh, sx,sy, "hdrTxt3_"+sPlr, TR("#90B0F0#{InputHeaderTxt3}"));
			CreateText(x3,yh, sx,sy, "hdrTxt4_"+sPlr, TR("#80A0E0#{InputHeaderTxt4}"));  }
		
		///  ------ custom action sorting ----------------
		int i = 0, y = 0, ya = 26 / 2, yc=0, y0 = yh + 28;
		std::map <std::string, int> yRow;
		// player
		yRow["Throttle"] = y;	y+=2;	yRow["Brake"] = y;	y+=2;	yRow["Steering"] = y;	y+=2+2 +1;
		yRow["HandBrake"] = y;	y+=2;	yRow["Boost"] = y;	y+=2;	yRow["Flip"] = y;		y+=2+2 +2;
		yRow["ShiftUp"] = y;	y+=2;	yRow["ShiftDown"] = y;	y+=2 +1;
		yRow["PrevCamera"] = y; y+=2;	yRow["NextCamera"] = y; y+=2+1;  //yc = 40 + ya * y;
		yRow["LastChk"] = y;	y+=2;   yRow["Rewind"] = y;  y+=2;
		y = 0;  // general
		yRow["ShowOptions"] = y; y+=2+1;
		yRow["PrevTab"] = y;     y+=2;	yRow["NextTab"] = y;    y+=2+1;
		yRow["RestartGame"] = y; y+=2;	yRow["ResetGame"] = y;  y+=2+1;
		yRow["Screenshot"] = y;  y+=2;	yc = y0 + ya * y;

		if (!playerTab)
		{	y = yc+2*ya;  //  camera infos
			CreateText(20,y, 280,24, "txtcam1", TR("#A0D0F0#{InputMapNextCamera} / #{InputMapPrevCamera}"));  y+=2*ya;
			CreateText(40,y, 280,24, "txtcam2", TR("#A0D0F0#{InputCameraTxt1}"));  y+=3*ya;
			//  replay controls info text
			CreateText(20,y, 500,24, "txtrpl1", TR("#A0D0F0#{Replay}:"));  y+=2*ya;
			CreateText(40,y, 500,24, "txtrpl2", TR("#80B0F0#{InputRplCtrl1}"));  y+=2*ya;
			CreateText(40,y, 500,24, "txtrpl3", TR("#80B0F0#{InputRplCtrl2}"));  y+=2*ya;
			CreateText(40,y, 500,24, "txtrpl4", TR("#80B0F0#{InputRplCtrl3}"));  y+=2*ya;
			CreateText(40,y, 500,24, "txtrpl5", TR("#60A0D0#{InputRplCtrl4}"));  y+=2*ya;
		}
		
		///  Actions  ------------------------------------------------
		for (std::map<OISB::String, OISB::Action*>::const_iterator
			ait = (*it).second->mActions.begin();
			ait != (*it).second->mActions.end(); ++ait,++i)
		{
			const OISB::String& sAct = (*ait).first;
			OISB::Action* act = (*ait).second;

			const String& name = (*ait).second->getName();
			y = y0 + ya * yRow[name];

			//  description label
			StaticTextPtr desc = tabitem->createWidget<TextBox>("TextBox",
				x0, y+5, s0, sy,  ALIGN,
				"staticText_" + sAct );
			setOrigPos(desc, "OptionsWnd");
			desc->setCaption( TR("#{InputMap" + name + "}") );
		
			//  Keyboard binds  --------------------------------
			//  get information about binds from OISB and set variables how the rebind buttons should be created
			std::string skey1 = TR("#{InputKeyUnassigned}");
			std::string skey2 = TR("#{InputKeyUnassigned}");
			
			//  bound key(s)
			bool analog = act->getActionType() == OISB::AT_ANALOG_AXIS;
			bool button2 = act->getName() == "Steering" || act->getName() == "Flip";  // full
			if (!act->mBindings.empty() && act->mBindings.front()->getNumBindables() > 0 &&
				act->mBindings.front()->getBindable(0) && act->mBindings.front()->getBindable(0) != (OISB::Bindable*)1)
				if (act->getActionType() == OISB::AT_TRIGGER)
				{
					skey1 = GetInputName(act->mBindings.front()->getBindable(0)->getBindableName());
				}
				else if (analog)
				{
					//  look for increase/decrease binds
					OISB::Bindable* increase = NULL, *decrease = NULL, *none = NULL;
					for (std::vector<std::pair<String, OISB::Bindable*> >::const_iterator
						bnit = act->mBindings.front()->mBindables.begin();
						bnit != act->mBindings.front()->mBindables.end(); bnit++)
					{
						if ((*bnit).first == "inc")			increase = (*bnit).second;
						else if ((*bnit).first == "dec")	decrease = (*bnit).second;
						else none = (*bnit).second;
					}
					if (increase)  skey1 = GetInputName(increase->getBindableName());
					if (decrease)  skey2 = GetInputName(decrease->getBindableName());
					if (none)
						(button2 ? skey2 : skey1) = GetInputName(none->getBindableName());
				}
				
			//  binding button  ----------------
			ButtonPtr btn1 = tabitem->createWidget<Button>("Button",
				x1, button2 ? (y + ya*2) : y, sx, sy,  ALIGN,
				"inputbutton_" + sAct + "_" + sPlr + "_1");
			setOrigPos(btn1, "OptionsWnd");
			btn1->setCaption( skey1 );
			btn1->eventMouseButtonClick += newDelegate(this, &App::inputBindBtnClicked);
			
			if (button2)
			{	ButtonPtr btn2 = tabitem->createWidget<Button>("Button",
					x1, y, sx, sy,  ALIGN,
					"inputbutton_" + sAct + "_" + sPlr + "_2");
				btn2->setCaption( skey2 );
				setOrigPos(btn2, "OptionsWnd");
				btn2->eventMouseButtonClick += MyGUI::newDelegate(this, &App::inputBindBtnClicked);
			}
			
			//  value bar  --------------
			if (playerTab)
			{
				StaticImagePtr bar = tabitem->createWidget<ImageBox>("ImageBox",
					x2 + (button2 ? 0 : 64), y+4, button2 ? 128 : 64, 16, ALIGN,
					"bar_" + sAct + "_" + sPlr);
				setOrigPos(bar, "OptionsWnd");
				bar->setImageTexture(String("input_bar.png"));  bar->setImageCoord(IntCoord(0,0,128,16));
			}

			//  detail btn  ----------------
			if (analog)
			{	btn1 = tabitem->createWidget<Button>("Button",
					x3, y, 32, sy,  ALIGN,
					"inputdetail_" + sAct + "_" + sPlr + "_1");
				setOrigPos(btn1, "OptionsWnd");
				btn1->setCaption(">");
				btn1->setColour(Colour(0.6f,0.8f,1.0f));
				btn1->eventMouseButtonClick += newDelegate(this, &App::inputDetailBtn);
			}
		}
	}
	
	TabItemPtr tabitem = inputTab->addItem(TR("#C0C0FF#{Other}"));
	int y = 32, ya = 26 / 2, yb = 20 / 2,  xa = 20, xa1=xa+16, xb = 250, xb1=xb+16;
	CreateText(xa,y, 500,24, "txtoth1", TR("#A0D0FF#{InputOther1}"));  y+=2*ya;
	CreateText(xa,y, 500,24, "txtoth2", TR("#A0D0FF#{InputOther2}"));  y+=2*ya;
	//CreateText(xa,y, 500,24, "txtoth3", TR("#80B0F0#{InputOther3}"));  y+=2*ya;
	y+=2*ya;
	CreateText(xa,y, 500,24, "txttir0", TR("#B0C0D0#{TiresEdit}"));  y+=2*ya;
	CreateText(xa1,y, 500,24, "txttir1", TR("#A0B0C0#{TiresEdit1}"));  y+=2*ya;
	CreateText(xa1,y, 500,24, "txttir2", TR("#A0B0C0#{TiresEdit2}"));  y+=2*ya;
	CreateText(xa1,y, 500,24, "txttir3", TR("#A0B0C0#{TiresEdit3}"));  y+=2*ya;
	CreateText(xa1,y, 500,24, "txttir4", TR("#A0B0C0#{TiresEdit4}"));  y+=3*ya;
	CreateText(xa,y, 500,24, "txttir5", TR("#90B0D0#{InputMapPrevTab}/#{InputMapNextTab} - #{InputGraphsType}"));  y+=3*ya;

	CreateText(xa,y, 500,24, "txttwk1", TR("#A0B8D0#{TweakEdit1}"));  y+=2*ya;
	CreateText(xa,y, 500,24, "txttwk2", TR("#A0B8D0#{TweakEdit2}"));  y+=3*ya;
	CreateText(xa,y, 500,24, "txttwk3", TR("#90B0C8#{TweakEdit3}"));  y+=2*ya;


	y = 32;
	tabitem = inputTab->addItem(TR("#B0A0E0#{Shortcuts}"));
	EditBox* ed = tabitem->createWidget<EditBox>("EditBoxEmpty", xa,y, 360,36, ALIGN, "txtshc0");
	ed->setCaption("#A0C0E0"+TR("#{ShortcutsInfo}"));  setOrigPos(ed, "OptionsWnd");  y+=5*yb;
	ed->setEditReadOnly(1);  ed->setEditMultiLine(1);  ed->setEditWordWrap(1);

	CreateText(xa,y, 200,24, "txtshc1", "#60FF60"+TR("Q  #{Track}"));  y+=2*yb;
	CreateText(xa,y, 200,24, "txtshc2", "#FF6050"+TR("C  #{Car}"));  y+=3*yb;
	CreateText(xa,y,  200,24, "txtshc3", "#90A0A0"+TR("T  #{Setup}"));  y+=2*yb;
	CreateText(xa,y,  200,24, "txtshc3", "#C0C080"+TR("W  #{Game}"));  y+=3*yb;
	CreateText(xa,y, 200,24, "txtshc5", "#FFC060"+TR("J  #{Tutorial}"));  y+=2*yb;
	CreateText(xa,y, 200,24, "txtshc5", "#80C0FF"+TR("H  #{Championship}"));  y+=2*yb;
	CreateText(xa,y, 200,24, "txtshc5", "#80FFCC"+TR("L  #{Challenge}"));  y+=3*yb;

	CreateText(xa,y, 200,24, "txtshc4", "#A0A0FF"+TR("U  #{Multiplayer}"));  y+=3*yb;
	CreateText(xa,y, 200,24, "txtshc6", "#FFA050"+TR("R  #{Replay}"));  y+=4*yb;
	CreateText(xa,y, 200,24, "txtshc7", "#60D060"+TR("#{InputFocusFind}"));  y+=2*yb;

	y = 32 + 5*yb;
	CreateText(xb,y, 200,24, "txtshd1", "#C0E0FF"+TR("S  #{Screen}"));  y+=2*yb;
	CreateText(xb1,y, 200,24, "txtshd2", "#E0C080"+TR("E  #{Effects}"));  y+=2*yb;
	CreateText(xb,y, 200,24, "txtshd3", "#B0B0FF"+TR("G  #{Graphics}"));  y+=2*yb;
	CreateText(xb1,y, 200,24, "txtshd4", "#90FF30"+TR("N  #{Vegetation}"));  y+=3*yb;
	
	CreateText(xb,y, 200,24, "txtshd5", "#D0FFFF"+TR("V  #{View}"));  y+=2*yb;
	CreateText(xb1,y, 200,24, "txtshd6", "#60F8F8"+TR("M  #{Minimap}"));  y+=2*yb;
	CreateText(xb1,y, 200,24, "txtshd7", "#C0A0E0"+TR("O  #{Other}"));  y+=3*yb;
	CreateText(xb,y, 200,24, "txtshd8", "#FFFF60"+TR("I  #{Input}"));  y+=3*yb;
	CreateText(xb,y, 200,24, "txtshd8", "#B090E0"+TR("P  #{Sound}"));  y+=3*yb;
*/
}


///  Bind Input
//----------------------------------------------------------------------------------------------------------------------------------

void App::inputBindBtnClicked(WP sender)
{
	static_cast<StaticTextPtr>(sender)->setCaption( TR("#{InputAssignKey}"));
	// activate key capture mode
	bAssignKey = true;
	pressedKeySender = sender;
	axisCnt = 0;
	hideMouse();
}

void App::InputBind(int key, int button, int axis)
{
	/*
	if (!bAssignKey)  return;
	bAssignKey = false;
	showMouse();

	//  cancel (unbind) on Backspace or Escape
	bool cancel = key == OIS::KC_ESCAPE;
	//  when esc quits? todo: add unbind X button
	
	//  upd key name on button
	bool isKey = key > -1, isAxis = axis > -1;
	String skey0 = isKey ? "Keyboard/" + toStr(key) : 
				isAxis ? joyName + "/Axis " + toStr(axis) :
						joyName + "/Button " + toStr(button);

	static_cast<StaticTextPtr>(pressedKeySender)->setCaption(
		cancel ? TR("#{InputKeyUnassigned}") : MyGUI::UString(GetInputName(skey0)));

	
	//  get action/schema/index from widget name
	Ogre::vector<String>::type ss = StringUtil::split(pressedKeySender->getName(), "_");
	std::string actionName = ss[1], schemaName = ss[2], index = ss[3];
	
	OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];  if (!schema)  return;//
	OISB::Action* action = schema->mActions[actionName];  if (!action)  return;//
	if (action->mBindings.empty())
		action->createBinding();
	OISB::Binding* binding = action->mBindings.front();  if (!binding)  return;//

	
	///  change AnalogAxis params  key/button <-> axis
	if (action->getActionType() == OISB::AT_ANALOG_AXIS)
	{
		bool full = action->getName() == "Steering" || action->getName() == "Flip";  //-1..1
		OISB::AnalogAxisAction* act = (OISB::AnalogAxisAction*)action;
		act->setProperty("AnalogEmulator", isAxis ? "" : "Linear");
		act->setUseAbsoluteValues(false);
		if (!full && isAxis)
		{	// half axis inversed
			act->setProperty("MinValue", -2);
			act->setProperty("MaxValue", 0);
			act->setProperty("InverseMul", -0.5);
		}else{
			act->setProperty("MinValue", full ? -1 : 0);
			act->setProperty("MaxValue", 1);
			act->setProperty("InverseMul", 1);
		}
		act->setProperty("Sensitivity", 1);		// restore defaults
		OISB::AnalogEmulator* emu = act->getAnalogEmulator();  if (emu)  {
			emu->setProperty("DecSpeed", 5);		emu->setProperty("IncSpeed", 5);
			emu->setProperty("ReturnEnabled", 1);	emu->setProperty("ReturnValue", 0);
			emu->setProperty("ReturnDecSpeed", 5);	emu->setProperty("ReturnIncSpeed", 5);  }
	}
	
	//  save keys
	String decKey = "", incKey = "";
	size_t num = binding->getNumBindables();
	for (int i = 0; i < num; ++i)
	{
		OISB::Bindable* bind = binding->getBindable(i);
		if (bind && bind != (void*)1)
		{
		String name = bind->getBindableName();
		String role = binding->getRole(bind);

		if (role == "dec")  decKey = name;
		if (role == "inc")  incKey = name;
	}	}

	//  clear, unbind
	for (int i = num-1; i >= 0; --i)
		binding->unbind(binding->getBindable(i));
	if (cancel)  return;  //-

	//  change
	String skey = cancel ? "" : skey0;
		 if (index == "1")  incKey = skey;  // lower btn - inc
	else if (index == "2")  decKey = skey;  // upper btn - dec

	//  update, bind  key/button
	if (!isAxis)
	{	if (incKey != "")	binding->bind(incKey, "inc");
		if (decKey != "")	binding->bind(decKey, "dec");
				
		//  update button labels  . . . . . . . 
		MyGUI::ButtonPtr b1 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "1", "", false);
		MyGUI::ButtonPtr b2 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "2", "", false);
		if (b1)  b1->setCaption(GetInputName(incKey));
		if (b2)  b2->setCaption(GetInputName(decKey));
	}
	else  // axis
		binding->bind(skey,"");
		*/
}


///  edit details
//-------------------------------------------------------------------------------
void App::inputDetailBtn(WP sender)
{
	/*
	Ogre::vector<String>::type ss = StringUtil::split(sender->getName(), "_");
	std::string actionName = ss[1], schemaName = ss[2], index = ss[3];
	if (txtInpDetail)  txtInpDetail->setCaption(TR("#{InputDetailsFor}")+":  "+schemaName+"  "+actionName);

	OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];  if (!schema)  return;//
	OISB::Action* action = schema->mActions[actionName];  if (!action)  return;//
	OISB::AnalogAxisAction* act = (OISB::AnalogAxisAction*)action;  if (!act)  return;//
	actDetail = act;
	if (panInputDetail)  panInputDetail->setVisible(false);

	if (edInputMin)  edInputMin->setCaption(toStr(act->getProperty<OISB::Real>("MinValue")));
	if (edInputMax)  edInputMax->setCaption(toStr(act->getProperty<OISB::Real>("MaxValue")));
	if (edInputMul)  edInputMul->setCaption(toStr(act->getProperty<OISB::Real>("InverseMul")));
	if (edInputReturn)    edInputReturn->setCaption(toStr(act->getProperty<OISB::Real>("ReturnIncSpeed")));        // =ReturnDecSpeed
	if (edInputIncrease)  edInputIncrease->setCaption(toStr(act->getProperty<OISB::Real>("IncSpeed")));  // =DecSpeed
	if (cmbInpDetSet)  cmbInpDetSet->setIndexSelected(0);
	*/
}

void App::editInput(MyGUI::EditPtr ed)
{
	/*
	if (!actDetail)  return;
	Real vMin = s2r(edInputMin->getCaption());
	Real vMax = s2r(edInputMax->getCaption());
	Real vMul = s2r(edInputMul->getCaption());
	actDetail->setProperty("MinValue",vMin);
	actDetail->setProperty("MaxValue",vMax);
	actDetail->setProperty("InverseMul",vMul);
	Real vRet = s2r(edInputReturn->getCaption());  // keyboard only
	Real vInc = s2r(edInputIncrease->getCaption());
	if (actDetail->getActionType() != OISB::AT_ANALOG_AXIS)
	{	// AnalogAxisAction doesn't have these properties
		actDetail->setProperty("ReturnDecSpeed",vRet);	actDetail->setProperty("DecSpeed",vInc);
		actDetail->setProperty("ReturnIncSpeed",vRet);	actDetail->setProperty("IncSpeed",vInc);
	}
	if (cmbInpDetSet)  cmbInpDetSet->setIndexSelected(0);
	*/
}

void App::comboInputPreset(MyGUI::ComboBoxPtr cmb, size_t val)
{
	/*
	if (!actDetail || val==0)  return;
	//key half, key full, axis half, axis half inversed, axis full, axis full inversed
	const Real aMin[6] = {0,-1, 0,  -2,  -1, -1};
	const Real aMax[6] = {1, 1, 2,   0,   1,  1};
	const Real aMul[6] = {1, 1, 0.5,-0.5, 1, -1};
	val = std::min((size_t)5, val-1);
	Real vMin = aMin[val];  if (edInputMin)  edInputMin->setCaption(toStr(vMin));
	Real vMax = aMax[val];  if (edInputMax)  edInputMax->setCaption(toStr(vMax));
	Real vMul = aMul[val];  if (edInputMul)  edInputMul->setCaption(toStr(vMul));
	actDetail->setProperty("MinValue",vMin);
	actDetail->setProperty("MaxValue",vMax);
	actDetail->setProperty("InverseMul",vMul);
	*/
}

void App::comboInputKeyAllPreset(MyGUI::ComboBoxPtr cmb, size_t val)
{
	/*
	if (val == 0)  return;  cmb->setIndexSelected(0);
	TabPtr tPlr = mGUI->findWidget<Tab>("InputTab",false);  if (!tPlr)  return;
	int id = tPlr->getIndexSelected();  if (id == 0)  return;
	String schemaName = "Player"+toStr(id);

	const int numActs = 6;  // these actions have key emul params (analog)
	const std::string keyActs[numActs] = {"Boost","Brake","Flip","HandBrake","Steering","Throttle"};
	const Real speeds[3] = {3,4,5},  // presets
			speedsRet[3] = {3,4,5};
	Real vInc = speeds[val-1], vRet = speedsRet[val-1];

	OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];  if (!schema)  return;
	for (int i=0; i < numActs; ++i)
	{
		OISB::Action* action = schema->mActions[keyActs[i]];  if (!action)  continue;
		OISB::AnalogAxisAction* act = (OISB::AnalogAxisAction*)action;  if (!act)  continue;

		act->setProperty("ReturnDecSpeed",vRet);	act->setProperty("DecSpeed",vInc);
		act->setProperty("ReturnIncSpeed",vRet);	act->setProperty("IncSpeed",vInc);
	}
	if (!actDetail)  return;  // update edit vals
	if (edInputReturn)      edInputReturn->setCaption(toStr(actDetail->getProperty<OISB::Real>("IncSpeed")));
	if (edInputIncrease)  edInputIncrease->setCaption(toStr(actDetail->getProperty<OISB::Real>("ReturnIncSpeed")));
	*/
}


///  update input bars vis,dbg
//-------------------------------------------------------------------------------
void App::UpdateInputBars()
{
	/*
	MyGUI::TabPtr inputTab = mGUI->findWidget<Tab>("InputTab");
	if (!inputTab)  return;

	OISB::System& sys = OISB::System::getSingleton();  int sch = 0;
	std::map<OISB::String, OISB::ActionSchema*> schemas = sys.mActionSchemas;
	for (std::map<OISB::String, OISB::ActionSchema*>::const_iterator it = schemas.begin(); it != schemas.end(); ++it,++sch)
	{
		const OISB::String& sPlr = (*it).first;
		if (inputTab->getIndexSelected() != sch)  continue;

		//*  one axis info
		bool oneAxis = false;  // when brake not bound
		OISB::Real valBr = 0.f;  // brake val from throttle
		if (sch > 0)  {
			OISB::AnalogAxisAction* act = static_cast<OISB::AnalogAxisAction*>(
				OISB::System::getSingleton().lookupAction("Player" + toStr(sch) + "/Brake"));
			if (act)  {
				OISB::Binding* binding = act->mBindings.front();
				if (binding)
					oneAxis = binding->getNumBindables() == 0;  }
			if (oneAxis)
			{	OISB::AnalogAxisAction* act = static_cast<OISB::AnalogAxisAction*>(
					OISB::System::getSingleton().lookupAction("Player" + toStr(sch) + "/Throttle"));
				if (act)
				{	valBr = act->getAbsoluteValue();
					valBr = valBr < 0.f ? -valBr : 0.f;  }
		}  }//*
		
		//  Action
		for (std::map<OISB::String, OISB::Action*>::const_iterator
			ait = (*it).second->mActions.begin();
			ait != (*it).second->mActions.end(); ++ait)
		{
			const OISB::String& sAct = (*ait).first;
			OISB::Action* act = (*ait).second;
			float val = -1.f;
			bool full = act->getName() == "Steering" || act->getName() == "Flip";
			//  get val
			if (act->getActionType() == OISB::AT_ANALOG_AXIS)
			{
				OISB::AnalogAxisAction* ac = static_cast<OISB::AnalogAxisAction*>(act);
				if (ac)  val = ac->getAbsoluteValue();
			}else
				val = act->isActive() ? 1.f : 0.f;
				
			if (oneAxis && act->getName() == "Throttle")  if (val < 0.f)  val = 0.f;
			if (oneAxis && act->getName() == "Brake")  val = valBr;
				
			std::string sBar = "bar_" + sAct + "_" + sPlr;
			StaticImagePtr bar = mGUI->findWidget<StaticImage>(sBar, false);
			if (bar)
			{	const int wf = 128, w = 256;  int v = -val * 128, vf = -val * 64, s=512, s0=s/2;
				if (full)	bar->setImageCoord(IntCoord(std::max(0, std::min(s-wf, vf + s0 -wf/2)), 0, wf, 16));
				else		bar->setImageCoord(IntCoord(std::max(0, std::min(s-w, v + s0)), 0, w, 16));
			}
		}
	}
	*/
}


///  joysticks events
//
/*
bool App::axisMoved( const OIS::JoyStickEvent &e, int axis )
{
	if (txtJAxis)
	{	int iv = e.state.mAxes[axis].abs;
		float val = iv >= 0 ? iv / 32767.f : iv / 32768.f;
		txtJAxis->setCaption("Moved axis: "+toStr(axis)+"     val: "+fToStr(val,4,7));
	}
	if (abs(e.state.mAxes[axis].abs) > OIS::JoyStick::MAX_AXIS / 2)
		InputBind(-1, -1, axis);

	return true;
}

bool App::buttonPressed( const OIS::JoyStickEvent &e, int button )
{
	if (txtJBtn)  txtJBtn->setCaption("Pressed button: " + toStr(button));

	InputBind(-1, button);

	return true;
}
bool App::buttonReleased( const OIS::JoyStickEvent &e, int button )
{
	return true;
}
*/

void App::cmbJoystick(CMB)
{
	joyName = wp->getItemNameAt(val);
}
