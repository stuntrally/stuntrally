#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../vdrift/pathmanager.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/Slider.h"
#include "../shiny/Main/Factory.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;


///  gui tweak page, material properties
//------------------------------------------------------------------------------------------------------------
void CGui::CreateGUITweakMtr()
{
	ScrollView* view = app->mGui->findWidget<ScrollView>("TweakView",false);
	if (!view)  return;
	
	//  clear last view
	MyGUI::EnumeratorWidgetPtr widgets = view->getEnumerator ();
	app->mGui->destroyWidgets(widgets);

	if (pSet->tweak_mtr == "")  return;
	sh::MaterialInstance* mat = app->mFactory->getMaterialInstance(pSet->tweak_mtr);
	//if (!mat)  return;
	
	int y = 0;
	const auto& props = mat->listProperties();
	for (auto it : props)
	{
		sh::PropertyValuePtr pv = it.second;
		std::string name = it.first;
		
		//  get type
		std::string sVal = pv->_getStringValue();
		//? if (boost::is_alnum(sVal))  continue;
		bool isStr = false;
		for (int c=0; c < sVal.length(); ++c)  if (sVal[c] >= 'a' && sVal[c] <= 'z')
			isStr = true;

		if (!isStr)
		{
			//  get size
			std::vector<std::string> tokens;
			boost::split(tokens, sVal, boost::is_any_of(" "));
			int size = tokens.size();

			//LogO("PROP: " + name + "  val: " + sVal + "  type:" + toStr(type));
			const static char ch[6] = "rgbau";
			const static Colour clrsType[5] = {Colour(0.9,0.9,0.7),Colour(0.8,1.0,0.8),
						Colour(0.7,0.85,1.0),Colour(0.7,1.0,1.0),Colour(1.0,1.0,1.0)};

			//  for each component (xy,rgb..)
			for (int i=0; i < size; ++i)
			{
				String nameSi = name + ":" + toStr(size) + "." + toStr(i);  // size and id in name
				float val = boost::lexical_cast<float> (tokens[i]);
				int t = std::min(4,i);  const Colour& clr = clrsType[std::max(0,std::min(4,size-1))];

				//  name text
				int x = 0, xs = 150;
				TextBox* txt = view->createWidget<TextBox>("TextBox", x,y, xs,20, Align::Default, nameSi + ".txt");
				gcom->setOrigPos(txt, "OptionsWnd");  txt->setTextColour(clr);
				txt->setCaption(size == 1 ? name : name + "." + ch[t]);

				//  val edit
				x += xs;  xs = 60;
				EditBox* edit = view->createWidget<EditBox>("EditBox", x,y, xs,20, Align::Default, nameSi + "E");
				gcom->setOrigPos(edit, "OptionsWnd");  edit->setTextColour(clr);  edit->setColour(clr);
				edit->setCaption(fToStr(val,3,6));
				if (edit->eventEditTextChange.empty())  edit->eventEditTextChange += newDelegate(this, &CGui::edTweak);
				
				//  slider
				x += xs + 10;  xs = 400;
				Slider* sl = view->createWidget<Slider>("Slider", x,y-1, xs,19, Align::Default, nameSi);
				gcom->setOrigPos(sl, "OptionsWnd");  sl->setColour(clr);
				sl->setValue(val);  //powf(val * 1.f/2.f, 1.f/2.f));  //v
				if (sl->eventValueChanged.empty())  sl->eventValueChanged += newDelegate(this, &CGui::slTweak);

				y += 22;
			}
			y += 8;
		}
	}
	view->setCanvasSize(1300, y+1000);  //par-
	view->setCanvasAlign(Align::Default);

	gcom->doSizeGUI(view->getEnumerator());
}

//  gui change val events
//-----------------------------------------------------------------
void CGui::slTweak(Slider* sl, float val)
{
	std::string name = sl->getName();

	EditBox* edit = fEd(name + "E");
	if (edit)
		edit->setCaption(fToStr(val,3,6));

	TweakSetMtrPar(name, val);
}

void CGui::edTweak(EditPtr ed)
{
	std::string name = ed->getName();  name = name.substr(0,name.length()-1);  // ends with E
	float val = s2r(ed->getCaption());
	
	Slider* sl = app->mGui->findWidget<Slider>(name);
	if (sl)
		sl->setValue(val);

	TweakSetMtrPar(name, val);
}

///  change material property (float component)
//-----------------------------------------------------------------
void CGui::TweakSetMtrPar(std::string name, float val)
{
	std::string prop = name.substr(0,name.length()-4);  // cut ending, eg :2.1
	
	int id = -1, size = 1;
	if (name.substr(name.length()-2,1) == ".")  // more than 1 float
	{
		id = s2i(name.substr(name.length()-1,1));
		size = s2i(name.substr(name.length()-3,1));
	}
	//val = powf(val * 2.f, 2.f);  //v

	sh::MaterialInstance* mat = app->mFactory->getMaterialInstance(pSet->tweak_mtr);
	if (size == 1)  // 1 float
		mat->setProperty(prop, sh::makeProperty<sh::FloatValue>(new sh::FloatValue(val)));
	else
	{
		sh::PropertyValuePtr& vp = mat->getProperty(prop);
		switch (size)
		{
			case 2:
			{	sh::Vector2 v = sh::retrieveValue<sh::Vector2>(vp,0);
				((float*)&v.mX)[id] = val;
				mat->setProperty(prop, sh::makeProperty<sh::Vector2>(new sh::Vector2(v.mX, v.mY)));
			}	break;
			case 3:
			{	sh::Vector3 v = sh::retrieveValue<sh::Vector3>(vp,0);
				((float*)&v.mX)[id] = val;
				mat->setProperty(prop, sh::makeProperty<sh::Vector3>(new sh::Vector3(v.mX, v.mY, v.mZ)));
			}	break;
			case 4:
			{	sh::Vector4 v = sh::retrieveValue<sh::Vector4>(vp,0);
				((float*)&v.mX)[id] = val;
				mat->setProperty(prop, sh::makeProperty<sh::Vector4>(new sh::Vector4(v.mX, v.mY, v.mZ, v.mW)));
			}	break;
		}
	}
}

//  pick material from combo
void CGui::comboTweakMtr(ComboBoxPtr cmb, size_t val)
{
	pSet->tweak_mtr = cmb->getItemNameAt(val);
	CreateGUITweakMtr();
}
