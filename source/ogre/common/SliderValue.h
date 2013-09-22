#pragma once
//#include <map>
#include <OgreString.h>
#include <MyGUI_Delegate.h>

namespace MyGUI {  class Slider;  }

const float slHalf = 0.45f;  // added to int value sliders to their float value


class SliderValue
{
	friend class CGui;
private:
	MyGUI::Slider* slider;
	//  not required
	MyGUI::TextBox* text;  // for value show
	MyGUI::EditBox* edit;  // for value edit

public:
	typedef MyGUI::delegates::CMultiDelegate1<SliderValue*> ValueChanged;
	typedef std::map<int, Ogre::String> StrMap;
	

	//  set this first
	static MyGUI::Gui* pGUI;  // for findWidget
	//  gui inited, true to assign value on move (to pF or pI), false in init
	static bool* bGI;


	//  pointer to value
	float* pFloat;
	int* pInt;

	
	//  format float value (for display only)
	int fmtDigits, fmtLength;

	float fmtValMul;  // value multiplier
	Ogre::String sSuffix;


	//  map with strings for all int values (if needed, fill it before init)
	StrMap strMap;


	//  slider value range
	float fMin, fRange, fPow;

	ValueChanged event;  // sent after Move, user callback
		// add a method here if you need it executed


	//  ctor
	SliderValue();

	//  Init
	//  float*  // name in .layout  // power, 1=linear
	void Init(Ogre::String name, float* pF,
			float rMin=0.f, float rMax=1.f, float rPow=1.f);
	void Init(Ogre::String name, float* pF,
			float rMin, float rMax, float rPow,
			int fmtDig, int fmtLen);
	void Init(Ogre::String name, float* pF,
			float rMin, float rMax, float rPow,
			int fmtDig, int fmtLen,
			float valMul, Ogre::String suffix);
	//  int*
	void Init(Ogre::String name, int* pI,
			int rMin=0, int rMax=1, float rPow=1.f);

	//  update slider and text (new value in *pFloat or pFloat changed)
	void Upd();
	
	void setVisible(bool vis);


	//  default value for RMB click (real values)
	void DefaultF(float f);
	void DefaultI(int i);

	void SetValueF(float f);
	void SetValueI(int i);

private:
	void setValF(float f);  // set slider
	void setValI(int i);

	float getValF(float f);  // get slider value (0..1) for given float value (real)
	float getValI(int i);    // - - for int value


	//  event  slider moved
	void Move(MyGUI::Slider* sl, float val);

	//  event  editbox text changed
	void Edit(MyGUI::EditBox* sl);

	//  update text and send event
	void Update(), UpdTxt();

	//  init Gui
	void initGui(Ogre::String name);
};


class Check
{
private:
	MyGUI::Button* chk;

public:
	typedef MyGUI::delegates::CMultiDelegate1<Check*> ValueChanged;
	

	//  set this first
	static MyGUI::Gui* pGUI;
	static bool* bGI;  // gui inited, true to assign value, false in init


	//  pointer to value
	bool* pBool;
	
	ValueChanged event;  // after change, user callback
		// add a method here if you need it executed


	//  ctor
	Check();

	//  init   // name in .layout
	void Init(Ogre::String name, bool* pB);

	//  update checkbox if value or pointer changed
	void Upd();


	void SetValue(bool b);
	void Invert();  // set opposite value

private:
	void setVal(bool b);


	//  event  clicked
	void Click(MyGUI::Widget* btn);

	//  update text and send event
	void Update();

	//  init Gui
	void initGui(Ogre::String name);
};
