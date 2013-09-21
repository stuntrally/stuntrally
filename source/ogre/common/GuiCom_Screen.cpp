#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CGui.h"
	#include "../SplitScreen.h"
	#include "../vdrift/settings.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
#endif
#include "SDL_video.h"
using namespace MyGUI;
using namespace Ogre;


//  [Screen] 
//-----------------------------------------------------------------------------------------------------------

void CGui::cmbAntiAliasing(MyGUI::ComboBox* wp, size_t val)
{
	int v = s2i(wp->getItemNameAt(val));
	if (bGI)  pSet->fsaa = v;
}

///  resolutions
//  change
void CGui::btnResChng(WP)
{
	if (!resList)  return;
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE)  return;
	String mode = resList->getItemNameAt(resList->getIndexSelected());

	pSet->windowx = StringConverter::parseInt(StringUtil::split(mode, "x")[0]);
	pSet->windowy = StringConverter::parseInt(StringUtil::split(mode, "x")[1]);

	Uint32 flags = SDL_GetWindowFlags(app->mSDLWindow);
	if (flags & SDL_WINDOW_MAXIMIZED)  // Can't change size of a maximized window
		SDL_RestoreWindow(app->mSDLWindow);

	if (pSet->fullscreen)
	{
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(app->mSDLWindow, &mode);
		mode.w = pSet->windowx;
		mode.h = pSet->windowy;
		SDL_SetWindowDisplayMode(app->mSDLWindow, &mode);
		SDL_SetWindowFullscreen(app->mSDLWindow, 0);
		SDL_SetWindowFullscreen(app->mSDLWindow, SDL_WINDOW_FULLSCREEN);
	}else
		SDL_SetWindowSize(app->mSDLWindow, pSet->windowx, pSet->windowy);
}


//  get screen resolutions
struct ScrRes {  int w,h;  String mode;  };

bool ResSort(const ScrRes& r1, const ScrRes& r2)
{
	return (r1.w <= r2.w) && (r1.h <= r2.h);
}

void CGui::InitGuiScreenRes()
{
	ButtonPtr bchk;
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//  video resolutions combobox
	resList = app->mGui->findWidget<List>("ResList");
	if (resList)
	{
		//  get resolutions
		const StringVector& videoModes = Root::getSingleton().getRenderSystem()->getConfigOptions()["Video Mode"].possibleValues;
		String modeSel = "";
		std::vector<ScrRes> vRes;
		for (int i=0; i < videoModes.size(); i++)
		{
			String mode = videoModes[i];
			StringUtil::trim(mode);
			if (StringUtil::match(mode, "*16-bit*"))  continue;  //skip ?DX

			StringVector vmopts = StringUtil::split(mode, " x");  // only resolution
			int w = StringConverter::parseUnsignedInt(vmopts[0]);
			int h = StringConverter::parseUnsignedInt(vmopts[1]);
			
			if (w >= 800 && h >= 600)  // min res
			{
				mode = toStr(w) + " x " + toStr(h);
				ScrRes res;  res.w = w;  res.h = h;  res.mode = mode;
				vRes.push_back(res);
				int ww = w - app->mWindow->getWidth(), hh = h - app->mWindow->getHeight();
				if (abs(ww) < 30 && abs(hh) < 50)  // window difference
					modeSel = mode;
			}
		}
		//  sort and add
		std::sort(vRes.begin(), vRes.end(), ResSort);
		for (int r=0; r < vRes.size(); ++r)
			resList->addItem(vRes[r].mode);

		//  sel current mode
		if (modeSel != "")
			resList->setIndexSelected(resList->findItemIndexWith(modeSel));
	}
	ButtonPtr btnRes = app->mGui->findWidget<Button>("ResChange");
	if (btnRes)  {  btnRes->eventMouseButtonClick += newDelegate(this, &CGui::btnResChng);  }
}


//  resize Options wnd
void CGui::ResizeOptWnd()
{
	if (!app->mWndOpts)  return;

	const int wx = pSet->windowx, wy = pSet->windowy;
	const int yN = 7;
	const Real yw[yN] = {400.f, 600.f, 720.f, 768.f, 960.f, 1024.f, 1200.f};
	const Real yf[yN] = {0.0f,  0.0f,  0.05f, 0.1f,  0.2f,  0.3f,   0.3f};

	Real xm = 0.f, ym = 0.f;  // margin
	for (int i=0; i < yN; ++i)
		if (wy >= yw[i]-10)  ym = yf[i];

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	#ifndef SR_EDITOR  // game
	app->mWndGame->setCoord(xm, ym, xo, yo);
	app->mWndReplays->setCoord(xm, ym, xo, yo);
	//ap->mWndTweak->setCoord(0, 6, xo/3, yo-ym);
	#else  // editor
	app->mWndEdit->setCoord(xm, ym, xo, yo);
	#endif  // both
	app->mWndHelp->setCoord(xm, ym, xo, yo);
	app->mWndOpts->setCoord(xm, ym, xo, yo);

	if (bnQuit)  //  reposition Quit btn
		bnQuit->setCoord(wx - 0.09*wx, 0, 0.09*wx, 0.03*wy);

	updTrkListDim();
	#ifndef SR_EDITOR
	updChampListDim();  // resize lists
	#endif
}

void CGui::chkVidFullscr(WP wp)
{
	ChkEv(fullscreen);
	SDL_SetWindowFullscreen(app->mSDLWindow,  wp->castType<MyGUI::Button>()->getStateSelected()? SDL_WINDOW_FULLSCREEN : 0);
	#ifndef SR_EDITOR
	app->bSizeHUD = true;
	#endif
}

void CGui::chkVidVSync(WP wp)
{		
	ChkEv(vsync); 
	Ogre::Root::getSingleton().getRenderSystem()->setWaitForVerticalBlank(pSet->vsync);
}

void CGui::comboRenderSystem(ComboBoxPtr wp, size_t val)
{
	pSet->rendersystem = wp->getItemNameAt(val);
}
