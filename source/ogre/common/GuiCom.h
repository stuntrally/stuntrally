#pragma once
#include "Gui_Def.h"
#include "SliderValue.h"

#include <MyGUI_Prerequest.h>
//#include <MyGUI_Types.h>
#include <MyGUI_WidgetToolTip.h>
#include <MyGUI_Enumerator.h>
#include <OgreString.h>


namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
class Scene;  class CData;  class SplineRoad;
class App;  class SETTINGS;


//  tracks,cars list items - with info for sorting
struct TrkL
{
	std::string name;
	const class TrackInfo* ti;  // 0 means user trk
	static int idSort;
	TrkL();
};


class CGuiCom : public BGui
{
public:
	App* app;
	SETTINGS* pSet;
	Scene* sc;
	MyGUI::Gui* mGui;
	
	CGuiCom(App* app1);

	typedef std::list <std::string> strlist;


	///  Gui common   --------------------------

	//  resize
	void SizeGUI(), doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	float GetGuiMargin(int wy);
	void setOrigPos(WP wp, const char* relToWnd);
	void CreateFonts();

	//  tooltip
	WP mToolTip;  Ed mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(WP sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(WP moving, const MyGUI::IntPoint& point);

	//  language
	bool bGuiReinit;  // lang change
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>

	void UnfocusLists();
	Btn bnQuit;  void btnQuit(WP);

	//  init
	void GuiCenterMouse();
	void GuiInitTooltip(), GuiInitLang();
	//  init tabs
	void GuiInitGraphics(), GuiInitTrack(), initMiniPos(int i);

	//  main menu
	void InitMainMenu();
	void btnMainMenu(WP);  void tabMainMenu(Tab tab, size_t id);
	MyGUI::Tab* FindSubTab(WP tab);  // util
	

	///  [Graphics]
	SlV(ViewDist);  SlV(Anisotropy);
	SlV(TerDetail);  SlV(TerDist);  SV svRoadDist;
	SV svTexSize, svTerMtr, svTerTripl;  // detail
	SV svTrees, svGrass, svTreesDist, svGrassDist;  // veget
	SV svShadowType, svShadowCount, svShadowSize, svShadowDist;  // shadow
	SlV(WaterSize);

	Ck ckUseImposters, ckImpostorsOnly;
	Ck ckWaterReflect, ckWaterRefract;  void chkWater(Ck*);

	void comboTexFilter(CMB), cmbAntiAliasing(CMB);
	void btnShadows(WP), btnShaders(WP), btnTrGrReset(WP);
	
	Ck ckLimitFps;
	SV svLimitFps,svLimitSleep;
	

	//  track path 
	Ogre::String pathTrk[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	Ogre::String PathListTrk(int user=-1);
	Ogre::String PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	bool TrackExists(Ogre::String name);
	Ogre::String GetSceneryColor(Ogre::String name);
	std::map<Ogre::String, Ogre::String> scnClr, scnN;


	///  [Track]

	//  selected track name, user
	Ogre::String sListTrack;
	bool bListTrackU;
	
	void listTrackChng(Mli2, size_t);
	void SortTrkList();
	void TrackListUpd(bool resetNotFound=false);

	Mli2 trkList;
	Ed trkDesc[2];  // description
	bool needSort(Mli2 li);

	Img imgPrv[2], imgMini[2], imgTer[2];  // view,  mini: road, terrain
	Img imgTrkIco1, imgTrkIco2, imgMiniPos[2];
	MyGUI::RotatingSkin* imgMiniRot[2];

	//  st - road stats,dim  inf - tracks.ini ratings
	const static int StTrk = 14, InfTrk = 12;
	Txt stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs
	Img imStTrk[2][4], imInfTrk[2][InfTrk];

	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack,
		float timeCur, bool reverse=false, int champ=0);
	void ReadTrkStats();

	//  track views
	void btnTrkView1(WP), btnTrkView2(WP);
	void ChangeTrackView();
	void updTrkListDim();

	//  const list column widths
	const static int colTrk[32];
	const static Ogre::String clrsDiff[9], clrsRating[6], clrsLong[10];

	//  track find
	Ogre::String sTrkFind;
	Ed edTrkFind;  void editTrkFind(Ed); 

	//  list fill
	strlist liTracks,liTracksUser;
	std::list<TrkL> liTrk;
	void FillTrackLists();

	void AddTrkL(std::string name, int user, const class TrackInfo* ti);
	void trkListNext(int rel);


	///  [Screen]
	Cmb resList;
	void InitGuiScreenRes(), ResizeOptWnd();

	CK(VidFullscr);  CK(VidVSync);
	void btnResChng(WP);
	void comboGraphicsAll(CMB), comboRenderSystem(CMB);
		
};
