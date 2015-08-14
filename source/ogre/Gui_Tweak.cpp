#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/data/CData.h"
#include "common/data/BltObjects.h"
#include "common/CScene.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "CGui.h"
#include "CGame.h"
#include "CarModel.h"
#include <MyGUI_Window.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_TabItem.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ComboBox.h>
#include <boost/filesystem.hpp>
using namespace std;
using namespace Ogre;
using namespace MyGUI;
namespace fs = boost::filesystem;


///  Tweak Car
//-----------------------------------------------------------------------------------------------------------

void CGui::TweakCarSave()
{
	String text = "";
	for (int i=0; i < ciEdCar; ++i)  // sum all edits
		text += edCar[i]->getCaption();
	if (text == "")  return;
	
	text = StringUtil::replaceAll(text, "##", "#");
	text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0]);
	
	PATHMANAGER::CreateDir(pathUserDir);
	std::ofstream fo(pathUser.c_str());
	fo << text.c_str();
	fo.close();
	
	app->NewGame();
}

//  fill gui with .car sections
void CGui::TweakCarLoad()
{
	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0]);

	if (!PATHMANAGER::FileExists(path))
	{
		for (int i=0; i < ciEdCar; ++i)
			edCar[i]->setCaption("");
		txtTweakPath->setCaption("Not Found ! " + path);
		txtTweakPath->setColour(Colour(1,0,0));
	}else
	{
		std::ifstream fi(path.c_str());
		const int iSecNum = 11;
		const static String sSecNames[iSecNum] = {
			"collision", "engine", "transmission", "suspension",
			"tire", "brakes", " drag", "wheel-F", "wheel-R", "particle-0", "aaa"};

		String s;  std::vector<String> lines;
		int secLn[ciEdCar];
		for (int i=0; i < ciEdCar; ++i)  secLn[i]=0;

		int l=0, sec=0, sec0=0, lastEmp = 0;
		while (getline(fi,s))
		{
			s += "\n";
			s = StringUtil::replaceAll(s, "#", "##");
			s = StringUtil::replaceAll(s, "#E5F4FF", "");  //clr!-

			//  split to car edit sections
			bool emp = s == "\n";
			if (emp)  {  lastEmp = l;  secLn[sec] = l;  }

			//  check section name
			bool found = false;  int sn=sec0;
			if (!emp && s[0] == '[')
			while (!found && sn < iSecNum)
			{
				if (sn == 1 && s.find("hover") != std::string::npos)
				{	found = true;  ++sec;  ++sec0;  }
				else
				if (sn == 2 && s.find("hover_h") != std::string::npos)
				{	found = true;  ++sec;  ++sec0;  }
				else
				if (s.find(sSecNames[sn]) != std::string::npos)
				{	found = true;  ++sec;  ++sec0;  }
				++sn;
			}
			if (s.find("torque-val-mul") != std::string::npos)
			{	
				lastEmp = l;  secLn[sec] = l;  ++sec;
			}
			
			lines.push_back(s);  ++l;
		}
		fi.close();

		for (int i=0; i < ciEdCar; ++i)
			edCar[i]->setCaption("");

		sec = 0;  s = "";
		for (l=0; l < lines.size(); ++l)
		{
			//s += lines[l];
			//  next sec or last line
			if (l==lines.size()-1 || l >= secLn[sec])
			{
				//edCar[sec]->setCaption(edCar[sec]->getCaption() + UString(s));
				//s="";
				if (sec < ciEdCar-1)  ++sec;
			}
			edCar[sec]->setCaption(edCar[sec]->getCaption() + UString(lines[l]));
		}
		
		//edTweak->setCaption(UString(text));
		//edCar[sec]->getVScrollPosition(0);
		//void setTextCursor(size_t _index);
		/** Get text cursor position */
		//size_t getTextCursor() const;

		size_t p = path.find("carsim");
		if (p != string::npos)
			path = path.substr(p+7, path.length());
		txtTweakPath->setCaption(TR(user ? "#{TweakUser}: " : "#{TweakOriginal}: ") + path);
		txtTweakPath->setTextColour(user ? Colour(1,1,0.5) : Colour(0.5,1,1));
		
		//MyGUI::InputManager::getInstance().resetKeyFocusWidget();
		//MyGUI::InputManager::getInstance().setKeyFocusWidget(edTweak);
	}
}


//  buttons, events
//-----------------------------------------------------------------------------------------------------------

void CGui::btnTweakCarSave(WP){		TweakCarSave();  }
void CGui::btnTweakCarLoad(WP){		TweakCarLoad();  }
void CGui::btnTweakTireSave(WP){	TweakTireSave();  }

void CGui::editTweakTireSet(Ed ed)
{
	if (txtTweakTire)
		txtTweakTire->setCaption("");
}														

void CGui::listTwkTiresUser(Li li, size_t id)
{
	if (id==ITEM_NONE || li->getItemCount() == 0)  return;
	pGame->PickTireRef(li->getItemNameAt(id).substr(7));
	liTwkTiresOrig->setIndexSelected(ITEM_NONE);
}
void CGui::listTwkTiresOrig(Li li, size_t id)
{
	if (id==ITEM_NONE || li->getItemCount() == 0)  return;
	pGame->PickTireRef(li->getItemNameAt(id).substr(7));
	liTwkTiresUser->setIndexSelected(ITEM_NONE);
}

void CGui::btnTweakTireDelete(WP)
{
	if (liTwkTiresUser->getItemCount() == 0)  return;
	size_t id = liTwkTiresUser->getIndexSelected();
	if (id==ITEM_NONE)  return;

	string name = liTwkTiresUser->getItemNameAt(id).substr(7);
	string path = PATHMANAGER::CarSimU() + "/" + pSet->game.sim_mode + "/tires/" + name + ".tire";

	if (PATHMANAGER::FileExists(path))
	{	fs::remove(path);
		txtTweakTire->setCaption(TR("#FF8080#{RplDelete}: "+name));
		pGame->reloadSimNeed = true;  // to remove from list
	}
}

//  Load Tire
void CGui::btnTweakTireLoad(WP)
{
	if (app->carModels.size() < 1)  return;
	CAR* pCar = app->carModels[0]->pCar;
	if (!pCar)  return;

	//  load as current, from wheel
	CARTIRE* tire = pCar->dynamics.GetTire(FRONT_LEFT);
	if (!tire)  return;

	string s, st = tire->name;
	size_t id = liTwkTiresUser->getIndexSelected();
	if (id != ITEM_NONE)  // user
		s = liTwkTiresUser->getItemNameAt(id).substr(7);
	else
	{	id = liTwkTiresOrig->getIndexSelected();
		if (id != ITEM_NONE)
			s = liTwkTiresOrig->getItemNameAt(id).substr(7);
	}
	if (!s.empty())
	{
		int ti = pGame->tires_map[s]-1;  if (ti == -1)  return;
		*tire = pGame->tires[ti];  // set pars
		tire->CalculateSigmaHatAlphaHat();
		
		if (!sTireLoad.empty())
			sTireLoad = "";
		else
			txtTweakTire->setCaption(TR("#FFFF30#{Loaded}: "+s+" into "+st));
		return;
	}
}

void CGui::chkTEupd(Ck*)
{
	chkGraphs(0);
}


void CGui::FillTweakLists()
{
	//  clear
	liTwkTiresUser->removeAllItems();
	liTwkTiresOrig->removeAllItems();
	cmbSurfTire->removeAllItems();
	liTwkSurfaces->removeAllItems();

	//  tires
	for (int i=0; i < pGame->tires.size(); ++i)
	{
		const CARTIRE& ct = pGame->tires[i];
		if (ct.user)
		{	liTwkTiresUser->addItem("#C0F0F0"+ct.name);
			if (ct.name == sTireLoad)  liTwkTiresUser->setIndexSelected(liTwkTiresUser->getItemCount()-1);
		}else
		{	liTwkTiresOrig->addItem("#A0D0F0"+ct.name);
			if (ct.name == sTireLoad)  liTwkTiresOrig->setIndexSelected(liTwkTiresUser->getItemCount()-1);
		}
		cmbSurfTire->addItem(ct.name);
	}
	//  surf
	for (int i=0; i < pGame->surfaces.size(); ++i)
	{
		const TRACKSURFACE& su = pGame->surfaces[i];
		liTwkSurfaces->addItem("#C0C0F0"+su.name);
	}
}

//  Surfaces
//-----------------------------------------------------------------------------------------------------------
void CGui::listTwkSurfaces(Li, size_t id)
{
	if (id == ITEM_NONE)  return;
	updSld_TwkSurf(id);
}

void CGui::btnTwkSurfPick(WP)
{
	if (app->carModels.size() < 1)  return;
	CAR* pCar = app->carModels[0]->pCar;
	if (!pCar)  return;

	CARDYNAMICS& cd = pCar->dynamics;
	const TRACKSURFACE& tsu = cd.GetWheelContact(FRONT_LEFT).GetSurface();
	int id=-1;  // find in game, not const
	for (size_t i=0; i < pGame->surfaces.size(); ++i)
		if (pGame->surfaces[i] == tsu)  id = i;
	if (id==-1)  return;
	updSld_TwkSurf(id);
}

void CGui::updSld_TwkSurf(int id)
{
	if (id < 0 || id >= pGame->surfaces.size())  return;
	idTwkSurf = id;
	
	TRACKSURFACE* su = &pGame->surfaces[id];
	svSuFrict.UpdF(&su->friction);  svSuFrictX.UpdF(&su->frictionX);  svSuFrictY.UpdF(&su->frictionY);
	svSuBumpWave.UpdF(&su->bumpWaveLength);  svSuBumpAmp.UpdF(&su->bumpAmplitude);
	svSuBumpWave2.UpdF(&su->bumpWaveLength2);  svSuBumpAmp2.UpdF(&su->bumpAmplitude2);
	svSuRollDrag.UpdF(&su->rollingDrag);  svSuRollRes.UpdF(&su->rollingResist);
	//cmbSurfTire
	//cmbSurfType->setIndexSelected(su->type);
}

void CGui::comboSurfTire(Cmb cmb, size_t val)
{
	if (idTwkSurf==-1)  return;
	//  find tire for name
	string s = cmb->getItemNameAt(val);
	s = s.substr(7);
	int id = pGame->tires_map[s]-1;
	if (id == -1)  return;
	pGame->surfaces[idTwkSurf].tire = &pGame->tires[id];
}

void CGui::comboSurfType(Cmb cmb, size_t val)
{
	if (idTwkSurf==-1)  return;
	pGame->surfaces[idTwkSurf].type = TRACKSURFACE::TYPE(val);
}


//  collisions
//-----------------------------------------------------------------------------------------------------------

void CGui::TweakColSave()
{
	String text = edTweakCol->getCaption();
	if (text == "")  return;
	text = StringUtil::replaceAll(text, "##", "#");
	//text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!

	std::string path = PATHMANAGER::DataUser() + "/trees";
	PATHMANAGER::CreateDir(path);
	path += "/collisions.xml";
	std::ofstream fo(path.c_str());
	fo << text.c_str();
	fo.close();
	TweakColUpd(true);
	
	app->scn->data->objs->LoadXml();
	LogO(String("**** Loaded Vegetation objects: ") + toStr(app->scn->data->objs->colsMap.size()));
	app->NewGame();
}

void CGui::TweakColUpd(bool user)
{
	txtTweakPathCol->setCaption(TR(user ? "#{TweakUser}" : "#{TweakOriginal}"));
	txtTweakPathCol->setTextColour(user ? Colour(1,1,0.5) : Colour(0.5,1,1));
}

void CGui::TweakColLoad()
{
	bool user = true;
	std::string name = "/trees/collisions.xml",  // user
		file = PATHMANAGER::DataUser() + name;
	if (!PATHMANAGER::FileExists(file))  // original
	{	file = PATHMANAGER::Data() + name;  user = false;  }

	std::ifstream fi(file.c_str());
	String text = "", s;
	while (getline(fi,s))
		text += s + "\n";
	fi.close();

	text = StringUtil::replaceAll(text, "#", "##");
	//text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!
	edTweakCol->setCaption(UString(text));
	
	TweakColUpd(user);
		
	MyGUI::InputManager::getInstance().resetKeyFocusWidget();
	MyGUI::InputManager::getInstance().setKeyFocusWidget(edTweakCol);
}

void CGui::btnTweakColSave(WP){	TweakColSave();  }


///  Tweak read / save file
//-----------------------------------------------------------------------------------------------------------
void CGui::TweakToggle()
{
	//  window
	bool vis = !app->mWndTweak->getVisible();
	app->mWndTweak->setVisible(vis);

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0]);
	
	//  load  if car changed
	static string lastPath = "";
	if (lastPath != path || app->ctrl)  // force reload  ctrl-alt-Z
	{	lastPath = path;
	
		TweakCarLoad();
		TweakColLoad();
		FillTweakLists();
	}
	
	//  save and reload  shift-alt-Z
	if (!vis && app->shift)
	if (tabTweak && tabTweak->getIndexSelected() < 2)
		TweakCarSave();
	else
		TweakColSave();
}

void CGui::tabCarEdChng(Tab, size_t id)
{
	pSet->car_ed_tab = id;
}
void CGui::tabTweakChng(Tab, size_t id)
{
	pSet->tweak_tab = id;
}


//  Get car file path
bool CGui::GetCarPath(std::string* pathCar,
	std::string* pathSave, std::string* pathSaveDir,
	std::string carname, bool forceOrig)
{
	std::string file = carname + ".car",
		pathOrig  = PATHMANAGER::CarSim()  + "/" + pSet->game.sim_mode + "/cars/" + file,
		pathUserD = PATHMANAGER::CarSimU() + "/" + pSet->game.sim_mode + "/cars/",
		pathUser  = pathUserD + file;

	if (pathSave)  *pathSave = pathUser;
	if (pathSaveDir)  *pathSaveDir = pathUserD;
	
	if (!forceOrig && PATHMANAGER::FileExists(pathUser))
	{
		*pathCar = pathUser;
		return true;
	}
	*pathCar = pathOrig;
	return false;
}


//  Tire edit const
//----------------------------------------------------------------------------------------------------------------------
const String CGui::csLateral[15][2] = {
	"  a0","#F0FFFFShape factor",
	"  a1","#C0E0FFLoad infl. on friction coeff",
	"  a2","#F0FFFFLateral friction coeff at load = 0",
	"  a3","#F0FFFFMaximum stiffness",
	"  a4","#F0FFFFLoad at maximum stiffness",
	"  a5","#C0E0FF-Camber infl. on stiffness",
	"  a6","Curvature change with load",
	"  a7","Curvature at load = 0",
	"  a8","#A0C0D0  -Horiz. shift because of camber",
	"  a9","  Load infl. on horizontal shift",
	" a10","  Horizontal shift at load = 0",
	"a111","  -Camber infl. on vertical shift",
	"a112","  -Camber infl. on vertical shift",
	" a12","  Load infl. on vertical shift",
	" a13","  Vertical shift at load = 0" };
const String CGui::csLongit[13][2] = {
	"  b0","#FFFFF0Shape factor",
	"  b1","#F0F0A0Load infl. on long. friction coeff",
	"  b2","#FFFFF0Longit. friction coeff at load = 0",
	"  b3","#F0F0A0Curvature factor of stiffness",
	"  b4","#F0F0A0Change of stiffness with load at load = 0",
	"  b5","#E0C080Change of progressivity/load",  //of stiffness
	"  b6","Curvature change with load^2",
	"  b7","Curvature change with load",
	"  b8","Curvature at load = 0",
	"  b9","#D0D0A0  Load infl. on horizontal shift",
	" b10","  Horizontal shift at load = 0",
	" b11","  Load infl. on vertical shift",
	" b12","  Vertical shift at load = 0" };
const String CGui::csAlign[18][2] = {
	" c0","#E0FFE0Shape factor",
	" c1","Load infl. of peak value",
	" c2","Load infl. of peak value",
	" c3","Curvature factor of stiffness",
	" c4","Change of stiffness with load at load = 0",
	" c5","Change of progressivity/load",
	" c6","-Camber infl. on stiffness",
	" c7","Curvature change with load",
	" c8","Curvature change with load",
	" c9","Curvature at load = 0",
	"c10","-Camber infl. of stiffness",
	"c11","  -Camber infl. on horizontal shift",
	"c12","  Load infl. on horizontal shift",
	"c13","  Horizontal shift at load = 0",
	"c14","  -Camber infl. on vertical shift",
	"c15","  -Camber infl. on vertical shift",
	"c16","  Load infl. on vertical shift",
	"c17","  Vertical shift at load = 0" };
const String CGui::sCommon = "#C8C8F0Pacejka's Magic Formula coeffs\n";


//  Save Tire
void CGui::TweakTireSave()
{
	//Nope todos: sliders for vals=
	// jump to section-, help on current line=
	// ed find text? syntax clr?=

	txtTweakTire->setCaption("");
	if (app->carModels.size() < 1)  return;
	CAR* pCar = app->carModels[0]->pCar;
	if (!pCar)  return;

	const CARTIRE* tire = app->carModels[0]->pCar->dynamics.GetTire(FRONT_LEFT);
	if (!tire)  return;
	const std::vector <Dbl>& a = tire->lateral, b = tire->longitudinal, c = tire->aligning;
	
	string name = edTweakTireSet->getCaption();
	string pathUserT = PATHMANAGER::CarSimU() + "/" + pSet->game.sim_mode + "/tires/";
	PATHMANAGER::CreateDir(pathUserT);
	string file = pathUserT+"/"+name+".tire";
	if (PATHMANAGER::FileExists(file))
	{
		txtTweakTire->setCaption(TR("#FF3030#{AlreadyExists}."));
		return;
	}

	ofstream fo(file.c_str());  int i=0;

	fo << "[ params ]\n";
	fo << "#--------	Lateral force\n";  i = 0;
	fo << "a0="<< a[i++] << "	# Shape factor  A0\n";
	fo << "a1="<< a[i++] << "	# Load infl. on lat. friction coeff (*1000)  (1/kN)  A1\n";
	fo << "a2="<< a[i++] << "	# Lateral friction coefficient at load = 0 (*1000)  2\n";
	fo << "a3="<< a[i++] << "	# Maximum stiffness   (N/deg)  A3\n";
	fo << "a4="<< a[i++] << "	# Load at maximum stiffness   (kN)  A4\n";
	fo << "a5="<< a[i++] << "	# Camber influence on stiffness   (%/deg/100)  A5\n";
	fo << "a6="<< a[i++] << "	# Curvature change with load  A6\n";
	fo << "a7="<< a[i++] << "	# Curvature at load = 0	 A7\n";
	fo << "a8="<< a[i++] << "	# Horizontal shift because of camber  (deg/deg)  A8\n";
	fo << "a9="<< a[i++] << "	# Load influence on horizontal shift  (deg/kN)  A9\n";
	fo << "a10="<< a[i++] << "	# Horizontal shift at load = 0  (deg)  A10\n";
	fo << "a111="<<a[i++] << "	# Camber influence on vertical shift  (N/deg/kN)  A11.1\n";
	fo << "a112="<<a[i++] << "	# Camber influence on vertical shift  (N/deg/kN**2)  A11.2\n";
	fo << "a12="<< a[i++] << "	# Load influence on vertical shift  (N/kN)  A12\n";
	fo << "a13="<< a[i++] << "	# Vertical shift at load = 0  (N)  A13\n";
	fo << "#--------	Longitudinal force\n";  i = 0;
	fo << "b0="<< b[i++] << "	# Shape factor   B0\n";
	fo << "b1="<< b[i++] << "	# Load infl. on long. friction coeff (*1000)  (1/kN)   B1\n";
	fo << "b2="<< b[i++] << "	# Longitudinal friction coefficient at load = 0 (*1000)  B2\n";
	fo << "b3="<< b[i++] << "	# Curvature factor of stiffness   (N/%/kN**2)   B3\n";
	fo << "b4="<< b[i++] << "	# Change of stiffness with load at load = 0 (N/%/kN)   B4\n";
	fo << "b5="<< b[i++] << "	# Change of progressivity of stiffness/load (1/kN)   B5\n";
	fo << "b6="<< b[i++] << "	# Curvature change with load   B6\n";
	fo << "b7="<< b[i++] << "	# Curvature change with load   B7\n";
	fo << "b8="<< b[i++] << "	# Curvature at load = 0   B8\n";
	fo << "b9="<< b[i++] << "	# Load influence on horizontal shift   (%/kN)   B9\n";
	fo << "b10="<<b[i++] << "	# Horizontal shift at load = 0   (%)   B10\n";
	fo << "#---------	Aligning moment\n";  i = 0;
	fo << "c0="<< c[i++] << "	# Shape factor   C0\n";
	fo << "c1="<< c[i++] << "	# Load influence of peak value   (Nm/kN**2)   C1\n";
	fo << "c2="<< c[i++] << "	# Load influence of peak value   (Nm/kN)   C2\n";
	fo << "c3="<< c[i++] << "	# Curvature factor of stiffness   (Nm/deg/kN**2)   C3\n";
	fo << "c4="<< c[i++] << "	# Change of stiffness with load at load = 0 (Nm/deg/kN)   C4\n";
	fo << "c5="<< c[i++] << "	# Change of progressivity of stiffness/load (1/kN)   C5\n";
	fo << "c6="<< c[i++] << "	# Camber influence on stiffness   (%/deg/100)   C6\n";
	fo << "c7="<< c[i++] << "	# Curvature change with load   C7\n";
	fo << "c8="<< c[i++] << "	# Curvature change with load   C8\n";
	fo << "c9="<< c[i++] << "	# Curvature at load = 0   C9\n";
	fo << "c10="<<c[i++] << "	# Camber influence of stiffness   C10\n";
	fo << "c11="<<c[i++] << "	# Camber influence on horizontal shift (deg/deg)   C11\n";
	fo << "c12="<<c[i++] << "	# Load influence on horizontal shift (deg/kN)   C12\n";
	fo << "c13="<<c[i++] << "	# Horizontal shift at load = 0 (deg)   C13\n";
	fo << "c14="<<c[i++] << "	# Camber influence on vertical shift (Nm/deg/kN**2)   C14\n";
	fo << "c15="<<c[i++] << "	# Camber influence on vertical shift (Nm/deg/kN)   C15\n";
	fo << "c16="<<c[i++] << "	# Load influence on vertical shift (Nm/kN)   C16\n";
	fo << "c17="<<c[i++] << "	# Vertical shift at load = 0 (Nm)   C17\n";
	fo << "#---------\n";


	txtTweakTire->setCaption(TR("#30FF30#{Saved}."));

	//  LoadTires in game thread, FillTweakLists after, in render
	sTireLoad = name;
	pGame->reloadSimNeed = true;
}


//  reset all
void CGui::btnTweakTireReset(WP)
{
	pGame->reloadSimNeed = true;
	txtTweakTire->setCaption(TR("#FF9030#{Reset}."));
}
