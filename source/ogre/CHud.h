#pragma once
#include "Replay.h"
#include "../vdrift/cardefs.h"
#include "CarModel.h"
#include "CarReflection.h"

#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"
#include "common/GraphView.h"
#include "common/Gui_Def.h"

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


class CHud : public BGui
{
public:
	App* app =0;
	SETTINGS* pSet =0;
	CGui* gui =0;
	
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
		Txt txTimTxt =0, txTimes =0;
		Ogre::String sTimes, sLap;
		//  lap results
		Txt txLapTxt =0, txLap =0;  Img bckLap =0;
		//  opponents list  columns: trk %, dist m, nick
		Txt txOpp[3]={0,0,0};  Img bckOpp =0;
		int xOpp =0, yOpp =0, lastOppH = -1;

		//  wrong check warning, win place
		Txt txWarn =0,  txPlace =0;
		Img bckWarn =0, bckPlace =0;
		//  start countdown
		Txt txCountdown =0;

		//  gauges
		Ogre::SceneNode    *ndNeedles =0, *ndGauges =0;
		Ogre::ManualObject *moNeedles =0, *moGauges =0;
		//  gear, vel
		Txt txVel =0, txGear =0, txAbs =0, txTcs =0;
		Img bckVel =0;

		//  damage %, rewind time, boost fuel
		float dmgBlink = 0.f, dmgOld = 0.f;
		Img imgDamage =0;
		Txt txDamage =0,  txRewind =0,  txBFuel =0;
		Img icoDamage =0, icoRewind =0, icoBFuel =0, icoBInf =0;
		
		//  input bars
		//Img imgSteer =0, barSteer =0,  barThrottle =0, barBrake =0;
		//Txt txtOther =0;  //barHandBrake = 0, barBoost = 0, barRewind =0;

		//  current camera name
		Txt txCam =0;

		//  miniap
		Ogre::ManualObject *moMap =0;  Ogre::SceneNode *ndMap =0;
		//  all info for this car's minimap
		std::vector<SMiniPos> vMiniPos;  // const size: 6
		
		//  center position .
		Ogre::Vector2 vcRpm, vcVel;  Ogre::Real fScale;
		bool updGauges =0;

		WP parent =0;
		Hud();
	};
	std::vector<Hud> hud;  // const size: max viewports 4


	///  global hud  ---------

	//  car pos tris on minimap
	//  one for all cars on all viewports
	Ogre::SceneNode* ndPos =0;  Ogre::ManualObject* moPos =0;

	//  chat messages
	Txt txMsg =0;  Img bckMsg =0;
	
	//  camera move info
	Txt txCamInfo =0;
	//  car debug texts  todo...
	Txt txDbgCar =0, txDbgTxt =0, txDbgExt =0;

	Ogre::SceneNode    *ndTireVis[4];
	Ogre::ManualObject *moTireVis[4];

	struct OvrDbg
	{
		Ogre::OverlayElement* oL=0,*oR=0,*oS=0, *oU=0,*oX=0;
		OvrDbg();
	};
	std::vector<OvrDbg> ov;
	Ogre::Overlay *ovCarDbg =0, *ovCarDbgTxt =0, *ovCarDbgExt =0;


	///  checkpoint arrow
	struct Arrow
	{
		Ogre::Entity* ent;
		Ogre::SceneNode* node,*nodeRot;  // checkpoint arrow
		Ogre::Quaternion qStart, qEnd, qCur;  // smooth animation
		Arrow();
		void Create(Ogre::SceneManager* mSceneMgr, SETTINGS* pSet);
	} arrow;
		
	float asp =1.f, scX =1.f, scY =1.f,
		minX =0.f, maxX =0.f, minY =0.f, maxY =0.f;  // minimap visible range
	Ogre::SceneNode *ndLine =0;

	//------------------------------------------

	//  init
	void Create(), Destroy();

	//  show, size
	void Size(), Show(bool hideAll=false), ShowVp(bool vp);

	///  update
	void Update(int carId, float time);
	//  update internal
	void UpdPosElems(int cnt, int cntC, int carId),
		 UpdRotElems(int baseCarId, int carId, float vel, float rpm),  // rpm < 0.f to hide
		 GetCarVals(int id, float* vel, float* rpm, float* clutch, int* gear),
		 UpdMiniTer(), UpdDbgTxtClr(),
		 UpdMultiplayer(int cnt, float time),
		 UpdOpponents(Hud& h, int cnt, CarModel* pCarM),
		 UpdMotBlur(CAR* pCar, float time),
		 UpdCarTexts(int carId, Hud& h, float time, CAR* pCar),
		 UpdTimes(int carId, Hud& h, float time, CAR* pCar, CarModel* pCarM),
		 UpdDebug(CAR* pCar, CarModel* pCarM);


	//  util create
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn /*= false*/, bool clr /*= false*/,
		Ogre::Real mul, Ogre::Vector2 ofs,
		Ogre::uint32 vis, Ogre::uint8 rndQue, int cnt = 1);

	Txt CreateNickText(int carId, Ogre::String text);
	Ogre::Vector3 projectPoint(const Ogre::Camera* cam, const Ogre::Vector3& pos);  // 2d xy, z - out info

	//  string utils
	Ogre::String StrClr(Ogre::ColourValue c);

#ifndef BT_NO_PROFILE
	//  bullet debug text
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);
#endif // BT_NO_PROFILE
};
