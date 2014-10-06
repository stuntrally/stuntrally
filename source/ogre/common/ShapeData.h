#pragma once

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
	SU_ObjectStatic	= 0x800,
	SU_Fluid 		= 0x900; //+surfId  solid fluids, ice etc
	//SU_ObjectDynamic= 0xA00;  //..


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
