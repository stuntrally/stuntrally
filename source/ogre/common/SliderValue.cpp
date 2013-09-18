#include "pch.h"
#include "Defines.h"
#include "Gui_Def.h"
#include "Slider.h"
#include "SliderValue.h"

#include "MyGUI_Prerequest.h"
#include "MyGUI_Gui.h"
#include "MyGUI_Button.h"
#include "MyGUI_TextBox.h"
using namespace Ogre;


MyGUI::Gui* SliderValue::pGUI = 0;
bool* SliderValue::bGI = 0;

//  ctor
SliderValue::SliderValue()
	:slider(0), text(0)
	,pFloat(0), pInt(0)
	,fMin(0), fRange(1.f), fPow(1.f)
	,fmtDigits(2), fmtLength(4)
	,fmtValMul(1.f), sSuffix()
{	}


//  event move slider
//-------------------------------------------------------------------------
void SliderValue::Move(MyGUI::Slider* sl, float val)
{
	bool gi = bGI && *bGI;  // Gui was inited, set values and post event
	
	if (pFloat)
	{
		float v = fPow != 1.f ?
			fMin + fRange * powf(val, fPow) :
			fMin + fRange * val;
		if (gi)
			*pFloat = v;
	}
	else //if (pInt)
	{
		int i = fMin + val * fRange +slHalf;
		if (gi)
			*pInt = i;
	}
	Update();
}

//  update internal
void SliderValue::Update()
{
	if (text)
		text->setCaption(
			pFloat ?
				fToStr(*pFloat * fmtValMul, fmtDigits,fmtLength) + sSuffix :
			(strMap.empty() ?
				iToStr(*pInt) :
				strMap[*pInt]));

	if (bGI && *bGI)
		event(this);  // callback
}

//  set value
//-------------------------------------------------------------------------
void SliderValue::SetValueF(float f)
{
	*pFloat = f;
	float v = setValF(f);
	Update();
}

void SliderValue::SetValueI(int i)
{
	*pInt = i;
	float v = setValI(i);
	Update();
}

//  set val internal
float SliderValue::setValF(float f)
{
	float v = fPow != 1.f ?
		powf( (f - fMin) / fRange, 1.f/fPow) :
			( (f - fMin) / fRange);
	slider->setValue(v);
	return v;
}

float SliderValue::setValI(int i)
{
	float v = (i - fMin) / fRange;
	slider->setValue(v);
	return v;
}


//  Init
//-------------------------------------------------------------------------

//  Gui
void SliderValue::initGui(String name)
{
	slider = pGUI->findWidget<MyGUI::Slider>(name);  // throws if not found

	if (slider->eventValueChanged.empty())
		slider->eventValueChanged += newDelegate(this, &SliderValue::Move);

	text = pGUI->findWidget<MyGUI::TextBox>(name+"Val", false);  // not required
}

//  Float
//------------------------------------
void SliderValue::Init(
	String name, float* pF,
	float rMin, float rMax, float rPow)
{
	Init(name, pF,  rMin, rMax, rPow,  2, 4,  1.f, "");
}

void SliderValue::Init(
	String name, float* pF,
	float rMin, float rMax, float rPow,
	int fmtDig, int fmtLen)
{
	Init(name, pF,  rMin, rMax, rPow,  fmtDig, fmtLen,  1.f, "");
}

void SliderValue::Init(
	String name, float* pF,
	float rMin, float rMax, float rPow,
	int fmtDig, int fmtLen,
	float valMul, String suffix)
{
	initGui(name);
	fPow = rPow;
	fMin = rMin;  fRange = rMax - rMin;
	fmtDigits = fmtDig;  fmtLen = fmtLength;
	fmtValMul = valMul;  sSuffix = suffix;

	pFloat = pF;
	float v = setValF(*pF);
	Move(slider, v);  // no event bGI=false
}

//  Int
//------------------------------------
void SliderValue::Init(
	String name, int* pI,
	float rMin, float rMax)
{
	initGui(name);
	fMin = rMin;  fRange = rMax - rMin;

	pInt = pI;
	float v = setValI(*pI);
	Move(slider, v);
}


//  Check
//------------------------------------------------------------------------------------------------
MyGUI::Gui* Check::pGUI = 0;
bool* Check::bGI = 0;

//  ctor
Check::Check()
	:chk(0), pBool(0)
{	}


//  button event
void Check::Click(MyGUI::Widget* btn)
{
	bool gi = bGI && *bGI;  // Gui inited
	if (pBool)
	{	bool b = chk->getStateSelected();
		if (gi)
			*pBool = b;
	}
	Update();
}

//  update internal
void Check::Update()
{
	if (bGI && *bGI)
		event(this);  // callback
}


//  set value

void Check::SetValue(bool b)
{
	*pBool = b;
	chk->setStateSelected(b);
	Update();
}

//  Init
//------------------------------------

void Check::initGui(String name)
{
	chk = pGUI->findWidget<MyGUI::Button>(name);  // throws if not found

	if (chk->eventMouseButtonClick.empty())
		chk->eventMouseButtonClick += newDelegate(this, &Check::Click);
}

//  Init
void Check::Init(String name, bool* pB)
{
	initGui(name);

	pBool = pB;
	chk->setStateSelected(*pB);
}
