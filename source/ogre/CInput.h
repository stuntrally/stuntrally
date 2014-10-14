#pragma once
#include <string>
//#include "SDL_keycode.h"


// These IDs are referenced in the user config files.
// To keep them valid, make sure to:
// - Add new actions at the end of the enum
// - Instead of deleting an action, replace it with a dummy one eg A_Unused

enum Actions
{	A_ShowOptions, A_PrevTab, A_NextTab, A_RestartGame, A_ResetGame, A_Screenshot, NumActions  };

const static std::string csActions[NumActions] =
{	"ShowOptions", "PrevTab", "NextTab", "RestartGame", "ResetGame", "Screenshot"  };


enum PlayerActions
{	A_Throttle, A_Brake, A_Steering, A_HandBrake, A_Boost, A_Flip,
	A_ShiftUp, A_ShiftDown, A_PrevCamera, A_NextCamera, A_LastChk, A_Rewind, NumPlayerActions
};

const static std::string csPlayerActions[NumPlayerActions] =
{	"Throttle", "Brake", "Steering", "HandBrake", "Boost", "Flip",
	"ShiftUp", "ShiftDown", "PrevCamera", "NextCamera", "LastChk", "Rewind"
};

namespace ICS
{	class InputControlSystem;  class Control;  }


//  Input
//-----------------------------------------------------------------
struct InputAction
{
	int mId;
	std::string mName;
	SDL_Keycode mKeyInc, mKeyDec;

	enum Type
	{	Trigger = 0x00,
		Axis = 0x01,     // 2-sided axis, centered in the middle, keyboard emulation with left & right keys
		HalfAxis = 0x11  // 1-sided axis, keyboard emulation with 1 key
	} mType;

	ICS::InputControlSystem* mICS;
	ICS::Control* mControl;

	InputAction(int id, bool player, SDL_Keycode incKey, Type type = Trigger)
		: mId(id), mKeyInc(incKey), mKeyDec(SDLK_UNKNOWN), mType(type)
		, mICS(0), mControl(0)
	{
		mName = player ? csPlayerActions[id] : csActions[id];
	}
	InputAction(int id, bool player, SDL_Keycode decKey, SDL_Keycode incKey)
		: mId(id), mKeyInc(incKey), mKeyDec(decKey), mType(Axis)
		, mICS(0), mControl(0)
	{
		mName = player ? csPlayerActions[id] : csActions[id];
	}
};


class CInput
{
public:
	class App* app;
	CInput(App* app1);

	//  Input
	float mPlayerInputState[4][NumPlayerActions];
	boost::mutex mPlayerInputStateMutex;
	
	std::vector<InputAction> mInputActions;
	std::vector<InputAction> mInputActionsPlayer[4];

	///  Add
	//  Global trigger
	void AddGlob(Actions a, SDL_Keycode key)
	{
		mInputActions.push_back(InputAction(a,false,key));
	}
	//  player trigger
	void AddTrig(int plr, PlayerActions a, SDL_Keycode key)
	{
		mInputActionsPlayer[plr].push_back(InputAction(a,true, key));
	}
	//  player Axis
	void AddAxis(int plr, PlayerActions a, SDL_Keycode incKey, SDL_Keycode decKey)
	{
		mInputActionsPlayer[plr].push_back(InputAction(a,true, incKey,decKey));
	}
	//  player Half axis
	void AddHalf(int plr, PlayerActions a, SDL_Keycode key)
	{
		mInputActionsPlayer[plr].push_back(InputAction(a,true, key, InputAction::HalfAxis));
	}

	void LoadInputDefaults();
	void LoadInputDefaults(std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
};
