#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "CGui.h"
#include "CGame.h"
#include "CData.h"
#include "CarModel.h"
#include "common/Gui_Def.h"
#include "common/BltObjects.h"

using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Tweak
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
	
	PATHMANAGER::CreateDir(pathUserDir, pGame->error_output);
	std::ofstream fo(pathUser.c_str());
	fo << text.c_str();
	fo.close();
	
	app->NewGame();
}

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
		txtTweakPath->setCaption((user ? "User: " : "Original: ") + path);
		txtTweakPath->setTextColour(user ? Colour(1,1,0.5) : Colour(0.5,1,1));
		
		//MyGUI::InputManager::getInstance().resetKeyFocusWidget();
		//MyGUI::InputManager::getInstance().setKeyFocusWidget(edTweak);
	}
}

void CGui::CmbTweakCarSet(CMB)
{
}
void CGui::CmbTweakTireSet(CMB)
{
}

void CGui::CmbEdTweakCarSet(EditPtr ed)
{
}
void CGui::CmbEdTweakTireSet(EditPtr ed)
{
	if (txtTweakTire)
		txtTweakTire->setCaption("");
}														


//  tweak save car and reload game
void CGui::TweakTireSave()
{
	//TODO: game reload tires, user
	// ed car setup, name, load
	// jump to section,  help on current line
	// ed find text? syntax clr?=
	
	const CARTIRE* tire = app->carModels[0]->pCar->dynamics.GetTire(FRONT_LEFT);  //!
	const std::vector <Dbl>& a = tire->lateral, b = tire->longitudinal, c = tire->aligning;
	//#define f2s(f)  fToStr(f, 4,6);
	
	string file = cmbTweakTireSet->getCaption();
	string pathUserT = PATHMANAGER::DataUser() + "/carsim/" + pSet->game.sim_mode + "/tires/";
	PATHMANAGER::CreateDir(pathUserT, pGame->error_output);
	file = pathUserT+"/"+file+".tire";
	if (PATHMANAGER::FileExists(file))
	{
		if (txtTweakTire)
		{	txtTweakTire->setCaption("File already exists.");
			txtTweakTire->setTextColour(Colour(1,0.2,0.2));  }
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

	if (txtTweakTire)
	{	txtTweakTire->setCaption("Saved.");
		txtTweakTire->setTextColour(Colour(0.2,1,0.2));  }
}

void CGui::btnTweakCarSave(WP){	TweakCarSave();  }
void CGui::btnTweakCarLoad(WP){	TweakCarLoad();  }
void CGui::btnTweakTireSave(WP){	TweakTireSave();  }


//  Tweak collisions
//-----------------------------------------------------------------------------------------

void CGui::TweakColSave()
{
	String text = edTweakCol->getCaption();
	if (text == "")  return;
	text = StringUtil::replaceAll(text, "##", "#");
	//text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!

	std::string path = PATHMANAGER::DataUser() + "/trees";
	PATHMANAGER::CreateDir(path, pGame->error_output);
	path += "/collisions.xml";
	std::ofstream fo(path.c_str());
	fo << text.c_str();
	fo.close();
	TweakColUpd(true);
	
	app->data->objs->LoadXml();
	LogO(String("**** Loaded Vegetation objects: ") + toStr(app->data->objs->colsMap.size()));
	app->NewGame();
}

void CGui::TweakColUpd(bool user)
{
	txtTweakPathCol->setCaption((user ? "User" : "Original"));
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
//-----------------------------------------------------------------------------------------
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
	}
	
	//  save and reload  shift-alt-Z
	if (!vis && app->shift)
	if (tabTweak && tabTweak->getIndexSelected() < 2)
		TweakCarSave();
	else
		TweakColSave();
}

void CGui::tabCarEdChng(MyGUI::TabPtr, size_t id)
{
	pSet->car_ed_tab = id;
}


//  Get car file path
bool CGui::GetCarPath(std::string* pathCar,
	std::string* pathSave, std::string* pathSaveDir,
	std::string carname, /*std::string tweakSetup,*/ bool forceOrig)
{
	std::string file = carname + ".car",
		pathOrig  = PATHMANAGER::CarSim()          + "/" + pSet->game.sim_mode + "/cars/" + file,
		pathUserD = PATHMANAGER::DataUser() + "/carsim/" + pSet->game.sim_mode + "/cars/",
		pathUser  = pathUserD + file;                          // (tweakSetup != "" ? tweakSetup+"/" : "")

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
