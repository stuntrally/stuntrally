#pragma once

// These IDs are referenced in the user config files.
// To keep them valid, make sure to:
// - Add new actions at the end of the enum
// - Instead of deleting an action, replace it with a dummy one eg A_Unused

enum Actions
{	A_ShowOptions, A_PrevTab, A_NextTab, A_RestartGame, A_ResetGame, A_Screenshot, NumActions	};

enum PlayerActions
{	A_Throttle, A_Brake, A_Steering, A_HandBrake, A_Boost, A_Flip,
	A_ShiftUp, A_ShiftDown, // TODO: Shift up/down could be a single "shift" action
	A_PrevCamera, A_NextCamera, A_LastChk, A_Rewind, NumPlayerActions
};

namespace ICS
{	class InputControlSystem;  class Control;  }

//  Input
//-----------------------------------------------------------------
struct InputAction
{
	std::string mName;  int mId;
	SDL_Keycode mKeyInc, mKeyDec;

	enum Type
	{	Trigger = 0x00,
		Axis = 0x01,     // 2-sided axis, centered in the middle, keyboard emulation with left & right keys
		HalfAxis = 0x11  // 1-sided axis, keyboard emulation with 1 key
	} mType;

	ICS::InputControlSystem* mICS;
	ICS::Control* mControl;

	InputAction(int id, const std::string& name, SDL_Keycode incKey, Type type)
		: mId(id), mName(name), mKeyInc(incKey), mKeyDec(SDLK_UNKNOWN), mType(type)
	{	}
	InputAction(int id, const std::string &name, SDL_Keycode decKey, SDL_Keycode incKey, Type type)
		: mId(id), mName(name), mKeyInc(incKey), mKeyDec(decKey), mType(Axis)
	{	}
};

class App;


class CInput
{
public:
	App* app;
	CInput(App* app1);

	//  Input
	float mPlayerInputState[4][NumPlayerActions];
	boost::mutex mPlayerInputStateMutex;

	std::vector<InputAction> mInputActions;
	std::vector<InputAction> mInputActionsPlayer[4];

	void LoadInputDefaults();
	void LoadInputDefaults(std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
};
