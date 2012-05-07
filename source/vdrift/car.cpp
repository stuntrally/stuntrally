#include "pch.h"
#include "car.h"
#include "cardefs.h"
#include "configfile.h"
#include "coordinatesystems.h"
#include "collision_world.h"
#include "tracksurface.h"
#include "configfile.h"
#include "settings.h"
#include "../ogre/OgreGame.h"  //+ replay
#include "../ogre/CarModel.h"  //+ camera pos
#include "../ogre/FollowCamera.h"  //+ camera pos
#include "../ogre/common/Defines.h"
#include "../ogre/common/GraphView.h"
#include "../network/protocol.hpp"
#include "tobullet.h"
#include "game.h"  //sound

#ifdef _WIN32
bool isnan(float number) {return (number != number);}
bool isnan(double number) {return (number != number);}
#endif

CAR::CAR() :
	pSet(0), pApp(0), id(0), pCarM(0),
	last_steer(0),
	sector(-1),
	iCamNext(0), bLastChk(0),bLastChkOld(0),
	fluidHitOld(0),
	trackPercentCopy(0), bRemoteCar(0),
	bResetPos(0)
{
	//dynamics.pCar = this;
	vInteriorOffset[0]=0;
	vInteriorOffset[1]=0;
	vInteriorOffset[2]=0;
	
	for (int i = 0; i < 4; i++)
	{
		curpatch[i] = NULL;
		//wheelnode[i] = NULL;
		//floatingnode[i] = NULL;
	}
	for (int i=0; i < Ncrashsounds; ++i)
		crashsoundtime[i] = 0.f;
}

///unload any loaded assets
CAR::~CAR()
{	}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::Load(class App* pApp1,
	SETTINGS* settings,
	CONFIGFILE & carconf,
	const std::string & carpath,
	const std::string & driverpath,
	const std::string & carname,
	const MATHVECTOR <float, 3> & init_pos,
	const QUATERNION <float> & init_rot,
	COLLISION_WORLD & world,
	bool soundenabled,
	const SOUNDINFO & sound_device_info,
	const SOUNDBUFFERLIBRARY & soundbufferlibrary,
	bool defaultabs, bool defaulttcs,
	bool isRemote,
	int idCar,
  	bool debugmode,
  	std::ostream & info_output,
  	std::ostream & error_output )
{
	pApp = pApp1;
	pSet = settings;
	cartype = carname;
	bRemoteCar = isRemote;
	id = idCar;
	std::stringstream nullout;

	//load car body graphics
	if (!LoadInto( carpath+"/"+carname+"/body.joe", bodymodel, error_output))
		/*info_output << "No car body model, continuing without one" << std::endl*/;

	//load driver graphics --
	//if (!LoadInto( driverpath+"/body.joe", drivermodel, error_output))
	//	error_output << "Error loading driver graphics: " << driverpath << std::endl;

	//load car interior graphics
	if (!LoadInto( carpath+"/"+carname+"/interior.joe", interiormodel, nullout ))
		/*info_output << "No car interior model exists, continuing without one" << std::endl*/;

	//load car glass graphics
	if (!LoadInto( carpath+"/"+carname+"/glass.joe", glassmodel, nullout ))
		/*info_output << "No car glass model exists, continuing without one" << std::endl*/;

	//load wheel graphics
	for (int i = 0; i < 2; i++)  // front pair
	{
		if (!LoadInto( carpath+"/"+carname+"/wheel_front.joe", wheelmodelfront, error_output))
			/*info_output << "No car wheel_front model, continuing without one" << std::endl*/;

		//load floating elements
		std::stringstream nullout;
		LoadInto( carpath+"/"+carname+"/floating_front.joe", floatingmodelfront, nullout);
	}
	for (int i = 2; i < 4; i++)  // rear pair
	{
		if (!LoadInto( carpath+"/"+carname+"/wheel_rear.joe", wheelmodelrear, error_output))
			/*info_output << "No car wheel_rear model, continuing without one" << std::endl*/;

		//load floating elements
		std::stringstream nullout;
		LoadInto( carpath+"/"+carname+"/floating_rear.joe", floatingmodelrear, nullout);
	}

	// get coordinate system version
	int version = 1;
	carconf.GetParam("version", version);
	
	
	///-  custom interior model offset--
	vInteriorOffset[0] = 0.f;
	vInteriorOffset[1] = 0.f;
	vInteriorOffset[2] = 0.f;
	carconf.GetParam("model_ofs.interior-x", vInteriorOffset[0]);
	carconf.GetParam("model_ofs.interior-y", vInteriorOffset[1]);
	carconf.GetParam("model_ofs.interior-z", vInteriorOffset[2]);
	
	///-  custom exhaust pos for boost particles
	if (carconf.GetParam("model_ofs.exhaust-x", exhaustPosition[0]))
	{
		manualExhaustPos = true;
		carconf.GetParam("model_ofs.exhaust-y", exhaustPosition[1]);
		carconf.GetParam("model_ofs.exhaust-z", exhaustPosition[2]);
	}
	else
		manualExhaustPos = false;
	if (!carconf.GetParam("model_ofs.exhaust-mirror-second", has2exhausts))
		has2exhausts = false;

	///-  custom car collision params
	dynamics.coll_R = 0.3f;  dynamics.coll_H = 0.45f;  dynamics.coll_W = 0.5f;
	dynamics.coll_Lofs = 0.f;  dynamics.coll_Wofs = 0.f;  dynamics.coll_Hofs = 0.f;
	dynamics.coll_manual = false;  // normally auto

	if (carconf.GetParam("collision.manual", dynamics.coll_manual))
	{
		carconf.GetParam("collision.radius", dynamics.coll_R);
		carconf.GetParam("collision.width", dynamics.coll_W);
		carconf.GetParam("collision.height", dynamics.coll_H);

		carconf.GetParam("collision.offsetL", dynamics.coll_Lofs);
		carconf.GetParam("collision.offsetW", dynamics.coll_Wofs);
		carconf.GetParam("collision.offsetH", dynamics.coll_Hofs);
	}
	

	// load cardynamics
	{
		if (!dynamics.Load(carconf, error_output)) return false;

		MATHVECTOR<double, 3> position;
		QUATERNION<double> orientation;
		position = init_pos;	posAtStart = posLastCheck[0] = posLastCheck[1] = init_pos;
		orientation = init_rot;	rotAtStart = rotLastCheck[0] = rotLastCheck[1] = init_rot;
		
		dynamics.Init(pSet, &pApp->sc, &pApp->fluidsXml,
			world, bodymodel, wheelmodelfront, wheelmodelrear, position, orientation);

		dynamics.SetABS(defaultabs);
		dynamics.SetTCS(defaulttcs);
	}

	// load driver
	{
		float pos[3];
		if (!carconf.GetParam("driver.view-position", pos, error_output)) return false;
		if (version == 2) COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0], pos[1], pos[2]);
		
		driver_view_position.Set(pos[0], pos[1], pos[2]);
		
		if (!carconf.GetParam("driver.hood-mounted-view-position", pos, error_output)) return false;
		if (version == 2) COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0], pos[1], pos[2]);
		
		hood_view_position.Set(pos[0], pos[1], pos[2]);
		
		/*if (drivernode) //move the driver model to the coordinates given
		{
			MATHVECTOR <float, 3> floatpos;
			floatpos.Set(pos[0], pos[1], pos[2]);
			drivernode->GetTransform().SetTranslation(floatpos);
		}*/
	}

	//load sounds
	if (soundenabled)
	{
		if (!LoadSounds(carpath, carname, sound_device_info, soundbufferlibrary, info_output, error_output))
			return false;
	}

	mz_nominalmax = (GetTireMaxMz(FRONT_LEFT) + GetTireMaxMz(FRONT_RIGHT))*0.5;

	return true;
}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadInto(const std::string & joefile, MODEL_JOE03 & output_model,	std::ostream & error_output)
{
	if (!output_model.Loaded())
	{
		std::stringstream nullout;
		if (!output_model.ReadFromFile(joefile.substr(0,std::max((long unsigned int)0,(long unsigned int) joefile.size()-3))+"ova", nullout))
		{
			if (!output_model.Load(joefile, error_output))
			{
				/*error_output << "Error loading model: " << joefile << std::endl;*/
				return false;
			}
		}
	}
	return true;
}

void CAR::SetPosition(const MATHVECTOR <float, 3> & new_position)
{
	MATHVECTOR <double,3> newpos;
	newpos = new_position;
	dynamics.SetPosition(newpos);
	dynamics.AlignWithGround();//--

	QUATERNION <float> rot;
	rot = dynamics.GetOrientation();
}


void CAR::Update(double dt)
{
	dynamics.Update();
	UpdateSounds(dt);

	///  graphs new values  .-_/\_.-
	if (pApp->pSet->show_graphs && id == 0)  // for 1st car
		GraphsNewVals(dt);  // implementation in Hud_Graphs.cpp
}


///   Car Inputs  * * * * * * * 
//--------------------------------------------------------------------------------------------------------------------------
void CAR::HandleInputs(const std::vector <float> & inputs, float dt)
{
	assert(inputs.size() == CARINPUT::ALL); //-
	if (pApp && pApp->IsFocGuiInput())
		return;

	dynamics.inputsCopy = inputs;

	int cur_gear = dynamics.GetTransmission().GetGear();
	bool rear = pSet->rear_inv ? cur_gear == -1 : false;  //if (disable_auto_rear)  rear = false;

	//  set brakes
	if (!bRemoteCar)
	{
		float brake = !rear ? inputs[CARINPUT::BRAKE] : inputs[CARINPUT::THROTTLE];
		dynamics.SetBrake(brake);
	}
	dynamics.SetHandBrake(inputs[CARINPUT::HANDBRAKE]);
	
	//  boost, flip over
	if (!bRemoteCar)
		dynamics.doBoost = inputs[CARINPUT::BOOST];
	dynamics.doFlip = inputs[CARINPUT::FLIP];

	//  steering
	if (!bRemoteCar)
	{
		float steer_value = inputs[CARINPUT::STEER_RIGHT];
		if (std::abs(inputs[CARINPUT::STEER_LEFT]) > std::abs(inputs[CARINPUT::STEER_RIGHT])) //use whichever control is larger
			steer_value = -inputs[CARINPUT::STEER_LEFT];
		dynamics.SetSteering(steer_value);
		last_steer = steer_value;
	}

    //start the engine if requested
	//if (inputs[CARINPUT::START_ENGINE])
	//	dynamics.StartEngine();

	//  shifting
	int gear_change = 0;
	if (inputs[CARINPUT::SHIFT_UP] == 1.0)		gear_change = 1;
	if (inputs[CARINPUT::SHIFT_DOWN] == 1.0)	gear_change = -1;
	int new_gear = cur_gear + gear_change;

	/*if (inputs[CARINPUT::REVERSE])	new_gear = -1;
	if (inputs[CARINPUT::FIRST_GEAR])	new_gear = 1;*/

	//  throttle
	float throttle = !rear ? inputs[CARINPUT::THROTTLE] : inputs[CARINPUT::BRAKE];
	float clutch = 1 - inputs[CARINPUT::CLUTCH]; // 

	dynamics.ShiftGear(new_gear);
	dynamics.SetThrottle(throttle);
	dynamics.SetClutch(clutch);

	//  abs tcs
	///TODO: car setup separated for all (4) players: (auto shift, auto rear, rear inv, abs, tcs)  ...
	//if (inputs[CARINPUT::ABS_TOGGLE])	dynamics.SetABS(!dynamics.GetABSEnabled());
	//if (inputs[CARINPUT::TCS_TOGGLE])	dynamics.SetTCS(!dynamics.GetTCSEnabled());
	
	//  cam
	iCamNext = -inputs[CARINPUT::PREV_CAM] + inputs[CARINPUT::NEXT_CAM];
	
	bLastChkOld = bLastChk;
	bLastChk = inputs[CARINPUT::LAST_CHK];
	
	if (bResetPos)  // reset game
	{	ResetPos(true);  bResetPos = false;  }
	else
	if (bLastChk && !bLastChkOld)
		ResetPos(false);  // goto last checkpoint
		//ResetPos(false, shift);  // for 2nd ... press twice?
}


//--------------------------------------------------------------------------------------------------------------------------
float CAR::GetFeedback()
{
	return dynamics.GetFeedback() / (mz_nominalmax * 0.025);
}

float CAR::GetTireSquealAmount(WHEEL_POSITION i, float* slide, float* s1, float* s2) const
{
	const TRACKSURFACE* surface = dynamics.GetWheelContact(WHEEL_POSITION(i)).GetSurfacePtr();
	if (!surface)  return 0;
	if (surface->type == TRACKSURFACE::NONE)
		return 0;
		
	float d = dynamics.GetWheelContact(WHEEL_POSITION(i)).GetDepth() - 2*GetTireRadius(WHEEL_POSITION(i));
	bool inAir = d > 0.f;  //d > 0.9f || d < 0.f;  //*!
	//if (onGround)  *onGround = !inAir;
	if (inAir)  // not on ground 
		return 0;

	MATHVECTOR <float, 3> groundvel;
	groundvel = dynamics.GetWheelVelocity(WHEEL_POSITION(i));
	QUATERNION <float> wheelspace;
	wheelspace = dynamics.GetUprightOrientation(WHEEL_POSITION(i));
	(-wheelspace).RotateVector(groundvel);
	float wheelspeed = dynamics.GetWheel(WHEEL_POSITION(i)).GetAngularVelocity()*dynamics.GetTire(WHEEL_POSITION(i)).GetRadius();
	groundvel[0] -= wheelspeed;
	groundvel[1] *= 2.0;
	groundvel[2] = 0;
	float squeal = (groundvel.Magnitude() - 3.0) * 0.2;
	if (slide)  *slide = squeal + 0.6;

	std::pair <double, double> slideslip = dynamics.GetTire(i).GetSlideSlipRatios();
	if (s1)  *s1 = slideslip.first;
	if (s2)  *s2 = slideslip.second;
	double maxratio = std::max(std::abs(slideslip.first), std::abs(slideslip.second));
	float squealfactor = std::max(0.0, maxratio - 1.0);
	squeal *= squealfactor;
	if (squeal < 0)  squeal = 0;
	if (squeal > 1)  squeal = 1;
	return squeal;
}

void CAR::EnableGlass(bool enable)
{
	/*if (glassdraw)
	{
		glassdraw->SetDrawEnable(enable);
	}*/
}

bool CAR::Serialize(joeserialize::Serializer & s)
{
	_SERIALIZE_(s,dynamics);
	_SERIALIZE_(s,last_steer);
	return true;
}


//  Network CAR data send/receive
///------------------------------------------------------------------------------------------------------------------------------
protocol::CarStatePackage CAR::GetCarStatePackage() const
{
	protocol::CarStatePackage csp;
	csp.pos = ToMathVector<float>(dynamics.chassis->getCenterOfMassPosition());
	csp.rot = ToMathQuaternion<float>(dynamics.chassis->getCenterOfMassTransform().getRotation());
	csp.linearVel = GetVelocity();
	csp.angularVel = GetAngularVelocity();

	//  steer
	csp.steer = dynamics.GetSteering();
	csp.boost = dynamics.doBoost * 255.f;  // pack to uint8
	csp.brake = dynamics.IsBraking() * 255.f;
	//csp.trackPercent = trackPercentCopy;  // needed from CarModel
	return csp;
}

void CAR::UpdateCarState(const protocol::CarStatePackage& state)
{
	// Velocity based estimation from physics engine works rather well
	// for a while, so we use pos/rot only for lazy corrections
	MATHVECTOR<float,3> curpos = ToMathVector<float>(dynamics.chassis->getCenterOfMassPosition());
	MATHVECTOR<float,3> errorvec = state.pos - curpos;
	MATHVECTOR<float,3> newpos = curpos + (errorvec * 0.05f);
	QUATERNION<float> currot = ToMathQuaternion<float>(dynamics.chassis->getCenterOfMassTransform().getRotation());
	QUATERNION<float> newrot = currot.QuatSlerp(state.rot, 0.5f);

	// If the estimate drifts too far for some reason, do a quick correction
	if (errorvec.MagnitudeSquared() > 9.0f) {
		newpos = state.pos;
		newrot = state.rot;  }

	SetPosition(newpos);

	btTransform transform;
	transform.setOrigin(ToBulletVector(newpos));
	transform.setRotation(ToBulletQuaternion(newrot));
	dynamics.chassis->setWorldTransform(transform);

	// No interpolation in velocities
	dynamics.chassis->setLinearVelocity(ToBulletVector(state.linearVel));
	dynamics.chassis->setAngularVelocity(ToBulletVector(state.angularVel));

	dynamics.SynchronizeBody();  // set body from chassis
	dynamics.UpdateWheelContacts();

	//  steer
	dynamics.SetSteering(state.steer);	last_steer = state.steer;
	dynamics.doBoost = state.boost / 255.f;  // unpack from uint8
	dynamics.SetBrake(state.brake / 255.f);
	trackPercentCopy = state.trackPercent / 255.f * 100.f;
}

///  reset car, pos and state
///------------------------------------------------------------------------------------------------------------------------------
void CAR::ResetPos(bool fromStart, int chkId)
{
	MATHVECTOR <float, 3> pos = fromStart ? posAtStart : posLastCheck[chkId];
	QUATERNION <float> rot =    fromStart ? rotAtStart : rotLastCheck[chkId];
	SetPosition(pos);

	btTransform transform;
	transform.setOrigin(ToBulletVector(pos));
	transform.setRotation(ToBulletQuaternion(rot));
	dynamics.chassis->setWorldTransform(transform);

	dynamics.chassis->setLinearVelocity(btVector3(0,0,0));
	dynamics.chassis->setAngularVelocity(btVector3(0,0,0));

	dynamics.SynchronizeBody();  // set body from chassis
	if (fromStart)  // restore boost fuel
		dynamics.boostFuel = gfBoostFuelStart;

	//  engine, wheels
	dynamics.engine.SetInitialConditions();
	for (int w=0; w < 4; ++w)
	{
		MATHVECTOR <CARDYNAMICS::T, 3> zero(0,0,0);
		dynamics.wheel[w].SetAngularVelocity(0);
		//dynamics.wheel_velocity[w] = zero;
	}
	crashdetection.Update(0.f, 0.1f);  //prevent car hit sound

	//dynamics.SynchronizeChassis();
	dynamics.UpdateWheelContacts();
}

///  save car pos and rot
void CAR::SavePosAtCheck()
{
	posLastCheck[1] = posLastCheck[0];  // 2nd last check
	rotLastCheck[1] = rotLastCheck[0];
	posLastCheck[0] = dynamics.body.GetPosition();
	rotLastCheck[0] = dynamics.body.GetOrientation();
}
