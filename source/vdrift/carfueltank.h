#ifndef _CARFUELTANK_H
#define _CARFUELTANK_H

#include "mathvector.h"
#include "joeserialize.h"
#include "macros.h"
//#include <iostream>
#include "../ogre/common/Defines.h"


template <typename T>
class CARFUELTANK
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		T capacity;
		T density;
		MATHVECTOR <T, 3> position;
		
		//variables
		T mass;
		T volume;
		
		//for info only
		
		void UpdateMass()
		{
			mass = density * volume;
		}
		
	public:
		//default constructor makes an S2000-like car
		CARFUELTANK() : capacity(0.0492), density(730.0), mass(730.0*0.0492), volume(0.0492) {}

		void DebugPrint(std::ostream & out)
		{
			out << "---Fuel Tank---" << std::endl;
			out << "Cur vol " << volume << std::endl;
			out << "Capacity " << capacity << std::endl;
			out << "Mass " << mass << std::endl;
		}

	void SetCapacity ( const T& value )
	{
		capacity = value;
	}

	void SetDensity ( const T& value )
	{
		density = value;
		UpdateMass();
	}

	void SetPosition ( const MATHVECTOR< T, 3 >& value )
	{
		position = value;
	}
	

	MATHVECTOR< T, 3 > GetPosition() const
	{
		return position;
	}

	T GetMass() const
	{
		return mass;
	}

	void SetVolume ( const T& value )
	{
		volume = value;
		UpdateMass();
	}
		
	void Fill()
	{
		volume = capacity;
	}
	
	bool Empty () const {return (volume <= 0.0);}
	
	T FuelPercent() const {return volume / capacity;}
	
	void Consume(T amount)
	{
		volume -= amount;
		if (volume < 0.0)
		{
			volume = 0.0;
		}

		UpdateMass();
	}
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,mass);
		_SERIALIZE_(s,volume);
		return true;
	}
};

#endif
