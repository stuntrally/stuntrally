#ifndef _SIGNALPROCESSING_H
#define _SIGNALPROCESSING_H

//#include <ostream>
//#include <vector>

namespace signalprocessing
{

class DELAY
{
	private:
		std::vector <float> oldstate;
		int index;
		
	public:
		DELAY(int newdelayticks)
		{
			oldstate.resize(newdelayticks+1);
			Clear(0.0);
		}
		
		void Clear(float clearvalue)
		{
			for (int i = 0; i < (int)oldstate.size(); i++)
				oldstate[i] = clearvalue;
			
			index = 0;
		}
		
		float Process(float input)
		{
			assert (index < (int)oldstate.size());
			
			oldstate[index] = input;
			index++;
			if (index >= (int)oldstate.size())
				index = 0;
			return oldstate[index];
		}
};

class LOWPASS
{
	private:
		float lastout;
		float coeff;
		
	public:
		LOWPASS(float newcoeff) : coeff(newcoeff) {}
		
		float Process(float input)
		{
			lastout = input*coeff + lastout*(1.0f-coeff);
			return lastout;
		}
};

class PID
{
	private:
		float dState; // Last position input
		float iState; // Integrator state
		float iMax, iMin; // Maximum and minimum allowable integrator state
		float	pGain, // integral gain
			iGain, // proportional gain
			dGain; // derivative gain
		bool limiting;
	
	public:
		PID(float p, float i, float d, bool newlimiting) : dState(0), iState(0), iMax(1), iMin(-1),
			pGain(p),iGain(i),dGain(d),limiting(newlimiting) {}
		
		float Process(float error, float position)
		{
			float pTerm, dTerm, iTerm;
			pTerm = pGain * error;
			// calculate the proportional term
			// calculate the integral state with appropriate limiting
			iState += error;
			if (limiting)
			{
				if (iState > iMax) iState = iMax;
				else if (iState < iMin) iState = iMin;
			}
			iTerm = iGain * iState;  // calculate the integral term
			dTerm = dGain * (position - dState);
			dState = position;
			return pTerm + iTerm - dTerm;
		}
		
		void SetState(float init)
		{
			iState = init;
		}
};

}

#endif
