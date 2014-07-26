#pragma once
#include <string>
#include <assert.h>
#include "cartire.h"

const static char csTRKsurf[8/*NumTypes*/][10] =
{	"[none]", "Asphalt", "Grass", "Gravel", "Concrete", "Sand", "Cobbles", "[all]"	};


class TRACKSURFACE
{
public:
	enum TYPE
	{	NONE=0, ASPHALT, GRASS, GRAVEL, CONCRETE, SAND, COBBLES, NumTypes	};

	float friction, frictionX, frictionY;  // x,y - multipliers
	float bumpWaveLength, bumpAmplitude, bumpWaveLength2, bumpAmplitude2;
	float rollingDrag, rollingResist;

	TYPE type;
	std::string name, tireName;  // .tire file source (without ".tire")
	CARTIRE* tire;  /// tire params set
	
	static CARTIRE* pTireDefault;
	
	TRACKSURFACE() :
		friction(1.0f),
		frictionX(1.0f), frictionY(1.0f),
		bumpWaveLength(10.f), bumpAmplitude(0.f),
		bumpWaveLength2(14.f), bumpAmplitude2(0.f),
		rollingDrag(1.f), rollingResist(1.f),
		type(GRASS),
		tireName("DEFAULT"),
		tire(CARTIRE::None())
	{	}
	
	void setType(unsigned int i)
	{
		type = i < NumTypes ? (TYPE)i : NumTypes;
	}

	bool operator==(const TRACKSURFACE& t) const
	{
		return (type == t.type)
			&& (bumpWaveLength == t.bumpWaveLength) && (bumpAmplitude == t.bumpAmplitude)
			&& (friction == t.friction)
			&& (rollingDrag == t.rollingDrag) && (tire == t.tire)
			&& (rollingResist == t.rollingResist)
			&& (frictionX == t.frictionX) && (frictionY == t.frictionY)
			&& (bumpWaveLength2 == t.bumpWaveLength2) && (bumpAmplitude2 == t.bumpAmplitude2);
	}
	
	static TRACKSURFACE * None()
	{
		static TRACKSURFACE s;
		return &s;
	}
};
