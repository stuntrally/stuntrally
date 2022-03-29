#include "pch.h"
#include "toggle.h"

using std::endl;

void TOGGLE::DebugPrint(std::ostream & out)
{
	out << "State: " << state << "  Last state: " << laststate << endl;
}

void TOGGLE::Set(bool nextstate)
{
	if (nextstate)
		state = true;
	else
		state = false;
}

bool TOGGLE::GetState() const
{
	return state;
}

bool TOGGLE::GetImpulseRising() const
{
	return (state && !laststate);
}

bool TOGGLE::GetImpulseFalling() const
{
	return (!state && laststate);
}

bool TOGGLE::GetImpulse() const
{
	return (GetImpulseRising() || GetImpulseFalling());
}

void TOGGLE::Tick()
{
	laststate = state;
}
