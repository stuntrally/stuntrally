#pragma once
//#include <map>
//#include <MyGUI.h>

const float slHalf = 0.45f;  // added to int value sliders to their float value


namespace MyGUI {  class Slider;  }


//TODO: class SliderEdit for ed..

class SliderValue
{
private:
	MyGUI::Slider* slider;
	MyGUI::TextBox* text;  // for value show

public:
	typedef MyGUI::delegates::CMultiDelegate1<SliderValue*> ValueChanged;
	typedef std::map<int, Ogre::String> StrMap;
	

	//  set this first
	static MyGUI::Gui* pGUI;
	static bool* bGI;  // gui inited, true to assign value on move (to pF or pI), false in init


	//  pointer to value
	float* pFloat;
	int* pInt;
	
	//  format float value (for display only)
	int fmtDigits, fmtLength;
	float fmtValMul;  // value multiplier
	Ogre::String sSuffix;

	//  map with strings for all int values (fill before init)
	StrMap strMap;

	//  slider value range
	float fMin, fRange, fPow;

	ValueChanged event;  // sent after Move, user callback
		// add a method here if you need it executed


	//  ctor
	SliderValue();

	//  float   // name in .layout  // power, 1=linear
	void Init(Ogre::String name, float* pF,
			float rMin=0.f, float rMax=1.f, float rPow=1.f);
	void Init(Ogre::String name, float* pF,
			float rMin, float rMax, float rPow,
			int fmtDig, int fmtLen);
	void Init(Ogre::String name, float* pF,
			float rMin, float rMax, float rPow,
			int fmtDig, int fmtLen,
			float valMul, Ogre::String suffix);
	//  int
	void Init(Ogre::String name, int* pI,
			float rMin=0.f, float rMax=1.f);

	void SetValueF(float f);
	void SetValueI(int i);

private:
	float setValF(float f);
	float setValI(int i);

	//  event move slider
	void Move(MyGUI::Slider* sl, float val);

	//  update text and send event
	void Update();

	//  gui
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
	static bool* bGI;  // gui inited, true to assign value on move (to pF or pI), false in init


	//  pointer to value
	bool* pBool;
	
	ValueChanged event;  // after change, user callback
		// add a method here if you need it executed


	//  ctor
	Check();

	//  init   // name in .layout
	void Init(Ogre::String name, bool* pB);

	void SetValue(bool b);

private:
	void setVal(bool b);

	//  event gui
	void Click(MyGUI::Widget* btn);

	//  update text and send event
	void Update();

	//  gui
	void initGui(Ogre::String name);
};



///  .h
typedef SliderValue SV;

#define SlV(a)  SV sv##a;  void sl##a(SV*)  // declare slider and its event
