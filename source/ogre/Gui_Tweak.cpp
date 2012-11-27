#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "common/Gui_Def.h"

using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Tweak
//-----------------------------------------------------------------------------------------------------------

void App::TweakCarSave()
{
	String text = edTweak->getCaption();
	if (text == "")  return;
	text = StringUtil::replaceAll(text, "##", "#");
	text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);
	
	PATHMANAGER::CreateDir(pathUserDir, pGame->error_output);
	std::ofstream fo(pathUser.c_str());
	fo << text.c_str();
	fo.close();
	
	NewGame();
}

void App::TweakCarLoad()
{
	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);

	if (!PATHMANAGER::FileExists(path))
	{
		edTweak->setCaption("");
		txtTweakPath->setCaption("Not Found ! " + path);
		txtTweakPath->setColour(Colour(1,0,0));
	}else
	{
		std::ifstream fi(path.c_str());
		String text = "", s;
		while (getline(fi,s))
			text += s + "\n";
		fi.close();

		text = StringUtil::replaceAll(text, "#", "##");
		text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!
		edTweak->setCaption(UString(text));
		//edTweak->setVScrollPosition(0);

		size_t p = path.find("cars");
		if (p != string::npos)
			path = path.substr(p+5, path.length());
		txtTweakPath->setCaption((user ? "User: " : "Original: ") + path);
		txtTweakPath->setTextColour(user ? Colour(1,1,0.5) : Colour(0.5,1,1));
		
		MyGUI::InputManager::getInstance().resetKeyFocusWidget();
		MyGUI::InputManager::getInstance().setKeyFocusWidget(edTweak);
	}
}

void App::CmbTweakCarSet(CMB)
{
}
void App::CmbTweakTireSet(CMB)
{
}

void App::CmbEdTweakCarSet(EditPtr ed)
{
}
void App::CmbEdTweakTireSet(EditPtr ed)
{
	if (txtTweakTire)
		txtTweakTire->setCaption("");
}														


//  tweak save car and reload game
void App::TweakTireSave()
{
	//TODO: game reload tires, user
	// ed car setup, name, load
	// jump to section,  help on current line
	// ed find text? syntax clr?=
	
	const CARTIRE* tire = carModels[0]->pCar->dynamics.GetTire(FRONT_LEFT);  //!
	const std::vector <Dbl>& a = tire->lateral, b = tire->longitudinal, c = tire->aligning;
	//#define f2s(f)  fToStr(f, 4,6);
	
	string file = cmbTweakTireSet->getCaption();
	file = PATHMANAGER::GetTiresPathUser()+"/"+file+".tire";
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

void App::btnTweakCarSave(WP){	TweakCarSave();  }
void App::btnTweakCarLoad(WP){	TweakCarLoad();  }
void App::btnTweakTireSave(WP){	TweakTireSave();  }


///  Tweak read / save file
//-----------------------------------------------------------------------------------------
void App::TweakToggle()
{
	//  window
	bool vis = !mWndTweak->getVisible();
	mWndTweak->setVisible(vis);

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);
	
	//  load  if car changed
	static string lastPath = "";
	if (lastPath != path || ctrl)  // force reload  ctrl-alt-Z
	{	lastPath = path;
		TweakCarLoad();
	}
	
	//  save and reload  shift-alt-Z
	if (!vis && shift)
		TweakCarSave();
}


//  Get car file path
bool App::GetCarPath(std::string* pathCar, std::string* pathSave, std::string* pathSaveDir,
	std::string carname, bool asphalt, std::string tweakSetup, bool forceOrig)
{
	std::string file = carname /*+ (asphalt ? "_a":"")*/ + ".car",
		pathOrig  = PATHMANAGER::GetCarPath()     + "/"+carname+"/"+ file,
		pathUserD = PATHMANAGER::GetCarPathUser() + "/"+carname+"/"+ (tweakSetup != "" ? tweakSetup+"/" : ""),
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
