#include "pch.h"
#include "toggle.h"
#include "unittest.h"

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

QT_TEST(toggle_test)
{
	TOGGLE t;
	t.Clear();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Tick();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Set(false);
	t.Tick();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Set(true);
	QT_CHECK(t.GetState() && t.GetImpulse() && t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Tick();
	QT_CHECK(t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Tick();
	QT_CHECK(t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Set(false);
	QT_CHECK(!t.GetState() && t.GetImpulse() && !t.GetImpulseRising() && t.GetImpulseFalling());
	t.Tick();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Tick();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
	t.Set(true);
	t.Tick();
	t.Clear();
	QT_CHECK(!t.GetState() && !t.GetImpulse() && !t.GetImpulseRising() && !t.GetImpulseFalling());
}
