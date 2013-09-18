#pragma once
#include <ostream>

class TOGGLE
{
private:
	bool state;
	bool laststate;
	
public:
	TOGGLE() {Clear();}

	void Set(bool nextstate);
	void Set(const TOGGLE & other) {state = other.GetState();laststate = (!state && other.GetImpulseFalling()) || (state && !other.GetImpulseRising());}
	bool GetState() const;
	bool GetImpulseRising() const;
	bool GetImpulseFalling() const;
	bool GetImpulse() const;
	
	void Clear() {state = false; laststate = false;}
	
	void DebugPrint(std::ostream & out);

	void Tick();
};
