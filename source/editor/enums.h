#pragma once

enum ED_MODE
{
	ED_Deform=0, ED_Smooth, ED_Height, ED_Filter, /*ED_Paint,*/
	ED_Road, ED_Start, ED_PrvCam, ED_Fluids, ED_Objects, ED_Particles, ED_ALL
};
enum WND_Types
{	WND_Track=0, WND_Edit, WND_Help, WND_Options, WND_ALL  };  // pSet->inMenu
enum TABS_Edit
{	TAB_Back=0, TAB_Sun, TAB_Terrain, TAB_Layers, TAB_Grass, TAB_Veget, TAB_Surface, TAB_Road, TAB_Objects, TabsEdit_ALL  };
