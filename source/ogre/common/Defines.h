#ifndef __Defines_h__
#define __Defines_h__


#define PI_d  3.14159265358979323846

#include <OgreLogManager.h>
#define LogO(s)  Ogre::LogManager::getSingleton().logMessage(s)

#include <OgreStringConverter.h>
#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

static Ogre::String fToStr(const float v, const char precision, const char width=0, const char fill=' ')
{
	std::ostringstream s;
	if (width != 0)  s.width(width);  s.fill(fill);
	s << std::fixed << std::setprecision(precision) << v;
	return s.str();
} 

#define s2r(s)  Ogre::StringConverter::parseReal(s)
#define s2i(s)  Ogre::StringConverter::parseInt(s)
#define s2c(s)  Ogre::StringConverter::parseColourValue(s)
#define s2v(s)  Ogre::StringConverter::parseVector3(s)
#define b2s(b)  (b) ? "true" : "false"

/// translation
#define TR(s)  MyGUI::LanguageManager::getInstance().replaceTags(s)


///  boost fuel params  ----
const static float gfBoostFuelStart = 3.f,  // seconds (each lap)
	gfBoostFuelMax = 3.f,  // max val, tank	
	gfBoostFuelAddSec = 0.1f;  // add value each second


//  info  for shape user data (void*)
//------------------------------------------
const static int  // & 0xFF !
	SU_Road			= 0x100, //+mtrId
	SU_Pipe			= 0x200, //+mtrId
	SU_RoadWall		= 0x300,
	//SU_RoadColumn	= 0x400,  //=Wall
	SU_Terrain		= 0x500,
	SU_Vegetation	= 0x600,  // trees, rocks etc
	SU_Border		= 0x700,  // world border planes
	SU_ObjectStatic	= 0x800;
	//SU_ObjectDynamic= 0x900;  //..


//  info  for special collision objects  (fluids, triggers)
//-------------------------------------------------------------
enum EShapeType
{
	ST_Car=0, ST_Fluid, ST_Wheel, ST_Other
};

class CARDYNAMICS;  class FluidBox;
class ShapeData
{
public:
	EShapeType type;
	CARDYNAMICS* pCarDyn;
	FluidBox* pFluid;
	int whNum;

	ShapeData( EShapeType type1)
		: type(type1), pCarDyn(0), pFluid(0), whNum(0)
	{	}
	ShapeData( EShapeType type1, CARDYNAMICS* pCarDyn1, FluidBox* pFluid1, int whNum1=0)
		: type(type1), pCarDyn(pCarDyn1), pFluid(pFluid1), whNum(whNum1)
	{	}
};

#endif
