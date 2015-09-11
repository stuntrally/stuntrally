#pragma once
#include "Replay.h"
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
//#include <OgreOverlayElement.h>
#include <OgreOverlay.h>

#include "../shiny/Main/Factory.hpp"


namespace Ogre {  class SceneNode;  class SceneManager;  class Viewport;  class ManualObject;  }
namespace MyGUI  {  class MultiList2;  class Slider;  }
class GraphView;  class App;  class SETTINGS;  class CGui;


class CHud
{
public:
	App* app;
	SETTINGS* pSet;
	CGui* gui;
	
	CHud(App* ap1);
		
	
	//  minimap pos triangle, all needed info
	struct SMiniPos
	{
		float x,y;  // pos
		float px[4],py[4];  // points, rotation
	};

	///  HUD  ------------
	class Hud  // for 1 viewport/player
	{
	public:
		//  times bar
		MyGUI::TextBox *txTimTxt,*txTimes;  //MyGUI::ImageBox *bckTimes;
		Ogre::String sTimes,sLap;
		//  lap results
		MyGUI::TextBox *txLapTxt,*txLap;  MyGUI::ImageBox *bckLap;
		//  opponents list  columns: trk %, dist m, nick
		MyGUI::TextBox *txOpp[3];  MyGUI::ImageBox *bckOpp;
		int xOpp, yOpp, lastOppH;

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
		MyGUI::ImageBox *icoBFuel,*icoBInf,*icoDamage,*icoRewind;
		
		//  current camera name
		MyGUI::TextBox *txCam;

		//  miniap
		Ogre::ManualObject *moMap;  Ogre::SceneNode *ndMap;
		//  all info for this car's minimap
		std::vector<SMiniPos> vMiniPos;  // const size: 6
		
		//  center position .
		Ogre::Vector2 vcRpm, vcVel;  Ogre::Real fScale;
		bool updGauges;

		MyGUI::Widget* parent;
		Hud();
	};
	std::vector<Hud> hud;  // const size: max viewports 4

	///  global hud  ---------

	//  car pos tris on minimap
	//  one for all cars on all viewports
	Ogre::SceneNode* ndPos;
	Ogre::ManualObject* moPos;

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
		Ogre::Entity* ent;
		Ogre::SceneNode* node,*nodeRot;  // checkpoint arrow
		Ogre::Quaternion qStart, qEnd, qCur;  // smooth animation
		Arrow();
		void Create(Ogre::SceneManager* mSceneMgr, SETTINGS* pSet);
	} arrow;
		
	float asp, scX,scY, minX,maxX, minY,maxY;  // minimap visible range
	Ogre::SceneNode *ndLine;

	//------------------------------------------

	//  init
	void Create(), Destroy();
	Ogre::ManualObject* CreateVdrMinimap();  //vdr only

	//  show, size
	void Size(), Show(bool hideAll=false), ShowVp(bool vp);

	///  update
	void Update(int carId, float time);

	//  update internal
	void UpdRot(int baseCarId, int carId, float vel, float rpm);  // rpm < 0.f to hide
	void GetVals(int id, float* vel, float* rpm, float* clutch, int* gear);
	void UpdMiniTer(), UpdDbgTxtClr();

	//  util create
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn /*= false*/, bool clr /*= false*/,
		Ogre::Real mul, Ogre::Vector2 ofs,
		Ogre::uint32 vis, Ogre::uint8 rndQue, int cnt = 1);

	MyGUI::TextBox* CreateNickText(int carId, Ogre::String text);
	Ogre::Vector3 projectPoint(const Ogre::Camera* cam, const Ogre::Vector3& pos);  // 2d xy, z - out info

	//  string utils
	Ogre::String StrClr(Ogre::ColourValue c);

	//  bullet debug text
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);
};
