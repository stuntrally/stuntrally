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
using namespace MyGUI;

#define powS(x, p)  (x >= 0.f ? powf(x, p) : -powf(-x, p))


Gui* SliderValue::pGUI = NULL;
bool* SliderValue::bGI = NULL;

//  ctor
SliderValue::SliderValue()
	:slider(NULL), text(NULL), edit(NULL)
	,pFloat(NULL), pInt(NULL)
	,fMin(0.f), fRange(1.f), fPow(1.f)
	,fmtDigits(2), fmtLength(4)
	,fmtValMul(1.f), sSuffix()
{	}


//  events
//-------------------------------------------------------------------------

//  slider moved
void SliderValue::Move(Slider* sl, float val)
{
	bool gi = bGI && *bGI;  // Gui was inited, set values and post event
	
	if (pFloat)
	{
		float v = fMin + fRange * (fPow != 1.f ? powS(val, fPow) : val);
		if (gi)
			*pFloat = v;
	}
	else //if (pInt)
	{
		int i = fMin + fRange * val + slHalf;
		if (gi)
			*pInt = i;
	}
	Update();
}

//  editbox text changed  (if present)
void SliderValue::Edit(EditBox* ed)
{
	if (pFloat)
	{
		float val = s2r(ed->getCaption());
		*pFloat = val;
		setValF(*pFloat);  // upd slider
	}
	else //if (pInt)
	{
		int val = s2i(ed->getCaption());
		*pInt = val;
		setValI(*pInt);
	}
	if (bGI && *bGI)
		event(this);  // callback
}


//  Update
//-------------------------------------------------------------------------

//  new val, upd sld and txt
//  (pFloat or val changed)
void SliderValue::Upd()
{
	if (!pFloat && !pInt)  return;
	if (pFloat)
		setValF(*pFloat);
	else //if (pInt)
		setValI(*pInt);
	UpdTxt();
}

//  update internal
void SliderValue::UpdTxt()
{
	String s = 
		pFloat ?
			fToStr(*pFloat * fmtValMul, fmtDigits,fmtLength) + sSuffix :
		(strMap.empty() ?
			iToStr(*pInt) :
			strMap[*pInt]);
	(edit ? edit : text)->setCaption(s);
}

void SliderValue::Update()
{
	UpdTxt();

	if (bGI && *bGI)
		event(this);  // callback
}

//  Set Value
//-------------------------------------------------------------------------

//  set default value for RMB on slider
void SliderValue::DefaultF(float f)
{
	float v = setValF(f);
	slider->mfDefault = v;
}

void SliderValue::DefaultI(int i)
{
	float v = setValI(i);
	slider->mfDefault = v;
}

//  set value
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
	float v = (f - fMin) / fRange;
	if (fPow != 1.f)
		v = powS(v, 1.f/fPow);

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
	slider = pGUI->findWidget<Slider>(name);  // throws if not found

	if (slider->eventValueChanged.empty())
		slider->eventValueChanged += newDelegate(this, &SliderValue::Move);

	text = pGUI->findWidget<TextBox>(name+"Val", false);   // not required

	edit = pGUI->findWidget<EditBox>(name+"Edit", false);  // not required

	if (edit && edit->eventEditTextChange.empty())
	            edit->eventEditTextChange += newDelegate(this, &SliderValue::Edit);
}

void SliderValue::setVisible(bool vis)
{
	if (slider)  slider->setVisible(vis);
	if (text)  text->setVisible(vis);
	if (edit)  edit->setVisible(vis);
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
	fPow = rPow;  fMin = rMin;
	fRange = rMax - rMin;
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

Gui* Check::pGUI = NULL;
bool* Check::bGI = NULL;

//  ctor
Check::Check()
	:chk(NULL), pBool(NULL)
{	}


//  button event
void Check::Click(Widget* btn)
{
	bool gi = bGI && *bGI;  // Gui inited
	if (pBool)
	{	bool b = chk->getStateSelected();
		if (gi)
			*pBool = b;
	}
	Update();
}

//  update checkbox if value or pointer changed
void Check::Upd()
{
	chk->setStateSelected(*pBool);
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
	chk = pGUI->findWidget<Button>(name);  // throws if not found

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
