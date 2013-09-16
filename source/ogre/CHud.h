#pragma once
#include "ReplayGame.h"
#include "../vdrift/cardefs.h"
#include "CarModel.h"
#include "CarReflection.h"

#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"
#include "common/GraphView.h"

#include "../network/networkcallbacks.hpp"
#include <boost/thread.hpp>
#include <MyGUI.h>
#include <OgreShadowCameraSetup.h>
#include <OgreOverlayElement.h>
#include <OgreOverlay.h>

#include "../shiny/Main/Factory.hpp"


namespace Ogre {  class SceneNode;  class SceneManager;  class Viewport;  }
namespace MyGUI  {  class MultiList2;  class Slider;  }
class GraphView;
class App;
class SETTINGS;
class CGui;


class CHud
{
public:
	App* app;
	SETTINGS* pSet;
	CGui* gui;
	
	CHud(App* ap1);
		
	
	///  HUD  ------------
	class Hud  // for 1 viewport/player
	{
	public:
		//  times bar
		MyGUI::TextBox *txTimTxt,*txTimes;  MyGUI::ImageBox *bckTimes;
		Ogre::String sTimes;
		//  opponents list  columns: trk %, dist m, nick
		MyGUI::TextBox *txOpp[3];  MyGUI::ImageBox *bckOpp;

		//  wrong check warning, win place
		MyGUI::TextBox *txWarn,*txPlace;  MyGUI::ImageBox *bckWarn,*bckPlace;
		//  start countdown
		MyGUI::TextBox *txCountdown;

		//  gauges
		Ogre::SceneNode    *ndNeedles, *ndGauges;
		Ogre::ManualObject *moNeedles, *moGauges;
		//  gear, vel
		MyGUI::TextBox *txGear,*txVel, *txAbs,*txTcs;
		MyGUI::ImageBox *bckVel;

		//  boost fuel, damage %, rewind time
		MyGUI::TextBox  *txBFuel, *txDamage, *txRewind;
		MyGUI::ImageBox *icoBFuel,*icoDamage,*icoRewind;
		
		//  current camera name
		MyGUI::TextBox *txCam;

		//  miniap
		Ogre::ManualObject *moMap;  Ogre::SceneNode *ndMap;
		//  car pos tris on minimap +2ghosts
		std::vector<Ogre::SceneNode*> vNdPos;  //const size: 6
		std::vector<Ogre::ManualObject*> vMoPos;
		
		//  center position .
		Ogre::Vector2 vcRpm, vcVel;  Ogre::Real fScale;
		bool updGauges;

		MyGUI::Widget* parent;
		Hud();
	};
	std::vector<Hud> hud;  // const size: max viewports 4

	///  global hud
	//  chat messages
	MyGUI::TextBox *txMsg;  MyGUI::ImageBox *bckMsg;
	//  camera move info
	MyGUI::TextBox *txCamInfo;
	//  car debug texts  todo...
	MyGUI::TextBox *txDbgCar,*txDbgTxt,*txDbgExt;

	Ogre::SceneNode    *ndTireVis[4];
	Ogre::ManualObject *moTireVis[4];

	struct OvrDbg
	{
		Ogre::OverlayElement* oL,*oR,*oS, *oU,*oX;
		OvrDbg();
	};
	std::vector<OvrDbg> ov;
	Ogre::Overlay *ovCarDbg,*ovCarDbgTxt,*ovCarDbgExt;


	///  checkpoint arrow
	struct Arrow
	{
		Ogre::SceneNode* node,*nodeRot;  // checkpoint arrow
		Ogre::Quaternion qStart, qEnd, qCur;  // smooth animation
		Arrow();
	} arrow;
		
	float asp, scX,scY, minX,maxX, minY,maxY;  // minimap visible range
	Ogre::SceneNode *ndLine;

	//------------------------------------------

	//  init
	void Create(), Destroy();
	void CreateArrow();

	//  show, size
	void Size(bool full, Ogre::Viewport* vp=NULL);
	void Show(bool hideAll=false), ShowVp(bool vp);

	///  update
	void Update(int carId, float time);

	//  update internal
	void UpdRot(int baseCarId, int carId, float vel, float rpm);
	void GetVals(int id, float* vel, float* rpm, float* clutch, int* gear);
	void UpdMiniTer(), UpdDbgTxtClr();

	//  util create
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn /*= false*/, bool clr /*= false*/,
		Ogre::Real mul, Ogre::Vector2 ofs,
		Ogre::uint32 vis, Ogre::uint8 rndQue, bool comb = false);

	MyGUI::TextBox* CreateNickText(int carId, Ogre::String text);
	Ogre::Vector3 projectPoint(const Ogre::Camera* cam, const Ogre::Vector3& pos);  // 2d xy, z - out info

	//  string utils
	static Ogre::String StrTime(float time), StrTime2(float time);  // 2=short
	Ogre::String StrClr(Ogre::ColourValue c);

	//  bullet debug text
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);
};
