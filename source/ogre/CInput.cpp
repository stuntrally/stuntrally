#include "pch.h"
//#include "common/Def_Str.h"
#include "CGame.h"
#include "CInput.h"
#include "../oics/ICSInputControlSystem.h"
using namespace std;


//  ctor  -----------------------------------------------
CInput::CInput(App* app1)
	:app(app1)
{
	int p,a;
	for (p=0; p < 4; ++p)
	for (a=0; a < NumPlayerActions; ++a)
		mPlayerInputState[p][a] = 0;
}


///  input events
//----------------------------------------------------------------------------------------------------------------------------------
void CInput::LoadInputDefaults()
{
	//  clear
	mInputActions.clear();
	int i,p,n;
	for (i=0; i < 4; ++i)
		mInputActionsPlayer[i].clear();


	//  default keys
	const SDL_Keycode keyG[NumActions] =
	// ShowOptions,  PrevTab, NextTab,  RestartGame, ResetGame,  Screenshot	
		{ SDLK_TAB,  SDLK_F2, SDLK_F3,  SDLK_F5, SDLK_F4,  SDLK_F12 };

	for (i=0; i < NumActions; ++i)
		AddGlob(Actions(i), keyG[i]);


	//  players keys, [4] sets
	const int iHalf = 4, iAxis = 2, iTrig = 6;
	const PlayerActions
		aHalf[iHalf] = {A_Throttle, A_Brake,  A_HandBrake, A_Boost},
		aAxis[iAxis] = {A_Steering, A_Flip},
		aTrig[iTrig] = {A_ShiftUp, A_ShiftDown,  A_PrevCamera, A_NextCamera,  A_LastChk, A_Rewind};
	const SDL_Keycode
		kHalf[4][iHalf]	= {{SDLK_UP, SDLK_DOWN,  SDLK_SPACE, SDLK_LCTRL},
						   {SDLK_u, SDLK_m,  SDLK_n, SDLK_j},
						   {SDLK_r, SDLK_v,  SDLK_b, SDLK_f},
						   {SDLK_p, SDLK_SLASH, SDLK_PERIOD, SDLK_SEMICOLON}},

		kAxis[4][iAxis][2] = { {{SDLK_LEFT, SDLK_RIGHT}, {SDLK_q, SDLK_w}},
							   {{SDLK_h, SDLK_k}, {SDLK_y, SDLK_i}},
							   {{SDLK_d, SDLK_g}, {SDLK_e, SDLK_t}},
							   {{SDLK_l, SDLK_QUOTE}, {SDLK_o, SDLK_LEFTBRACKET}} },

		kTrig0[iTrig] = {SDLK_a, SDLK_s,  SDLK_x, SDLK_c,  SDLK_0, SDLK_BACKSPACE};  // 1st player, rest undefined

	//  map to add in order like in enum
	const int iBoth = iHalf+iAxis;
	bool bH[iBoth] = {1,1,0,1,1,0};  // 1 Half, 0 Axis
	int id[iBoth]  = {0,1,0,2,3,1};  // id for both

	//  add
	for (p=0; p < 4; ++p)
	{
		for (n=0; n < iBoth; ++n)
		{	i = id[n];
			if (bH[n])  AddHalf(p, aHalf[i], kHalf[p][i]);
			else        AddAxis(p, aAxis[i], kAxis[p][i][0], kAxis[p][i][1]);
		}
		for (i=0; i < iTrig; ++i)
			AddTrig(p, aTrig[i], p == 0 ? kTrig0[i] : SDLK_UNKNOWN);
	}

	//  Load
	LoadInputDefaults(mInputActions, app->mInputCtrl);
	for (i=0; i<4; ++i)
		LoadInputDefaults(mInputActionsPlayer[i], app->mInputCtrlPlayer[i]);
}


void CInput::LoadInputDefaults(vector<InputAction> &actions, ICS::InputControlSystem *pICS)
{
	for (auto it = actions.begin(); it != actions.end(); ++it)
	{
		ICS::Control* control;
		bool controlExists = (pICS->getChannel(it->mId)->getControlsCount() != 0);
		if (!controlExists)
		{
			string sId = boost::lexical_cast<string>(it->mId);

			if (it->mType == InputAction::Trigger)
				control = new ICS::Control(sId, false, true, 0, ICS::ICS_MAX, ICS::ICS_MAX, false);
			else if (it->mType == InputAction::Axis)
				control = new ICS::Control(sId, false, true, 0.5, 0.1, 30.0);
			else if (it->mType == InputAction::HalfAxis)
				control = new ICS::Control(sId, false, true, 0.0, 0.1, 30.0);

			pICS->addControl(control);

			if (it->mKeyInc != SDLK_UNKNOWN)  pICS->addKeyBinding(control, it->mKeyInc, ICS::Control::INCREASE);
			if (it->mKeyDec != SDLK_UNKNOWN)  pICS->addKeyBinding(control, it->mKeyDec, ICS::Control::DECREASE);

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
