#ifndef _EVENTSYSTEM_H
#define _EVENTSYSTEM_H

//#include <SDL/SDL.h>
#include "toggle.h"
//#include <vector>
//#include <map>
//#include <list>
//#include <iostream>
//#include <cassert>

class EVENTSYSTEM_SDL
{
public:
	class BUTTON_STATE
	{
	public:
		bool down; 	//button is down (false for up)
		bool just_down; //button was just pressed
		bool just_up;	//button was just released
		
		BUTTON_STATE() : down(false), just_down(false), just_up(false) {}
	};
	
	class JOYSTICK
	{
		public:
			class HATPOSITION
			{
				public:
					HATPOSITION() : centered(true), up(false), right(false), down(false), left(false) {}
					
					bool centered;
					bool up;
					bool right;
					bool down;
					bool left;
			};
		
		private:
			SDL_Joystick * sdl_joyptr;
			std::vector <float> axis;
			std::vector <TOGGLE> button;
			std::vector <HATPOSITION> hat;
		
		public:
			JOYSTICK(SDL_Joystick * ptr, int numaxes, int numbuttons, int numhats) : sdl_joyptr(ptr)
			{
				axis.resize(numaxes, 0);
				button.resize(numbuttons);
				hat.resize(numhats);
			}
			
			int GetNumAxes() const {return axis.size();}
			
			int GetNumButtons() const {return button.size();}
			
			void AgeToggles()
			{
				for (std::vector <TOGGLE>::iterator i = button.begin(); i != button.end(); ++i)
				{
					i->Tick();
				}
			}
			
			void SetAxis(unsigned int axisid, float newval)
			{
				assert (axisid < axis.size());
				axis[axisid] = newval;
			}
			
			float GetAxis(unsigned int axisid) const
			{
				//assert (axisid < axis.size()); //don't want to assert since this could be due to a control file misconfiguration
				if (axisid >= axis.size())
					return 0.0;
				else
					return axis[axisid];
			}
			
			TOGGLE GetButton(unsigned int buttonid) const
			{
				//don't want to assert since this could be due to a control file misconfiguration
				
				if (buttonid >= button.size())
					return TOGGLE();
				else
					return button[buttonid];
			}
			
			void SetButton(unsigned int id, bool state)
			{
				assert (id < button.size());
				button[id].Set(state);
			}
	};
	
private:
	double lasttick;
	double dt;
	bool quit;
	
	std::map <SDLKey, TOGGLE> keymap;
	std::map <int, TOGGLE> mbutmap;
	
	int mousex, mousey, mousexrel, mouseyrel;
	
	unsigned int fps_memory_window;
	std::list <float> fps_memory;
	
	enum DIRECTION {UP, DOWN};
	void HandleMouseMotion(int x, int y, int xrel, int yrel);
	
	template <class T>
	void HandleToggle(std::map <T, TOGGLE> & togglemap, const DIRECTION dir, const T & id)
	{
		togglemap[id].Tick();
		togglemap[id].Set(dir == DOWN);
	}
	void HandleMouseButton(DIRECTION dir, int id);
	void HandleKey(DIRECTION dir, SDLKey id);
	
	void RecordFPS(const float fps);
	
	void HandleQuit() {quit = true;}
	template <class T>
	void AgeToggles(std::map <T, TOGGLE> & togglemap)
	{
		std::list <typename std::map<T, TOGGLE>::iterator> todel;
		for(typename std::map <T, TOGGLE>::iterator i = togglemap.begin(); i != togglemap.end(); ++i)
		{
			i->second.Tick();
			if (!i->second.GetState() && !i->second.GetImpulseFalling())
				todel.push_back(i);
		}
		
		for(typename std::list <typename std::map<T, TOGGLE>::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			togglemap.erase(*i);
	}
	
	template <class T>
	BUTTON_STATE GetToggle(const std::map <T, TOGGLE> & togglemap, const T & id) const
	{
		BUTTON_STATE s;
		s.down = s.just_down = s.just_up = false;
		typename std::map <T, TOGGLE>::const_iterator i = togglemap.find(id);
		if (i != togglemap.end())
		{
			s.down = i->second.GetState();
			s.just_down = i->second.GetImpulseRising();
			s.just_up = i->second.GetImpulseFalling();
		}
		return s;
	}
	
	std::vector <JOYSTICK> joystick;

public:
	EVENTSYSTEM_SDL() : lasttick(0),dt(0),quit(false),mousex(0), mousey(0), mousexrel(0),mouseyrel(0),fps_memory_window(10) {}
	~EVENTSYSTEM_SDL() {}
	
	void Init(std::ostream & info_output);
	void BeginFrame();
	void EndFrame() {}
	inline double Get_dt() {return dt;}
	inline bool GetQuit() const {return quit;}
	void Quit() {quit = true;}
	void ProcessEvents();
	
	BUTTON_STATE GetMouseButtonState(int id) const;
	BUTTON_STATE GetKeyState(SDLKey id) const;
	BUTTON_STATE GetKeyState(int id) const
	{
		SDLKey keyid = (SDLKey) id;
		return GetKeyState(keyid);
	}
	
	///note that when the mouse cursor is hidden, it is also grabbed (confined to the application window)
	void SetMouseCursorVisibility(bool visible)
	{
		if (visible)
		{
			SDL_ShowCursor(SDL_ENABLE);
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		}
		else
		{
			SDL_ShowCursor(SDL_DISABLE);
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}
	}
	
	std::map <SDLKey, TOGGLE> & GetKeyMap() {return keymap;}
	
	///returns a 2 element vector (x,y)
	std::vector <int> GetMousePosition() const;
	
	///returns a 2 element vector (x,y)
	std::vector <int> GetMouseRelativeMotion() const;
	
	float GetFPS() const;
	
	enum TEST_STIM
	{
		STIM_AGE_KEYS,
  		STIM_AGE_MBUT,
    		STIM_INSERT_KEY_DOWN,
      		STIM_INSERT_KEY_UP,
      		STIM_INSERT_MBUT_DOWN,
		STIM_INSERT_MBUT_UP,
		STIM_INSERT_MOTION
	};
	void TestStim(TEST_STIM stim);
	
	float GetJoyAxis(unsigned int joynum, int axisnum) const
	{
		if (joynum < joystick.size())
			return joystick[joynum].GetAxis(axisnum);
		else
			return 0.0;
	}
	
	TOGGLE GetJoyButton(unsigned int joynum, int buttonnum) const
	{
		if (joynum < joystick.size())
			return joystick[joynum].GetButton(buttonnum);
		else
			return TOGGLE();
	}
	
	int GetNumJoysticks() const {return joystick.size();}
	
	int GetNumAxes(unsigned int joynum) const
	{
		if (joynum < joystick.size())
			return joystick[joynum].GetNumAxes();
		else
			return 0;
	}
	
	int GetNumButtons(unsigned int joynum) const
	{
		if (joynum < joystick.size())
			return joystick[joynum].GetNumButtons();
		else
			return 0;
	}
	
	std::vector <JOYSTICK> & GetJoysticks() {return joystick;}
};

#endif
