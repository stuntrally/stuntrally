#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "Slider.h"
#include "SliderValue.h"

#include "MyGUI_Prerequest.h"
#include "MyGUI_Gui.h"
#include "MyGUI_Button.h"
#include "MyGUI_TextBox.h"
#include "MyGUI_EditBox.h"
using namespace Ogre;
using namespace MyGUI;

#define powS(x, p)  (x >= 0.f ? powf(x, p) : -powf(-x, p))


Gui* SliderValue::pGUI = NULL;
bool* SliderValue::bGI = NULL;


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
		int i = fMin + fRange * (fPow != 1.f ? powS(val, fPow) : val) + slHalf;
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

//  change ponter to value
void SliderValue::UpdF(float *pF)
{
	pFloat = pF;
	Upd();
}
void SliderValue::UpdI(int *pI)
{
	pInt = pI;
	Upd();
}

float SliderValue::getF()
{
	if (!pFloat)  return 0.f;
	return *pFloat;
}

//  value changed, update slided and text
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
	float v = getValF(f);
	slider->mfDefault = v;
}

void SliderValue::DefaultI(int i)
{
	float v = getValI(i);
	slider->mfDefault = v;
}

//  set value
void SliderValue::SetValueF(float f)
{
	*pFloat = f;
	setValF(f);
	Update();
}

void SliderValue::SetValueI(int i)
{
	*pInt = i;
	setValI(i);
	Update();
}

//  set val internal
float SliderValue::getValF(float f)
{
	float v = (f - fMin) / fRange;
	if (fPow != 1.f)
		v = powS(v, 1.f/fPow);
	return v;
}
void SliderValue::setValF(float f)
{
	slider->setValue(getValF(f));
}

float SliderValue::getValI(int i)
{
	float v = (i - fMin) / fRange;
	if (fPow != 1.f)
		v = powS(v, 1.f/fPow);
	return v;
}
void SliderValue::setValI(int i)
{
	slider->setValue(getValI(i));
}


//  Init
//-------------------------------------------------------------------------

//  Gui
void SliderValue::initGui(String name)
{
	slider = pGUI->findWidget<Slider>(name, false);
	if (!slider)  LogO("!! Error: GUI slider not found: "+name);  // will crash after

	if (slider && slider->eventValueChanged.empty())
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

void SliderValue::setText(String txt)
{
	if (text)  text->setCaption(txt);
}
void SliderValue::setTextClr(float r,float g,float b)
{
	if (text)  text->setTextColour(Colour(r,g,b));
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
	fMin = rMin;  fPow = rPow;
	fRange = rMax - rMin;
	fmtDigits = fmtDig;  fmtLen = fmtLength;
	fmtValMul = valMul;  sSuffix = suffix;

	pFloat = pF;
	setValF(*pF);
	Update();  // no event bGI=false
}

//  Int
//------------------------------------
void SliderValue::Init(
	String name, int* pI,
	int rMin, int rMax, float rPow)
{
	initGui(name);
	fMin = rMin;  fPow = rPow;
	fRange = rMax - rMin;

	pInt = pI;
	setValI(*pI);
	Update();
}


//  Check
//------------------------------------------------------------------------------------------------

Gui* Check::pGUI = nullptr;
bool* Check::bGI = nullptr;


//  button event
void Check::Click(Widget* btn)
{
	bool gi = bGI && *bGI;  // Gui inited
	if (pBool)
	{
		bool b = !chk->getStateSelected();
		chk->setStateSelected(b);
		if (gi)
			*pBool = b;
	}
	Update();
}

//  update checkbox if value or pointer changed
void Check::Upd(bool* pB)
{
	pBool = pB;
	Upd();
}
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

void Check::Invert()
{
	SetValue(!(*pBool));
}

void Check::setVisible(bool vis)
{
	chk->setVisible(vis);
}


//  Init
//------------------------------------

void Check::initGui(String name)
{
	chk = pGUI->findWidget<Button>(name, false);
	if (!chk)  LogO("!! Error: GUI button not found: "+name);  // will crash after

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
