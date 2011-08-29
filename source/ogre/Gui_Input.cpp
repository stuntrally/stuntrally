#include "pch.h"
#include "Defines.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include <OIS/OIS.h>
#include "../oisb/OISB.h"

#include <OgreRoot.h>
using namespace MyGUI;
using namespace Ogre;


///  Gui Init - input tab
//----------------------------------------------------------------------------------------------------------------------------------
void App::InitInputGui()
{
	//  Log all devices (and keys) to log.txt
	pGame->info_output << " --------------------------------------  Input devices  BEGIN" << std::endl;
	//OISB::System::getSingleton().dumpActionSchemas(pGame->info_output);
	OISB::System::getSingleton().dumpDevices(pGame->info_output);
	pGame->info_output << " --------------------------------------  Input devices  END" << std::endl;

	MyGUI::TabPtr inputTab = mGUI->findWidget<Tab>("InputTab");
	if (!inputTab)  return;

	//  insert a tab item for every schema (4players,global)
	std::map<OISB::String, OISB::ActionSchema*> schemas = OISB::System::getSingleton().mActionSchemas;
	for (std::map<OISB::String, OISB::ActionSchema*>::const_iterator it = schemas.begin(); it != schemas.end(); it++)
	{
		MyGUI::TabItemPtr tabitem = inputTab->addItem( TR("#{InputMap" + (*it).first + "}") );

		#define CreateText(x,y, w,h, name, text)  {  MyGUI::StaticTextPtr txt =  \
			tabitem->createWidget<StaticText>("StaticText", x,y, w,h, MyGUI::Align::Relative, name);  \
			if (txt)  txt->setCaption(text);  }
		
		///  Headers (Key 1, Key 2)
		CreateText(220,10, 200,24, "staticText_" + (*it).first, TR("#88AAFF#{InputKey1}"));
		CreateText(360,10, 200,24, "staticText_" + (*it).first, TR("#88AAFF#{InputKey2}"));
				
		///  joystick selection menu
		//  only on player tabs
		bool playerTab = Ogre::StringUtil::startsWith( (*it).first, "player");
		if (playerTab)
		{
			MyGUI::ComboBoxPtr joysticks = tabitem->createWidget<ComboBox>("ComboBox",
				540,10, 150,24, MyGUI::Align::Relative,
				"joystickSel_" + (*it).first );
			joysticks->addItem(TR("#{InputNoJS}"));
			joysticks->setIndexSelected(0);
			for (std::vector<OISB::JoyStick*>::const_iterator jit=OISB::System::getSingleton().mJoysticks.begin();
				jit!=OISB::System::getSingleton().mJoysticks.end();	jit++)
			{
				joysticks->addItem( (*jit)->getName() );
			}
			joysticks->addItem("Dummy Joystick");	// test
			joysticks->setEditReadOnly(true);
			joysticks->eventComboChangePosition = MyGUI::newDelegate(this, &App::joystickSelectionChanged);
		
			//  add labels that print the last pressed joystick button / last moved axis
			CreateText(300,350, 300,24, "axisOutput_"   + (*it).first, TR("#{InputMoveAxisTip}"));
			CreateText(300,380, 300,24, "buttonOutput_" + (*it).first, TR("#{InputPressButtonTip}"));
		}
		
		///  ------ custom action sorting ----------------
		int i = 0, y = 0, ya = 26 / 2, yc1=0,yc2=0,yc3=0;
		std::map <std::string, int> yRow;
		// player
		yRow["Throttle"] = y;	y+=2;	yRow["Brake"] = y;		y+=2;
		yRow["Steering"] = y;	y+=2+1 +2;
		yRow["HandBrake"] = y;	y+=2;	yRow["Boost"] = y;		y+=2;
		yRow["Flip"] = y;
		yRow["FlipRight"] = y;	y+=2;	yRow["FlipLeft"] = y;	y+=2 +1;
		yRow["ShiftUp"] = y;	y+=2;	yRow["ShiftDown"] = y;	y+=2;
		// general
		y = 0;
		yRow["ShowOptions"] = y; y+=2+1;
		yRow["PrevTab"] = y;     y+=2;	yRow["NextTab"] = y;    y+=2+1;
		yRow["RestartGame"] = y; y+=2+1;  yc1 = 40 + ya * y;
		yRow["PrevCamera"] = y;  y+=2;    yc2 = 40 + ya * y;
		yRow["NextCamera"] = y;  y+=2+1;  yc3 = 40 + ya * y;

		if (!playerTab)
		{	//  camera infos
			CreateText(460, yc1, 280, 24, "txtcam1", TR("#C0D8F0#{InputCameraTxt1}"));
			CreateText(460, yc2, 280, 24, "txtcam1", TR("#C0D8F0#{InputCameraTxt2}"));
			//  replay controls info text
			CreateText(20, yc3+1*ya, 500, 24, "txtrpl1", TR("#A0D8FF#{InputRplCtrl0}"));
			CreateText(40, yc3+3*ya, 500, 24, "txtrpl2", TR("#90C0FF#{InputRplCtrl1}"));
			CreateText(40, yc3+5*ya, 500, 24, "txtrpl3", TR("#90C0FF#{InputRplCtrl2}"));
			CreateText(40, yc3+7*ya, 500, 24, "txtrpl4", TR("#90C0FF#{InputRplCtrl3}"));
		}
		
		///  Actions  ------------------------------------------------
		for (std::map<OISB::String, OISB::Action*>::const_iterator
			ait = (*it).second->mActions.begin();
			ait != (*it).second->mActions.end(); ait++,i++)
		{
			OISB::Action* act = (*ait).second;
			if (act->isAnalog() == false && act->getName() == "Flip")
				continue;

			//  button size and columns positon
			const int sx = 130, sy = 24, x0 = 20, x1 = 180, x2 = 320, x3 = 540;
			const String& name = (*ait).second->getName();
			y = 40 + ya * yRow[name];

			//  description label
			MyGUI::StaticTextPtr desc = tabitem->createWidget<StaticText>("StaticText",
				x0, y, sx+70, sy,  MyGUI::Align::Relative,
				"staticText_" + (*ait).first );
			desc->setCaption( TR("#{InputMap" + name + "}") );
		
			///  Keyboard binds  --------------------------------
			//  get information about binds from OISB and set variables how the rebind buttons should be created
			std::string skey1 = TR("#{InputKeyUnassigned}");
			std::string skey2 = TR("#{InputKeyUnassigned}");
			
			//  bound key(s)
			if (act->mBindings.size() > 0 && act->mBindings.front()->getNumBindables() > 0 && act->mBindings.front()->getBindable(0) && act->mBindings.front()->getBindable(0) != (OISB::Bindable*)1)
			if (act->getActionType() == OISB::AT_TRIGGER)
			{
				skey1 = act->mBindings.front()->getBindable(0)->getBindableName();
			}
			else if (act->getActionType() == OISB::AT_ANALOG_AXIS)
			{
				//  look for increase/decrease binds
				OISB::Bindable* increase = NULL, *decrease = NULL;
				for (std::vector<std::pair<String, OISB::Bindable*> >::const_iterator
					bnit = act->mBindings.front()->mBindables.begin();
					bnit != act->mBindings.front()->mBindables.end(); bnit++)
				{
					if ((*bnit).first == "inc")			increase = (*bnit).second;
					else if ((*bnit).first == "dec")	decrease = (*bnit).second;
				}
				if (increase)  skey1 = increase->getBindableName();
				if (decrease)  skey2 = decrease->getBindableName();
			}
				
			//  create buttons  ----------------
			bool button2 = false;
			if (act->getActionType() == OISB::AT_ANALOG_AXIS && !( act->getProperty<int> ("MinValue") == 0 ))
				button2 = true;

			MyGUI::ButtonPtr btn1 = tabitem->createWidget<Button>("Button", /*button2 ? x2 :*/
				x1, button2 ? (y + ya*2) : y, sx, sy,  MyGUI::Align::Relative,
				"inputbutton_" + (*ait).first + "_" + (*it).first + "_1");
			btn1->setCaption( StrFromKey(skey1) );
			btn1->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
			
			if (button2)
			{	MyGUI::ButtonPtr btn2 = tabitem->createWidget<Button>("Button",
					x1, y, sx, sy,  MyGUI::Align::Relative,
					"inputbutton_" + (*ait).first + "_" + (*it).first + "_2");
				btn2->setCaption( StrFromKey(skey2) );
				btn2->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
			}

			///  Joystick binds  --------------------------------
			//  only on player tab
			if (playerTab)
			{
				if (act->getActionType() == OISB::AT_TRIGGER)
				{
					MyGUI::ComboBoxPtr button = tabitem->createWidget<ComboBox>("ComboBox",
						x3, y, sx, sy,  MyGUI::Align::Relative,
						"jsButtonSel_" + (*ait).first + "_" + (*it).first );
					button->addItem(TR("#{InputKeyNoButton}"));
					button->setIndexSelected(0);  button->setEditReadOnly(true);
					button->eventComboChangePosition = MyGUI::newDelegate(this, &App::joystickBindChanged);
				}
				else if (act->getActionType() == OISB::AT_ANALOG_AXIS)
				{
					MyGUI::ComboBoxPtr axis = tabitem->createWidget<ComboBox>("ComboBox",
						x3, y, sx, sy,  MyGUI::Align::Relative,
						"jsAxisSel_" + (*ait).first + "_" + (*it).first );
					axis->addItem(TR("#{InputKeyNoAxis}"));
					axis->setIndexSelected(0);  axis->setEditReadOnly(true);
					axis->eventComboChangePosition = MyGUI::newDelegate(this, &App::joystickBindChanged);
				}
			}
		}
	}
	UpdateJsButtons(); // initial
}


void App::UpdateJsButtons()
{
	//  go through all action schemas & actions, and fill the combo boxes for JS axis / buttons
	std::map<OISB::String, OISB::ActionSchema*> schemas = OISB::System::getSingleton().mActionSchemas;
	for (std::map<OISB::String, OISB::ActionSchema*>::const_iterator it = schemas.begin(); it != schemas.end(); it++)
	{
		if (!Ogre::StringUtil::startsWith( (*it).first, "player"))
			continue;  // joystick only on player tabs
		
		for (std::map<OISB::String, OISB::Action*>::const_iterator
			ait = (*it).second->mActions.begin();
			ait != (*it).second->mActions.end(); ait++)
		{
			OISB::Action* act = (*ait).second;
			
			if (act->getName() == "Flip" && act->isAnalog() == false)
				continue;
			
			OISB::Binding* bnd2 = NULL;
			if (act->mBindings.size() >= 2) bnd2 = act->mBindings[1];
			
			//  find selected oisb joystick for this tab (to get num axis & buttons)
			MyGUI::ComboBoxPtr jsMenu = mGUI->findWidget<ComboBox>("joystickSel_" + (*it).first);
			std::string jsName;
			if (jsMenu->getIndexSelected() != MyGUI::ITEM_NONE)
				jsName = jsMenu->getItemNameAt( jsMenu->getIndexSelected() );
			
			OISB::JoyStick* js = NULL;
			for (std::vector<OISB::JoyStick*>::const_iterator jit = mOISBsys->mJoysticks.begin();
					jit != mOISBsys->mJoysticks.end(); jit++)
				if ( (*jit)->getName() == jsName ) js = (*jit);

			//  fill combo boxes
			if (act->getActionType() == OISB::AT_TRIGGER)
			{
				MyGUI::ComboBoxPtr button = mGUI->findWidget<ComboBox>("jsButtonSel_" + (*ait).first + "_" + (*it).first);
				button->removeAllItems();
				button->addItem( TR("#{InputKeyNoButton}") );
				if (js)
				{	for (std::vector<OISB::DigitalState*>::const_iterator it = js->buttons.begin();
							it != js->buttons.end(); it++)
						button->addItem( StrFromKey((*it)->getBindableName()) );
				}					
				button->setIndexSelected(0);
					
				//  select correct axis/button (from user keybinds)
				if (bnd2 && bnd2->mBindables.size() > 0) {
					size_t result;
					if (bnd2->getBindable(0) == NULL)
					{
						result = button->findItemIndexWith( StrFromKey(bnd2->getRole(NULL)) );
						if (result != MyGUI::ITEM_NONE)
							button->setIndexSelected( result );
					}else{
						result = button->findItemIndexWith( StrFromKey(bnd2->getBindable(0)->getBindableName()) );
						if (result != MyGUI::ITEM_NONE)
							button->setIndexSelected( result );
					}
				}
			}
			else if (act->getActionType() == OISB::AT_ANALOG_AXIS)
			{
				MyGUI::ComboBoxPtr axis = mGUI->findWidget<ComboBox>("jsAxisSel_" + (*ait).first + "_" + (*it).first);
				axis->removeAllItems();
				axis->addItem( TR("#{InputKeyNoAxis}") );
				if (js)
				{	for (std::vector<OISB::AnalogAxisState*>::const_iterator it = js->axis.begin();
							it != js->axis.end(); it++)
						axis->addItem( StrFromKey((*it)->getBindableName()) );
				}					
				axis->setIndexSelected(0);
				
				//  select correct axis/button (from user keybinds)
				if (bnd2 && bnd2->mBindables.size() > 0)
				{	size_t result;
					if (bnd2->getBindable(0) == NULL)
					{
						result = axis->findItemIndexWith( StrFromKey(bnd2->getRole(NULL)) );
						if (result != MyGUI::ITEM_NONE)
							axis->setIndexSelected( result );
					}else{
						result = axis->findItemIndexWith( StrFromKey(bnd2->getBindable(0)->getBindableName()) );
						if (result != MyGUI::ITEM_NONE)
							axis->setIndexSelected( result );
				}	}
			}
	}	}
}


//  Events
//-----------------------------------------------------------------------------------------------------------------

void App::controlBtnClicked(Widget* sender)
{
	sender->setCaption( TR("#{InputAssignKey}"));
	// activate key capture mode
	bAssignKey = true;
	pressedKeySender = sender;
	// hide mouse
	MyGUI::PointerManager::getInstance().setVisible(false);
}
void App::joystickBindChanged(Widget* sender, size_t val)
{
	// get action/schema this bind belongs too
	std::string actionName = Ogre::StringUtil::split(sender->getName(), "_")[1];
	std::string schemaName = Ogre::StringUtil::split(sender->getName(), "_")[2];
	
	LogO(actionName);
	LogO(schemaName);
	
	OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];
	OISB::Action* action = schema->mActions[actionName];
	if (action->mBindings.size() == 0) return;
	if (action->mBindings.size() == 1) action->createBinding();
	OISB::Binding* binding = action->mBindings[1];
	binding->mOptional = true;
	
	// get selected joystick
	// find selected oisb joystick for this tab (to get num axis & buttons)
	MyGUI::ComboBoxPtr jsMenu = mGUI->findWidget<ComboBox>("joystickSel_" + schemaName);
	std::string jsName;
	if (jsMenu->getIndexSelected() != MyGUI::ITEM_NONE)
		jsName = jsMenu->getItemNameAt( jsMenu->getIndexSelected() );
	else 
	{
		LogO("Couldnt get selected joystick"); return;
	}
	LogO(jsName);
		
	// get selected axis or button
	MyGUI::ComboBoxPtr box = static_cast<MyGUI::ComboBoxPtr> (sender);
	if (box->getItemCount() < box->getIndexSelected() || box->getIndexSelected() == MyGUI::ITEM_NONE)
	{
		LogO("Invalid item value"); return;
	}
	std::string bindName = box->getItemNameAt(box->getIndexSelected());
	LogO(bindName);
	
	// unbind old
	for (int i=0; i<binding->getNumBindables(); i++)
	{
		binding->unbind(binding->getBindable(i));
	}
	
	// bind new
	try
	{	binding->bind(jsName + "/" + bindName);  }
	catch (OIS::Exception) {
		LogO("Failed to bind '" + jsName + "/" + bindName + "'");	}
}

void App::joystickSelectionChanged(Widget* sender, size_t val)
{
	UpdateJsButtons();
	
	// ----------------  update all binds with the new joystick  -----------------------------------------
	std::string actionSchemaName = Ogre::StringUtil::split(sender->getName(), "_")[1];
	
	OISB::ActionSchema* schema = mOISBsys->mActionSchemas[actionSchemaName];
		
	for (std::map<OISB::String, OISB::Action*>::const_iterator
		ait = schema->mActions.begin();
		ait != schema->mActions.end(); ait++)
	{
		MyGUI::WidgetPtr box;
		if ((*ait).second->getActionType() == OISB::AT_TRIGGER)
			box = mGUI->findWidget<Widget>("jsButtonSel_" + (*ait).first + "_" + actionSchemaName);
		else if ((*ait).second->getActionType() == OISB::AT_ANALOG_AXIS)
			box = mGUI->findWidget<Widget>("jsAxisSel_" + (*ait).first + "_" + actionSchemaName);
			
		joystickBindChanged(box, 0);
	}
}
