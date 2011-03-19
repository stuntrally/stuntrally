#include "stdafx.h"

#include "eventsystem.h"
#include "unittest.h"

//#include <vector>
using std::vector;

//#include <map>
using std::map;
	 
//#include <list>
using std::list;
	 
//#include <iostream>
using std::endl;

//#include <cassert>

void EVENTSYSTEM_SDL::Init(std::ostream & info_output)
{
	info_output << SDL_NumJoysticks() << " joystick";
	if (SDL_NumJoysticks() != 1)
		info_output << "s";
	info_output << " found";
	if (SDL_NumJoysticks() > 0)
		info_output << ":" << endl;
	else
		info_output << "." << endl;
	for (int i=0; i < SDL_NumJoysticks(); i++ ) 
	{
		info_output << "    " << i << ". " << SDL_JoystickName(i) << endl;
	}
	
	SDL_JoystickEventState(SDL_ENABLE);
	
	int j;
	for (j = 0; j < SDL_NumJoysticks(); j++)
	{
		SDL_Joystick * ptr = SDL_JoystickOpen(j);
		assert(ptr);
		joystick.push_back(JOYSTICK(ptr, SDL_JoystickNumAxes(ptr), SDL_JoystickNumButtons(ptr), SDL_JoystickNumHats(ptr)));
	}
	assert((int)joystick.size() == SDL_NumJoysticks());
}

void EVENTSYSTEM_SDL::BeginFrame()
{
	return;
	#if 0
	if (lasttick == 0)
		lasttick = SDL_GetTicks();
	else
	{
		double thistick = SDL_GetTicks();

		dt = (thistick-lasttick)/1000.0;
		
		/*if (throttle && dt < game.TickPeriod())
		{
			//cout << "throttling: " << lasttick.data << "," << thistick << endl;
			SDL_Delay(10);
			thistick = SDL_GetTicks();
			dt = (thistick-lasttick)/1000.0;
		}*/
		
		lasttick = thistick;
	}
	
	//-RecordFPS(1.0f/dt);
	#endif
}

void EVENTSYSTEM_SDL::ProcessEvents()
{
	SDL_Event event;
	
	AgeToggles <SDLKey> (keymap);
	AgeToggles <int> (mbutmap);
	for (std::vector <JOYSTICK>::iterator i = joystick.begin(); i != joystick.end(); i++)
	{
		i->AgeToggles();
	}
	
	while ( SDL_PollEvent( &event ) )
	{
		switch( event.type )
		{
		case SDL_MOUSEMOTION:
			HandleMouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			HandleMouseButton(DOWN, event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			HandleMouseButton(UP, event.button.button);
			break;
		case SDL_KEYDOWN:
			HandleKey(DOWN, event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			HandleKey(UP, event.key.keysym.sym);
			break;
		case SDL_JOYBUTTONDOWN:
			assert(event.jbutton.which < joystick.size()); //ensure the event came from a known joystick
			joystick[event.jbutton.which].SetButton(event.jbutton.button, true);
			break;
		case SDL_JOYBUTTONUP:
			assert(event.jbutton.which < joystick.size()); //ensure the event came from a known joystick
			joystick[event.jbutton.which].SetButton(event.jbutton.button, false);
			break;
		case SDL_JOYHATMOTION:
			break;
		case SDL_JOYAXISMOTION:
			assert(event.jaxis.which < joystick.size()); //ensure the event came from a known joystick
			joystick[event.jaxis.which].SetAxis(event.jaxis.axis, event.jaxis.value / 32768.0f);
			//std::cout << "Joy " << (int) event.jaxis.which << " axis " << (int) event.jaxis.axis << " value " << event.jaxis.value / 32768.0f << endl;
			break;
		case SDL_QUIT:
			HandleQuit();
			break;
		default:
			break;
		}
	}
}

void EVENTSYSTEM_SDL::HandleMouseMotion(int x, int y, int xrel, int yrel)
{
	mousex = x;
	mousey = y;
	mousexrel = xrel;
	mouseyrel = yrel;
}

void EVENTSYSTEM_SDL::HandleMouseButton(DIRECTION dir, int id)
{
	//std::cout << "Mouse button " << id << ", " << (dir==DOWN) << endl;
	//mbutmap[id].Tick();
	HandleToggle <int> (mbutmap, dir, id);
}

void EVENTSYSTEM_SDL::HandleKey(DIRECTION dir, SDLKey id)
{
	//if (dir == DOWN) std::cout << "Key #" << (int)id << " pressed" << endl;
	HandleToggle <SDLKey> (keymap, dir, id);
}

EVENTSYSTEM_SDL::BUTTON_STATE EVENTSYSTEM_SDL::GetMouseButtonState(int id) const
{
	return GetToggle <int> (mbutmap, id);
}

EVENTSYSTEM_SDL::BUTTON_STATE EVENTSYSTEM_SDL::GetKeyState(SDLKey id) const
{
	return GetToggle <SDLKey> (keymap, id);
}

vector <int> EVENTSYSTEM_SDL::GetMousePosition() const
{
	vector <int> o;
	o.reserve(2);
	o.push_back(mousex);
	o.push_back(mousey);
	return o;
}

vector <int> EVENTSYSTEM_SDL::GetMouseRelativeMotion() const
{
	vector <int> o;
	o.reserve(2);
	o.push_back(mousexrel);
	o.push_back(mouseyrel);
	return o;
}

void EVENTSYSTEM_SDL::TestStim(TEST_STIM stim)
{
	if (stim == STIM_AGE_KEYS)
	{
		AgeToggles <SDLKey> (keymap);
	}
	if (stim == STIM_AGE_MBUT)
	{
		AgeToggles <int> (mbutmap);
	}
	if (stim == STIM_INSERT_KEY_DOWN)
	{
		HandleKey(DOWN, SDLK_t);
	}
	if (stim == STIM_INSERT_KEY_UP)
	{
		HandleKey(UP, SDLK_t);
	}
	if (stim == STIM_INSERT_MBUT_DOWN)
	{
		HandleMouseButton(DOWN, SDL_BUTTON_LEFT);
	}
	if (stim == STIM_INSERT_MBUT_UP)
	{
		HandleMouseButton(UP, SDL_BUTTON_LEFT);
	}
	if (stim == STIM_INSERT_MOTION)
	{
		HandleMouseMotion(50,55,2,1);
	}
}

QT_TEST(eventsystem_test)
{
	EVENTSYSTEM_SDL e;
	
	//key stuff
	{
		//check key insertion
		e.TestStim(EVENTSYSTEM_SDL::STIM_INSERT_KEY_DOWN);
		EVENTSYSTEM_SDL::BUTTON_STATE tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(tstate.down && tstate.just_down && !tstate.just_up);
		
		//check key aging
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_KEYS);
		tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(tstate.down && !tstate.just_down && !tstate.just_up);
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_KEYS); //age again
		tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(tstate.down && !tstate.just_down && !tstate.just_up);
		
		//check key removal
		e.TestStim(EVENTSYSTEM_SDL::STIM_INSERT_KEY_UP);
		tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(!tstate.down && !tstate.just_down && tstate.just_up);
		
		//check key aging
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_KEYS);
		tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(!tstate.down && !tstate.just_down && !tstate.just_up);
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_KEYS); //age again
		tstate = e.GetKeyState(SDLK_t);
		QT_CHECK(!tstate.down && !tstate.just_down && !tstate.just_up);
	}
	
	//mouse button stuff
	{
		//check button insertion
		e.TestStim(EVENTSYSTEM_SDL::STIM_INSERT_MBUT_DOWN);
		EVENTSYSTEM_SDL::BUTTON_STATE tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(tstate.down && tstate.just_down && !tstate.just_up);
	
		//check button aging
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_MBUT);
		tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(tstate.down && !tstate.just_down && !tstate.just_up);
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_MBUT); //age again
		tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(tstate.down && !tstate.just_down && !tstate.just_up);
	
		//check button removal
		e.TestStim(EVENTSYSTEM_SDL::STIM_INSERT_MBUT_UP);
		tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(!tstate.down && !tstate.just_down && tstate.just_up);
	
		//check button aging
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_MBUT);
		tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(!tstate.down && !tstate.just_down && !tstate.just_up);
		e.TestStim(EVENTSYSTEM_SDL::STIM_AGE_MBUT); //age again
		tstate = e.GetMouseButtonState(SDL_BUTTON_LEFT);
		QT_CHECK(!tstate.down && !tstate.just_down && !tstate.just_up);
	}
	
	//mouse motion stuff
	{
		e.TestStim(EVENTSYSTEM_SDL::STIM_INSERT_MOTION);
		vector <int> mpos = e.GetMousePosition();
		QT_CHECK_EQUAL(mpos.size(),2);
		QT_CHECK_EQUAL(mpos[0],50);
		QT_CHECK_EQUAL(mpos[1],55);
		vector <int> mrel = e.GetMouseRelativeMotion();
		QT_CHECK_EQUAL(mrel.size(),2);
		QT_CHECK_EQUAL(mrel[0],2);
		QT_CHECK_EQUAL(mrel[1],1);
	}
}

void EVENTSYSTEM_SDL::RecordFPS(const float fps)
{
	fps_memory.push_back(fps);
	if (fps_memory.size() > fps_memory_window)
		fps_memory.pop_front();
	
	//ensure no fps memory corruption
	assert(fps_memory.size() <= fps_memory_window);
}

float EVENTSYSTEM_SDL::GetFPS() const
{
	float avg(0);
	for (list <float>::const_iterator i = fps_memory.begin(); i != fps_memory.end(); i++)
	{
		avg += *i;
	}
	//float avg = std::accumulate(fps_memory.begin(), fps_memory.end(), 0);
	
	if (!fps_memory.empty())
		avg = avg / fps_memory.size();
	
	return avg;
}
