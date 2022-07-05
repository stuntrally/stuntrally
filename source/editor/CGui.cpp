#include "pch.h"
#include "enums.h"
#include "BaseApp.h"
#include "GuiCom.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/CScene.h"
#include "../vdrift/pathmanager.h"
#include "settings.h"
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Widget.h>
#include <MyGUI_Button.h>
#include <MyGUI_Window.h>
#include <MyGUI_TabControl.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


//  ctor
//--------------------------------------------------------------------------
CGui::CGui(App* app1)
	: app(app1), gcom(0)
{	
	int i;  // tool texts
	for (i=0; i<BR_TXT; ++i){  brTxt[i]=0;  brVal[i]=0;  brKey[i]=0;  }
	for (i=0; i<RD_TXT; ++i){  rdTxt[i]=0;  rdVal[i]=0;  rdKey[i]=0;  rdImg[i]=0;  }
	for (i=0; i<RDS_TXT;++i){  rdTxtSt[i]=0;  rdValSt[i]=0;  }

	for (i=0; i<ST_TXT; ++i)  stTxt[i]=0;
	for (i=0; i<FL_TXT; ++i)  flTxt[i]=0;
	for (i=0; i<OBJ_TXT;++i)  objTxt[i]=0;
	for (i=0; i<EMT_TXT;++i)  emtTxt[i]=0;
	
	for (i=0; i < 4/*MTRs*/; ++i)  {  btnRoad[i]=0;  btnPipe[i]=0;  }
	for (i=0; i < P_All; ++i)  {  liPickX[i] = 0.4f;  liPickW[i] = 300;  }

	sc = app1->scn->sc;
	scn = app1->scn;
	pSet = app1->pSet;
	data = app->scn->data;
	mGui = app->mGui;
}



//  Main menu
//----------------------------------------------------------------------------------------------------------------
void CGui::InitMainMenu()
{
	Btn btn;
	for (int i=0; i < WND_ALL; ++i)
	{
		const String s = toStr(i);
		app->mWndMainPanels[i] = fWP("PanMenu"+s);
		Btn("BtnMenu"+s, btnMainMenu);  app->mWndMainBtns[i] = btn;
	}

	//  center
	int wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
	
	Wnd wnd = app->mWndMain;  IntSize w = wnd->getSize();
	wnd->setPosition((wx-w.width)*0.5f, (wy-w.height)*0.5f);
}

void CGui::btnMainMenu(WP wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == app->mWndMainBtns[i])
		{
			pSet->bMain = false;
			pSet->inMenu = i;
			app->gui->toggleGui(false);
			return;
		}
}

void CGui::tabMainMenu(Tab tab, size_t id)
{
	if (id != 0)  return;  // <back
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->bMain = true;
	app->gui->toggleGui(false);  // back to main
}
