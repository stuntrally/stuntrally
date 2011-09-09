#include "pch.h"
#include "../Defines.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef ROAD_EDITOR
	#include "../../vdrift/game.h"
	#include "../OgreGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/OgreApp.h"
#endif
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Gui_Def.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init  [Graphics]
//----------------------------------------------------------------------------------------------------------------

//  textures
void App::comboTexFilter(SL)
{
	TextureFilterOptions tfo;							
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);  //if (bGI)
}

void App::slAnisotropy(SL)
{	if (bGI)  {
	MaterialManager::getSingleton().setDefaultAnisotropy(val);	pSet->anisotropy = val;  }
	if (valAnisotropy)	valAnisotropy->setCaption(toStr(val));
}

//  view dist
void App::slViewDist(SL)
{
	Real v = 50.f + 6950.f * powf(val/res, 2.f);
	Vector3 sc = v*Vector3::UNIT_SCALE;

	SceneNode* nskb = mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58);
	else  if (ndSky)  ndSky->setScale(sc);

	if (bGI)  pSet->view_distance = v;
	if (valViewDist){	Fmt(s, "%4.1f km", v*0.001f);	valViewDist->setCaption(s);  }

	// Set new far clip distance for all cams
	#ifndef ROAD_EDITOR
	/*?if (bGI)*/  mSplitMgr->UpdateCamDist();
	#endif
}

//  ter detail
void App::slTerDetail(SL)
{
	Real v = 20.f * powf(val/res, 2.f);  if (bGI)  {  pSet->terdetail = v;
		if (mTerrainGlobals)
			mTerrainGlobals->setMaxPixelError(v);  }
	if (valTerDetail){	Fmt(s, "%4.1f %%", v);	valTerDetail->setCaption(s);  }
}

//  ter dist
void App::slTerDist(SL)
{
	Real v = 2000.f * powf(val/res, 2.f);  if (bGI)  {  pSet->terdist = v;
		if (mTerrainGlobals)
			mTerrainGlobals->setCompositeMapDistance(v);  }
	if (valTerDist){	Fmt(s, "%4.0f m", v);	valTerDist->setCaption(s);  }
}

//  road dist
void App::slRoadDist(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->road_dist = v;
	if (valRoadDist){	Fmt(s, "%5.2f", v);	valRoadDist->setCaption(s);  }
}


//  trees/grass
void App::slTrees(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->trees = v;
	if (valTrees){	Fmt(s, "%4.2f", v);	valTrees->setCaption(s);  }
}
void App::slGrass(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->grass = v;
	if (valGrass){	Fmt(s, "%4.2f", v);	valGrass->setCaption(s);  }
}

void App::slTreesDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  if (bGI)  pSet->trees_dist = v;
	if (valTreesDist){	Fmt(s, "%4.2f", v);	valTreesDist->setCaption(s);  }
}
void App::slGrassDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  if (bGI)  pSet->grass_dist = v;
	if (valGrassDist){	Fmt(s, "%4.2f", v);	valGrassDist->setCaption(s);  }
}

void App::btnTrGrReset(WP wp)
{
	HScrollPtr sl;  size_t v;
	#define setSld(name)  sl##name(0,v);  \
		sl = (HScrollPtr)mWndOpts->findWidget(#name);  if (sl)  sl->setScrollPosition(v);
	v = res*powf(1.f /4.f, 0.5f);
	setSld(Trees);
	setSld(Grass);
	v = res*powf((1.f-0.5f) /6.5f, 0.5f);
	setSld(TreesDist);
	setSld(GrassDist);
}


void App::slShaders(SL)
{
	int v = val;  if (bGI)  pSet->shaders = v;
	if (valShaders)
	{	if (v == 0)  valShaders->setCaption("Vertex");  else
		if (v == 1)  valShaders->setCaption("Pixel");  else
		if (v == 2)  valShaders->setCaption("Metal");  }
}

void App::slTexSize(SL)
{
	int v = val;  if (bGI)  pSet->tex_size = v;
	if (valTexSize)
	{	if (v == 0)  valTexSize->setCaption("Small");  else
		if (v == 1)  valTexSize->setCaption("Big");  }
}

void App::slTerMtr(SL)
{
	int v = val;  if (bGI)  pSet->ter_mtr = v;
	if (valTerMtr)
	{	if (v == 0)  valTerMtr->setCaption("Lowest");  else
		if (v == 1)  valTerMtr->setCaption("Low");  else
		if (v == 2)  valTerMtr->setCaption("Normal");  else
		if (v == 3)  valTerMtr->setCaption("Parallax");  }
	if (bGI)  changeShadows();
}


//  shadows
void App::btnShadows(WP){	changeShadows();	}

void App::slShadowType(SL)
{
	int v = val;	if (bGI)  pSet->shadow_type = v;
	if (valShadowType)
	{	if (v == 0)  valShadowType->setCaption("None");  else
		if (v == 1)  valShadowType->setCaption("Old");  else
		if (v == 2)  valShadowType->setCaption("Normal");  else
		if (v == 3)  valShadowType->setCaption("Depth-");  }
}

void App::slShadowCount(SL)
{
	int v = 2 + 2.f * val/res;	if (bGI)  pSet->shadow_count = v;
	if (valShadowCount)  valShadowCount->setCaption(toStr(v));
}

void App::slShadowSize(SL)
{
	int v = std::max( 0.0f, std::min((float) ciShadowNumSizes-1, ciShadowNumSizes * val/res));
	if (bGI)  pSet->shadow_size = v;
	if (valShadowSize)  valShadowSize->setCaption(toStr(ciShadowSizesA[v]));
}

void App::slShadowDist(SL)
{
	Real v = 50.f + 4750.f * powf(val/res, 2.f);	if (bGI)  pSet->shadow_dist = v;
	if (valShadowDist){  Fmt(s, "%4.1f km", v*0.001f);	valShadowDist->setCaption(s);  }
}


//  init  common
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitGraphics()
{
	ButtonPtr btn, bchk;  ComboBoxPtr combo;
	HScrollPtr sl;  size_t v;

	//  detail
	Slv(TerDetail,	powf(pSet->terdetail /20.f, 0.5f));
	Slv(TerDist,	powf(pSet->terdist /2000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/6950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	Cmb(combo, "TexFiltering", comboTexFilter);
	Slv(Anisotropy,	pSet->anisotropy /res);
	Slv(Shaders,	pSet->shaders /res);
	Slv(TexSize,	pSet->tex_size /res);
	Slv(TerMtr,		pSet->ter_mtr /res);

	//  trees/grass
	Slv(Trees,		powf(pSet->trees /4.f, 0.5f));
	Slv(Grass,		powf(pSet->grass /4.f, 0.5f));
	Slv(TreesDist,	powf((pSet->trees_dist-0.5f) /6.5f, 0.5f));
	Slv(GrassDist,	powf((pSet->grass_dist-0.5f) /6.5f, 0.5f));
	Btn("TrGrReset", btnTrGrReset);

	//  shadows
	Slv(ShadowType,	pSet->shadow_type /res);
	Slv(ShadowCount,(pSet->shadow_count-2) /2.f);
	Slv(ShadowSize,	pSet->shadow_size /float(ciShadowNumSizes));
	Slv(ShadowDist,	powf((pSet->shadow_dist -50.f)/4750.f, 0.5f));
	Btn("Apply", btnShadows);
	
	Cmb(combo, "CmbGraphicsAll", comboGraphicsAll);
	if (combo)  {
		combo->addItem(TR("#{GraphicsAll_Lowest}"));
		combo->addItem(TR("#{GraphicsAll_Low}"));
		combo->addItem(TR("#{GraphicsAll_Medium}"));
		combo->addItem(TR("#{GraphicsAll_High}"));
		combo->addItem(TR("#{GraphicsAll_Ultra}"));
    }
}


//  util
//----------------------------------------------------------------------------------------------------------------
void App::GuiCenterMouse()
{
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;
}

void App::btnQuit(WP)
{
	mShutDown = true;
}


///  Tooltips
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitTooltip()
{
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
}

void App::setToolTips(EnumeratorWidgetPtr widgets)
{
    while (widgets.next())
    {
        WidgetPtr wp = widgets.current();
		wp->setAlign(Align::Relative);
        bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{	
			// needed for translation
			wp->setUserString("tip", LanguageManager::getInstance().replaceTags(wp->getUserString("tip")));
			wp->setNeedToolTip(true);
			wp->eventToolTip = newDelegate(this, &App::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
        setToolTips(wp->getEnumerator());
    }
}

void App::notifyToolTip(Widget *sender, const ToolTipInfo &info)
{
	if (!mToolTip)  return;

	#ifndef ROAD_EDITOR
	if (!isFocGui)
	{	mToolTip->setVisible(false);
		return;  }
	#endif

	if (info.type == ToolTipInfo::Show)
	{	// TODO: Tooltip isn't resizing properly ..
		mToolTip->setSize(320, 96);  // start size for wrap
		String s = TR(sender->getUserString("tip"));
		mToolTipTxt->setCaption(s);
		const IntSize &textsize = mToolTipTxt->getTextSize();
		mToolTip->setSize(textsize.width*1.5, textsize.height*1.5);
		mToolTip->setVisible(true);
		boundedMove(mToolTip, info.point);
	}
	else if (info.type == ToolTipInfo::Hide)
		mToolTip->setVisible(false);
}

//  Move a widget to a point while making it stay in the viewport.
void App::boundedMove(Widget* moving, const IntPoint& point)
{
	const IntPoint offset(20, 20);  // mouse cursor
	IntPoint p = point + offset;

	const IntSize& size = moving->getSize();
	
	int vpw = mWindow->getWidth();
	int vph = mWindow->getHeight();
	
	if (p.left + size.width > vpw)
		p.left = vpw - size.width;
	if (p.top + size.height > vph)
		p.top = vph - size.height;
			
	moving->setPosition(p);
}


//  Languages combo
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitLang()
{
	languages["en"] = "English";
	languages["de"] = "Deutsch";  //German
	languages["fi"] = "Suomi";   //Finnish
	languages["ro"] = "Romana";  //Romanian â?
	languages["pl"] = "Polski";  //Polish
	
	//ComboBoxPtr combo = mGUI->findWidget<ComboBox>("Lang");
	ComboBoxPtr combo = (ComboBoxPtr)mWndOpts->findWidget("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition = newDelegate(this, &App::comboLanguage);
	for (std::map<std::string, std::string>::const_iterator it = languages.begin();
		it != languages.end(); it++)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void App::comboLanguage(SL)
{
	if (val == MyGUI::ITEM_NONE)  return;
	MyGUI::ComboBoxPtr cmb = static_cast<MyGUI::ComboBoxPtr>(wp);
	std::string sel = cmb->getItemNameAt(val);
	
	for (std::map<std::string, std::string>::const_iterator it = languages.begin();
		it != languages.end(); it++)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);

	//  reinit gui
	bGuiReinit = true;
	
	#ifndef ROAD_EDITOR
	setTranslations();
	#endif
}


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//----------------------------------------------------------------------------------------------------------------

void App::ReadTrkStats()
{
	String sRd = PathListTrk() + "/road.xml";
	String sSc = PathListTrk() + "/scene.xml";

	Scene sc;  sc.LoadXml(sSc);  // fails to defaults
#ifndef ROAD_EDITOR  // game
	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load
	TIMER tim;  tim.Load(PATHMANAGER::GetTrackRecordsPath()+"/"+sListTrack+".txt", 0.f, pGame->error_output);
	tim.AddCar(sListCar);  tim.SetPlayerCarID(0);
	UpdGuiRdStats(&rd,sc, tim.GetBestLap(pSet->trackreverse));
#else
	SplineRoad rd;  rd.LoadFile(sRd,false);  // load
	UpdGuiRdStats(&rd,sc, 0.f);
#endif
}

void App::UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time)
{
	Fmt(s, "%5.3f km", sc.td.fTerWorldSize / 1000.f);	if (stTrk[1])  stTrk[1]->setCaption(s);
	if (!rd)  return;
	Fmt(s, "%5.3f km", rd->st.Length / 1000.f);			if (stTrk[0])  stTrk[0]->setCaption(s);

	Fmt(s, "%4.2f m", rd->st.WidthAvg);		if (stTrk[2])  stTrk[2]->setCaption(s);
	Fmt(s, "%3.1f m", rd->st.HeightDiff);	if (stTrk[3])  stTrk[3]->setCaption(s);

	Fmt(s, "%3.1f%%", rd->st.OnTer);	if (stTrk[4])  stTrk[4]->setCaption(s);
	Fmt(s, "%3.1f%%", rd->st.Pipes);	if (stTrk[5])  stTrk[5]->setCaption(s);
					
	//Fmt(s, "%4.2f%%", rd->st.Yaw);	if (stTrk[6])  stTrk[6]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[7])  stTrk[7]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Roll);	if (stTrk[8])  stTrk[8]->setCaption(s);
	
	int id = tracksXml.trkmap[sListTrack];
	for (int i=0; i < InfTrk; ++i)
		if (infTrk[i])  infTrk[i]->setCaption("");
	if (id > 0)
	{	const TrackInfo& ti = tracksXml.trks[id-1];
		#define str0(v)  ((v)==0 ? "" : toStr(v))
		if (infTrk[0])  infTrk[0]->setCaption(str0(ti.bumps));		if (infTrk[1])  infTrk[1]->setCaption(str0(ti.jumps));
		if (infTrk[2])  infTrk[2]->setCaption(str0(ti.loops));		if (infTrk[3])  infTrk[3]->setCaption(str0(ti.pipes));
		if (infTrk[4])  infTrk[4]->setCaption(str0(ti.banked));		if (infTrk[5])  infTrk[5]->setCaption(str0(ti.frenzy));
		if (infTrk[6])  infTrk[6]->setCaption(str0(ti.longn));
		if (infTrk[7])  infTrk[7]->setCaption(toStr(ti.diff));		if (infTrk[8])  infTrk[8]->setCaption(toStr(ti.rating));
	}

#ifndef ROAD_EDITOR  // game
	//  best time, avg vel,
	if (time < 0.1f)
	{	Fmt(s, "%s", GetTimeString(0.f).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "0 mph");
		else				Fmt(s, "0 km/h");		if (stTrk[7])  stTrk[7]->setCaption(s);
	}else
	{	Fmt(s, "%s", GetTimeString(time).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "%4.1f mph", rd->st.Length / time * 2.23693629f);
		else				Fmt(s, "%4.1f km/h", rd->st.Length / time * 3.6f);
		if (stTrk[7])  stTrk[7]->setCaption(s);
		//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[8])  stTrk[8]->setCaption(s);
	}
#else
	if (trkName)  //?.
		trkName->setCaption(sListTrack.c_str());
#endif
	if (trkDesc)  // desc
		trkDesc->setCaption(rd->sTxtDesc.c_str());
}


//  track  . . . . . . . . . . . . . . . . . . . . . . . . . 
void App::listTrackChng(List* li, size_t pos)
{
	if (!li)  return;
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;

	const UString& sl = li->getItemNameAt(i);  String s = sl;
	s = StringUtil::replaceAll(s, "*", "");
	sListTrack = s;

	int u = *li->getItemDataAt<int>(i,false);
	bListTrackU = u;
	
	//  won't refresh if same-...  road dissapears if not found...
	if (imgPrv)  imgPrv->setImageTexture(sListTrack+".jpg");
	if (imgTer)  imgTer->setImageTexture(sListTrack+"_ter.jpg");
	if (imgMini)  imgMini->setImageTexture(sListTrack+"_mini.png");
	ReadTrkStats();
}


//  tracks list	 . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------
struct TrkL  {  std::string name;  const TrackInfo* ti;  };

bool TrkSort(const TrkL& t1, const TrkL& t2)
{
	//if (!t1.ti || !t2.ti)
	//	return t1.name < t2.name;  // no info only name
	//else
	//	return (t1.ti->n) < (t2.ti->n);
	int n1 = !t1.ti ? 1000 : t1.ti->n;
	int n2 = !t2.ti ? 1000 : t2.ti->n;
	return n1 < n2;
}

void App::TrackListUpd()
{
	if (trkList)
	{	trkList->removeAllItems();
		#ifdef ROAD_EDITOR
		vsTracks.clear();  vbTracksUser.clear();
		std::string chkfile = "/scene.xml";
		#else
		std::string chkfile = "/track.txt";
		#endif
		int ii = 0, si = 0;  bool bFound = false;

		strlist li,lu;
		PATHMANAGER::GetFolderIndex(pathTrk[0], li);
		PATHMANAGER::GetFolderIndex(pathTrk[1], lu);  //name duplicates

		//  sort
		std::list<TrkL> liTrk;
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			TrkL trl;  trl.name = *i;
			//  get info for track name from tracksXml
			int id = tracksXml.trkmap[*i];
			const TrackInfo* pTrk = id==0 ? 0 : &tracksXml.trks[id-1];
			trl.ti = pTrk;  // 0 if not in tracksXml
			liTrk.push_back(trl);
		}
		liTrk.sort(TrkSort);
		
		//  original
		//for (strlist::iterator i = li.begin(); i != li.end(); ++i)  //no sort-
		//{	const std::string& name = *i;
		for (std::list<TrkL>::iterator i = liTrk.begin(); i != liTrk.end(); ++i)
		{	const std::string& name = (*i).name;
			#ifdef ROAD_EDITOR
			vsTracks.push_back(name);  vbTracksUser.push_back(false);
			#endif
			std::string s = pathTrk[0] + name + chkfile;
			std::ifstream check(s.c_str());
			if (check)  {
				std::string liName = /*((*i).ti ? toStr((*i).ti->n) : "==") +" "+ */name;
				trkList->addItem(liName, 0);
				if (!pSet->track_user && name == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 0;  }
				ii++;  }
		}
		//  user
		for (strlist::iterator i = lu.begin(); i != lu.end(); ++i)
		{
			#ifdef ROAD_EDITOR
			vsTracks.push_back(*i);  vbTracksUser.push_back(true);
			#endif
			std::string s = pathTrk[1] + *i + chkfile;
			std::ifstream check(s.c_str());
			if (check)  {
				trkList->addItem("*" + (*i) + "*", 1);
				if (pSet->track_user && *i == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 1;  }
				ii++;  }
		}
		//  not found last track, set 1st
		if (!bFound)
		{	pSet->track = *li.begin();  pSet->track_user = 0;
			#ifdef ROAD_EDITOR
			UpdWndTitle();
			#endif
		}
		trkList->beginToItemAt(std::max(0, si-11));  // center
	}
}


//  Gui Init  [Track]  . . . . . . . . . . . . . . . . . . . 
void App::GuiInitTrack()
{
	//  list
    trkList = (ListPtr)mWndOpts->findWidget("TrackList");
    TrackListUpd();
    if (trkList)
    	trkList->eventListChangePosition = newDelegate(this, &App::listTrackChng);
		//?trkList->eventMouseButtonDoubleClick = newDelegate(this, &App::btnNewGameStart);
	
	//  preview images
	imgPrv = (StaticImagePtr)mWndOpts->findWidget("TrackImg");
	imgTer = (StaticImagePtr)mWndOpts->findWidget("TrkTerImg");
	imgMini = (StaticImagePtr)mWndOpts->findWidget("TrackMap");
	//  stats text
	for (int i=0; i < StTrk; ++i)
		stTrk[i] = (StaticTextPtr)mWndOpts->findWidget("iv"+toStr(i+1));
	for (int i=0; i < InfTrk; ++i)
		infTrk[i] = (StaticTextPtr)mWndOpts->findWidget("ti"+toStr(i+1));
	listTrackChng(trkList,0);
}


//  [Screen] resolutions
//-----------------------------------------------------------------------------------------------------------

//  change
void App::btnResChng(WP)
{
	if (!resList)  return;
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE) return;
	String mode = resList->getItem(resList->getIndexSelected());

	pSet->windowx = StringConverter::parseInt(StringUtil::split(mode, "x")[0]);
	pSet->windowy = StringConverter::parseInt(StringUtil::split(mode, "x")[1]);
	
	mWindow->resize(pSet->windowx, pSet->windowy);
	
	if (pSet->fullscreen)
		mWindow->setFullscreen(true, pSet->windowx, pSet->windowy);
	else
	{
	#ifdef _WIN32
		int sx = GetSystemMetrics(SM_CXSCREEN), sy = GetSystemMetrics(SM_CYSCREEN);
		int cx = std::max(0,(sx - pSet->windowx) / 2), cy = std::max(0,(sy - pSet->windowy) / 2);
		mWindow->reposition(cx,cy);
	#else
		//mWindow->reposition(0,0);  //TODO: linux window size,center ?..
	#endif
	}
	bWindowResized = true;
}


//  get screen resolutions
struct ScrRes {  int w,h;  String mode;  };

bool ResSort(const ScrRes& r1, const ScrRes& r2)
{
	return (r1.w <= r2.w) && (r1.h <= r2.h);
}

void App::InitGuiScrenRes()
{
	ButtonPtr bchk;
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//  video resolutions combobox
	resList = (ListPtr)mWndOpts->findWidget("ResList");
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
				int ww = w - mWindow->getWidth(), hh = h - mWindow->getHeight();
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
	ButtonPtr btnRes = (ButtonPtr)mWndOpts->findWidget("ResChange");
	if (btnRes)  {  btnRes->eventMouseButtonClick = newDelegate(this, &App::btnResChng);  }
}


//  resize Options wnd
void App::ResizeOptWnd()
{
	if (!mWndOpts)  return;

	const int wx = pSet->windowx, wy = pSet->windowy;
	const int yN = 7;
	const Real yw[yN] = {400, 600, 720, 768, 960, 1024, 1200};
	const Real yf[yN] = {0.0, 0.0, 0.05, 0.1, 0.2, 0.3,  0.3};

	Real xm = 0.f, ym = 0.f;  // margin
	for (int i=0; i < yN; ++i)
		if (wy >= yw[i]-10)  ym = yf[i];

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	mWndOpts->setCoord(xm, ym, xo, yo);
	if (bnQuit)  //  reposition Quit btn
		bnQuit->setCoord(wx - 0.09*wx, 0, 0.09*wx, 0.03*wy);
}

void App::chkVidFullscr(WP wp)
{
	ChkEv(fullscreen);  //TODO: got broken, crashes on win ..
	mWindow->setFullscreen(pSet->fullscreen, pSet->windowx, pSet->windowy);
	mWindow->resize(pSet->windowx, pSet->windowy);
}

void App::chkVidVSync(WP wp)
{		
	ChkEv(vsync); 
	Ogre::Root::getSingleton().getRenderSystem()->setWaitForVerticalBlank(pSet->vsync);
}


///  change all gui settings  -------------------------------------
void App::comboGraphicsAll(ComboBoxPtr cmb, size_t val)
{
	//String s = cmb->getItemNameAt(val);
	//String n = cmb->getName();
	switch (val)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	}
}
