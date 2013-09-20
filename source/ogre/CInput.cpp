#include "pch.h"
//#include "common/Def_Str.h"
#include "CGame.h"
//#include "CHud.h"
#include "CInput.h"
#include "../oics/ICSInputControlSystem.h"


//  ctor  -----------------------------------------------
CInput::CInput(App* app1)
	:app(app1)
{
	for (int p=0;p<4;++p)
	{
		for (int a=0;a<NumPlayerActions;++a)
			mPlayerInputState[p][a] = 0;
	}
}


///  input events
//----------------------------------------------------------------------------------------------------------------------------------
void CInput::LoadInputDefaults()
{
	mInputActions.clear();
	mInputActions.push_back(InputAction(A_ShowOptions, "ShowOptions", SDLK_TAB, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_PrevTab, "PrevTab", SDLK_F2, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_NextTab, "NextTab", SDLK_F3, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_RestartGame, "RestartGame", SDLK_F5, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_ResetGame, "ResetGame", SDLK_F4, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_Screenshot, "Screenshot", SDLK_F12, InputAction::Trigger));

	LoadInputDefaults(mInputActions, app->mInputCtrl);

	std::vector<InputAction>* a = mInputActionsPlayer;
	a[0].clear();
	a[0].push_back(InputAction(A_Throttle, "Throttle", SDLK_UP, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Brake, "Brake", SDLK_DOWN, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Steering, "Steering", SDLK_LEFT, SDLK_RIGHT, InputAction::Axis));
	a[0].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_SPACE, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Boost, "Boost", SDLK_LCTRL, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Flip, "Flip", SDLK_q, SDLK_w, InputAction::Axis));
	a[0].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_a, InputAction::Trigger));
	a[0].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_z, InputAction::Trigger));
	a[0].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_x, InputAction::Trigger));
	a[0].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_c, InputAction::Trigger));
	a[0].push_back(InputAction(A_LastChk, "LastChk", SDLK_0, InputAction::Trigger));
	a[0].push_back(InputAction(A_Rewind, "Rewind", SDLK_BACKSPACE, InputAction::Trigger));

	a[1].clear();
	a[1].push_back(InputAction(A_Throttle, "Throttle", SDLK_u, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Brake, "Brake", SDLK_m, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Steering, "Steering", SDLK_h, SDLK_k, InputAction::Axis));
	a[1].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_n, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Boost, "Boost", SDLK_j, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Flip, "Flip", SDLK_y, SDLK_i, InputAction::Axis));
	a[1].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	a[2].clear();
	a[2].push_back(InputAction(A_Throttle, "Throttle", SDLK_r, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Brake, "Brake", SDLK_v, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Steering, "Steering", SDLK_d, SDLK_g, InputAction::Axis));
	a[2].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_b, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Boost, "Boost", SDLK_f, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Flip, "Flip", SDLK_e, SDLK_t, InputAction::Axis));
	a[2].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	a[3].clear();
	a[3].push_back(InputAction(A_Throttle, "Throttle", SDLK_p, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Brake, "Brake", SDLK_SLASH, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Steering, "Steering", SDLK_l, SDLK_QUOTE, InputAction::Axis));
	a[3].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_PERIOD, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Boost, "Boost", SDLK_SEMICOLON, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Flip, "Flip", SDLK_o, SDLK_LEFTBRACKET, InputAction::Axis));
	a[3].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	for (int i=0; i<4; ++i)
		LoadInputDefaults(a[i], app->mInputCtrlPlayer[i]);
}

void CInput::LoadInputDefaults(std::vector<InputAction> &actions, ICS::InputControlSystem *pICS)
{
	for (std::vector<InputAction>::iterator it = actions.begin(); it != actions.end(); ++it)
	{
		ICS::Control* control;
		bool controlExists = (pICS->getChannel(it->mId)->getControlsCount() != 0);
		if (!controlExists)
		{
			if (it->mType == InputAction::Trigger)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0, ICS::ICS_MAX, ICS::ICS_MAX, false);
			else if (it->mType == InputAction::Axis)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0.5, 0.1, 30.0);
			else if (it->mType == InputAction::HalfAxis)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0.0, 0.1, 30.0);

			pICS->addControl(control);

			if (it->mKeyInc != SDLK_UNKNOWN)
				pICS->addKeyBinding(control, it->mKeyInc, ICS::Control::INCREASE);
			if (it->mKeyDec != SDLK_UNKNOWN)
				pICS->addKeyBinding(control, it->mKeyDec, ICS::Control::DECREASE);

			control->attachChannel(pICS->getChannel(it->mId), ICS::Channel::DIRECT);
			pICS->getChannel(it->mId)->update();
		}
		else
			control = pICS->getChannel(it->mId)->getAttachedControls().front().control;

		it->mICS = pICS;
		it->mControl = control;

		if (pICS == app->mInputCtrl)
			pICS->getChannel(it->mId)->addListener(app);
	}
}
