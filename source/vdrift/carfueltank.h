#pragma once
#include "dbl.h"
#include "mathvector.h"
#include <iostream>


class CARFUELTANK
{
private:
	// constants
	Dbl capacity;
	Dbl density;
	MATHVECTOR<Dbl,3> position;
	
	// variables
	Dbl mass;
	Dbl volume;
	
	//for info only
	
	void UpdateMass()
	{
		mass = density * volume;
	}
	
public:
	//default constructor makes an S2000-like car
	CARFUELTANK()
		: capacity(0.0492), density(730.0), mass(730.0*0.0492), volume(0.0492)
	{	}

	void DebugPrint(std::ostream & out)
	{
		out << "---Fuel Tank---" << std::endl;
		out << "Cur vol " << volume << std::endl;
		out << "Capacity " << capacity << std::endl;
		out << "Mass " << mass << std::endl;
	}

	void SetCapacity (const Dbl& value)	{	capacity = value;	}

	void SetDensity (const Dbl& value)
	{
		density = value;
		UpdateMass();
	}

	void SetVolume (const Dbl& value)
	{
		volume = value;
		UpdateMass();
	}

	void SetPosition (const MATHVECTOR<Dbl,3>& value)	{	position = value;	}

	MATHVECTOR<Dbl,3> GetPosition() const				{	return position;	}

	Dbl GetMass() const		{	return mass;	}
		
	void Fill()				{	volume = capacity;		}
	
	bool Empty () const		{	return (volume <= 0.0);		}
	
	Dbl FuelPercent() const {	return volume / capacity;	}
	
	void Consume(Dbl amount)
	{
		volume -= amount;
		if (volume < 0.0)
			volume = 0.0;

		UpdateMass();
	}
};
