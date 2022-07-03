#pragma once
#include "Gui_Def.h"
#include "SliderValue.h"

#include <MyGUI_Prerequest.h>
//#include <MyGUI_Types.h>
#include <MyGUI_WidgetToolTip.h>
#include <MyGUI_Enumerator.h>
#include <OgreString.h>
#include "settings_com.h"


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

	MyGUI::TabPtr FindSubTab(WP tab);  // util
	

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
	Ogre::String GetSceneryColor(Ogre::String name, Ogre::String* scenery=0);
	std::map<Ogre::String, Ogre::String> scnClr, scnN;


	///  [Track]

	//  selected track name, user
	Ogre::String sListTrack;
	bool bListTrackU;
	
	void listTrackChng(Mli2, size_t);
	void SortTrkList();
	void TrackListUpd(bool resetNotFound=false);

	Mli2 trkList;
	Ed trkDesc[2], trkAdvice[2];  // description, advice
	bool needSort(Mli2 li);

	Img imgPrv[3], imgMini[3], imgTer[3];  // view,  mini: road, terrain, [2] is fullscr
	Img imgMiniPos[3];  MyGUI::RotatingSkin* imgMiniRot[3];
	Img imgTrkIco1 =0, imgTrkIco2 =0;
	

	//  st - road stats,dim  inf - tracks.ini ratings
	const static int InfTrk = 13, ImStTrk = 4,
	#ifdef SR_EDITOR
		StTrk = 9;
	#else
		StTrk = 14;
	#endif
	Txt stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs
	Img imStTrk[2][ImStTrk], imInfTrk[2][InfTrk];

	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack,
		float timeCur, bool reverse=false, int champ=0);
	void ReadTrkStats();

	//  track views
	void btnTrkView1(WP), btnTrkView2(WP), btnTrkFilter(WP);
	void ChangeTrackView();
	void updTrkListDim();


	//  columns, filters  ---
	Ck ckTrkFilter;  void chkTrkFilter(Ck*);
	Ck ckTrkColVis[COL_VIS];  void chkTrkColVis(Ck*);
	SV svTrkFilMin[COL_FIL], svTrkFilMax[COL_FIL];  void slTrkFil(SV*);
	void ChkUpd_Col();
	Txt txtTracksFAll, txtTracksFCur;

	//  const list column widths
	const static int colTrk[33], iClrsDiff = 9, iClrsRating = 7, iClrsLong = 11, iClrsSum = 10;
	const static Ogre::String clrsDiff[iClrsDiff], clrsRating[iClrsRating], clrsLong[iClrsLong], clrsSum[iClrsSum];
	const static Ogre::String getClrDiff(int), getClrRating(int), getClrLong(int), getClrSum(int);


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

	//  util
	void OpenBrowserUrl(std::string url);
};
