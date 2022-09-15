#include "pch.h"
#include "common/Def_Str.h"
#include "common/CScene.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/game.h"
#include "common/data/SceneXml.h"
#include "common/RenderConst.h"
#include "CGame.h"
#include "CGui.h"
#include "CarModel.h"
#include "SplitScreen.h"
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../road/Road.h"
#include "../shiny/Main/Factory.hpp"
#include "../network/gameclient.hpp"
#include <OgreException.h>
#include <OgreTerrain.h>
#include <OgreEntity.h>
#include <OgreManualObject.h>
#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreRibbonTrail.h>
#include <OgreBillboardSet.h>
#include <OgreBillboardChain.h>
#include <OgreSceneNode.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace std;
#define  FileExists(s)  PATHMANAGER::FileExists(s)


//  ctor
//------------------------------------------------------------------------------------------------------
CarModel::CarModel(int index, int colorId, eCarType type, const string& name,
	SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s, Camera* cam, App* app)
	:mSceneMgr(sceneMgr), pSet(set), pGame(game), sc(s), mCamera(cam), pApp(app)
	,iIndex(index), iColor(colorId % 6), sDirname(name), eType(type), vtype(V_Car)
	,fCam(0), iCamFluid(-1), fCamFl(0.6f)
	,pMainNode(0), pCar(0), terrain(0), ndSph(0), brakes(0)
	,pReflect(0), color(0,1,0), maxangle(26.f)
	,hideTime(1.f), mbVisible(true), bLightMapEnabled(true), bBraking(false)
	,iCamNextOld(0), bLastChkOld(0), bInSt(0), bWrongChk(0),  iFirst(0)
	,angCarY(0), vStartPos(0,0,0), pNickTxt(0)
	,ndNextChk(0), entNextChk(0)
	,all_subs(0), all_tris(0)  //stats
	,bGetStPos(true), fChkTime(0.f), iWonPlace(0), iWonPlaceOld(0), iWonMsgTime(0.f)
	,iInChk(-1),iCurChk(-1), iNumChks(0), iNextChk(0), iLoopChk(-1)  //ResetChecks();  // no road yet
	,iInWrChk(-1), timeAtCurChk(0.f), iLoopLastCam(-1)
	,sChkMtr("checkpoint_normal"), bChkUpd(true)
	,distFirst(1.f), distLast(1.f), distTotal(10.f), trackPercent(0.f)
	,updTimes(1), updLap(1), fLapAlpha(1.f)
{
	SetNumWheels(4);
	int i,w;
	for (w = 0; w < MAX_WHEELS; ++w)
	for (int p=0; p < PAR_ALL; ++p)
		par[p][w] = 0;

	for (i=0; i < PAR_BOOST; ++i)  parBoost[i] = 0;
	for (i=0; i < PAR_THRUST*2; ++i)  parThrust[i] = 0;
	parHit = 0;

	qFixWh[0].Rotate(2*PI_d,0,0,1);
	qFixWh[1].Rotate(  PI_d,0,0,1);

	Defaults();
}

void CarModel::SetNumWheels(int n)
{
	numWheels = n;
	whPos.resize(n);  whRadius.resize(n);  whWidth.resize(n);
	whTrail.resize(n);  whTemp.resize(n);
	ndWh.resize(n);  ndWhE.resize(n);  ndBrake.resize(n);
}

void CarModel::Defaults()
{
	int i,w;
	for (i=0; i < 3; ++i)
	{
		driver_view[i] = 0.f;  hood_view[i] = 0.f;  ground_view[i] = 0.f;
		interiorOffset[i] = 0.f;  boostOffset[i] = 0.f;  exhaustPos[i] = 0.f;
	}
	camDist = 1.f;
	for (i=0; i < PAR_THRUST; ++i)
	{
		for (w=0; w<3; ++w)  thrusterOfs[i][w] = 0.f;
		thrusterSizeZ[i] = 0.f;
		sThrusterPar[i] = "";
	}
	brakePos.clear();
	brakeClr = ColourValue(1,0,0);
	brakeSize = 0.f;

	bRotFix = false;
	sBoostParName = "Boost";  boostSizeZ = 1.f;

	for (w=0; w < numWheels; ++w)
	{
		whRadius[w] = 0.3f;  whWidth[w] = 0.2f;
	}
	manualExhaustPos = false;  has2exhausts = false;

	maxangle = 26.f;
	for (w=0; w < 2; ++w)
		posSph[w] = Vector3::ZERO;

	matStPos = Matrix4::IDENTITY;
	vStDist = Vector4(0,0,0,0);
}


//  Load CAR
//------------------------------------------------------------------------------------------------------
void CarModel::Load(int startId, bool loop)
{
	//  names for local play
	if (isGhostTrk())    sDispName = TR("#{Track}");
	else if (isGhost())  sDispName = TR("#{Ghost}");
	else if (eType == CT_LOCAL)
		sDispName = TR("#{Player}") + toStr(iIndex+1);
	

	///  load config .car
	string pathCar;
	pApp->gui->GetCarPath(&pathCar, 0, 0, sDirname, pApp->mClient.get() != 0);  // force orig for newtorked games
	LoadConfig(pathCar);

	
	///  Create CAR (dynamic)
	if (!isGhost())  // ghost has pCar, dont create
	{
		if (startId == -1)  startId = iIndex;
		if (pSet->game.start_order == 1)
		{	//  reverse start order
			int numCars = pApp->mClient ? pApp->mClient->getPeerCount()+1  // networked
										: pSet->game.local_players;  // splitscreen
			startId = numCars-1 - startId;
		}
		int i = pSet->game.collis_cars ? startId : 0;  // offset when cars collide

		//  start pos
		auto st = pApp->scn->sc->GetStart(i, loop);
		MATHVECTOR<float,3> pos = st.first;
		QUATERNION<float> rot = st.second;
		
		vStartPos = Vector3(pos[0], pos[2], -pos[1]);
		if (pSet->game.trackreverse)
		{	rot.Rotate(PI_d, 0,0,1);  rot[0] = -rot[0];  rot[1] = -rot[1];  }

		pCar = pGame->LoadCar(pathCar, sDirname, pos, rot, true, eType == CT_REMOTE, iIndex);

		if (!pCar)  LogO("Error: Creating CAR: " + sDirname + "  path: " + pathCar);
		else  pCar->pCarM = this;
	}
}

//  Destroy
//------------------------------------------------------------------------------------------------------
void CarModel::Destroy()
{
	if (pNickTxt){  pApp->mGui->destroyWidget(pNickTxt);  pNickTxt = 0;  }

	delete pReflect;  pReflect = 0;
	delete fCam;  fCam = 0;

	int i,w,s;  //  trails
	for (w=0; w < numWheels; ++w)  if (whTrail[w]) {  whTemp[w] = 0.f;
		whTrail[w]->setVisible(false);	whTrail[w]->setInitialColour(0, 0.5,0.5,0.5, 0);
		mSceneMgr->destroyMovableObject(whTrail[w]);  }

	//  destroy cloned materials
	for (i=0; i < NumMaterials; ++i)
		if (MaterialManager::getSingleton().resourceExists(sMtr[i]))
			MaterialManager::getSingleton().remove(sMtr[i], resGrpId);

	s = vDelEnt.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroyEntity(vDelEnt[i]);
	vDelEnt.clear();
	
	s = vDelPar.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroyParticleSystem(vDelPar[i]);
	vDelPar.clear();
	
	s = vDelNd.size();
	for (i=0; i < s; ++i)  mSceneMgr->destroySceneNode(vDelNd[i]);
	vDelNd.clear();

	if (brakes)  mSceneMgr->destroyBillboardSet(brakes);
	brakes = 0;
	//if (pMainNode)  mSceneMgr->destroySceneNode(pMainNode);  //last?

	//  destroy resource group, will also destroy all resources in it
	if (ResourceGroupManager::getSingleton().resourceGroupExists(resGrpId))
		ResourceGroupManager::getSingleton().destroyResourceGroup(resGrpId);
}

CarModel::~CarModel()
{
	Destroy();
}

void CarModel::ToDel(SceneNode* nd){		vDelNd.push_back(nd);  }
void CarModel::ToDel(Entity* ent){			vDelEnt.push_back(ent);  }
void CarModel::ToDel(ParticleSystem* par){	vDelPar.push_back(par);  }


///   Load .car
//------------------------------------------------------------------------------------------------------
static void ConvertV2to1(float & x, float & y, float & z)
{
	float tx = x, ty = y, tz = z;
	x = ty;  y = -tx;  z = tz;
}
void CarModel::LoadConfig(const string & pathCar)
{
	Defaults();

	///  load  -----
	CONFIGFILE cf;
	if (!cf.Load(pathCar))
	{  LogO("Error: CarModel Can't load .car "+pathCar);  return;  }


	//  vehicle type
	vtype = V_Car;
	string drive;
	cf.GetParam("drive", drive);

	if (drive == "hover")  //>
		vtype = V_Spaceship;
	else if (drive == "sphere")
		vtype = V_Sphere;


	//  wheel count
	int nw = 0;
	cf.GetParam("wheels", nw);
	if (nw >= 2 && nw <= MAX_WHEELS)
		SetNumWheels(nw);


	//-  custom interior model offset
	cf.GetParam("model_ofs.interior-x", interiorOffset[0]);
	cf.GetParam("model_ofs.interior-y", interiorOffset[1]);
	cf.GetParam("model_ofs.interior-z", interiorOffset[2]);
	cf.GetParam("model_ofs.rot_fix", bRotFix);

	//~  boost offset
	cf.GetParam("model_ofs.boost-x", boostOffset[0]);
	cf.GetParam("model_ofs.boost-y", boostOffset[1]);
	cf.GetParam("model_ofs.boost-z", boostOffset[2]);
	cf.GetParam("model_ofs.boost-size-z", boostSizeZ);
	cf.GetParam("model_ofs.boost-name", sBoostParName);
	
	//  thruster  spaceship hover  max 4 pairs
	int i;
	for (i=0; i < PAR_THRUST; ++i)
	{
		string s = "model_ofs.thrust";
		if (i > 0)  s += toStr(i);
		cf.GetParam(s+"-x", thrusterOfs[i][0]);
		cf.GetParam(s+"-y", thrusterOfs[i][1]);
		cf.GetParam(s+"-z", thrusterOfs[i][2]);
		cf.GetParam(s+"-size-z", thrusterSizeZ[i]);
		cf.GetParam(s+"-name", sThrusterPar[i]);
	}
	

	//~  brake flares
	float pos[3];  bool ok=true;  i=0;
	while (ok)
	{	ok = cf.GetParam("flares.brake-pos"+toStr(i), pos);  ++i;
		if (ok)  brakePos.push_back(bRotFix ? Vector3(-pos[0],pos[2],pos[1]) : Vector3(-pos[1],-pos[2],pos[0]));
	}
	cf.GetParam("flares.brake-color", pos);
	brakeClr = ColourValue(pos[0],pos[1],pos[2]);
	cf.GetParam("flares.brake-size", brakeSize);
	
	
	//-  custom exhaust pos for boost particles
	if (cf.GetParam("model_ofs.exhaust-x", exhaustPos[0]))
	{
		manualExhaustPos = true;
		cf.GetParam("model_ofs.exhaust-y", exhaustPos[1]);
		cf.GetParam("model_ofs.exhaust-z", exhaustPos[2]);
	}else
		manualExhaustPos = false;
	if (!cf.GetParam("model_ofs.exhaust-mirror-second", has2exhausts))
		has2exhausts = false;


	//- load cameras pos
	cf.GetParamE("driver.view-position", pos);
	driver_view[0]=pos[1]; driver_view[1]=-pos[0]; driver_view[2]=pos[2];
	
	cf.GetParamE("driver.hood-position", pos);
	hood_view[0]=pos[1]; hood_view[1]=-pos[0]; hood_view[2]=pos[2];

	if (cf.GetParam("driver.ground-position", pos))
	{	ground_view[0]=pos[1]; ground_view[1]=-pos[0]; ground_view[2]=pos[2];  }
	else
	{	ground_view[0]=0.f; ground_view[1]=1.6; ground_view[2]=0.4f;  }

	cf.GetParam("driver.dist", camDist);


	//  tire params
	float val;
	bool both = cf.GetParam("tire-both.radius", val);

	int axles = std::max(2, numWheels/2);
	for (i=0; i < axles; ++i)
	{
		WHEEL_POSITION wl, wr;  string pos;
		CARDYNAMICS::GetWPosStr(i, numWheels, wl, wr, pos);
		if (both)  pos = "both";
		
		float radius;
		cf.GetParamE("tire-"+pos+".radius", radius);
		whRadius[wl] = radius;  whRadius[wr] = radius;
		
		float width = 0.2f;
		cf.GetParam("tire-"+pos+".width-trail", width);
		whWidth[wl] = width;  whWidth[wr] = width;
	}
	
	//  wheel pos
	//  for track's ghost or garage view
	int version = 2;
	cf.GetParam("version", version);
	for (i = 0; i < numWheels; ++i)
	{
		string sPos = sCfgWh[i];
		float pos[3];  MATHVECTOR<float,3> vec;

		cf.GetParamE("wheel-"+sPos+".position", pos);
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		vec.Set(pos[0],pos[1], pos[2]);
		
		whPos[i] = vec;
	}
	//  steer angle
	maxangle = 26.f;
	cf.GetParamE("steering.max-angle", maxangle);
	maxangle *= pGame->GetSteerRange();
}

	
//  log mesh stats
void CarModel::LogMeshInfo(const Entity* ent, const String& name, int mul)
{
	//return;
	const MeshPtr& msh = ent->getMesh();
	int tris=0, subs = msh->getNumSubMeshes();
	for (int i=0; i < subs; ++i)
	{
		SubMesh* sm = msh->getSubMesh(i);
		tris += sm->indexData->indexCount;
	}
	all_tris += tris * mul;  //wheels x4
	all_subs += subs * mul;
	LogO("MESH info:  "+name+"\t sub: "+toStr(subs)+"  tri: "+fToStr(tris/1000.f,1,4)+"k");
}

//  CreatePart mesh
//---------------------------------------------------
void CarModel::CreatePart(SceneNode* ndCar, Vector3 vPofs,
	String sCar2, String sCarI, String sMesh, String sEnt,
	bool ghost, uint32 visFlags,
	AxisAlignedBox* bbox, String stMtr, bool bLogInfo)
{
	// return;  ///!!!
	if (!FileExists(sCar2 + sMesh))  return;
	LogO("CreatePart " + sCarI + sEnt + " " + sDirname +  sMesh + " r "+  sCarI);
	Entity* ent =0;
	try
	{
		ent = mSceneMgr->createEntity(sCarI + sEnt, sDirname + sMesh, sCarI);
	}
	catch (Ogre::Exception ex)
	{
		LogO(String("## CreatePart exc! ") + ex.what());
	}
	ToDel(ent);

	if (bbox)  *bbox = ent->getBoundingBox();
	if (ghost)  {  ent->setRenderQueueGroup(RQG_CarGhost);  ent->setCastShadows(false);  }
	else  if (visFlags == RV_CarGlass)  ent->setRenderQueueGroup(RQG_CarGlass);
	ndCar->attachObject(ent);  ent->setVisibilityFlags(visFlags);
	if (bLogInfo)  LogMeshInfo(ent, sDirname + sMesh);
}


//-------------------------------------------------------------------------------------------------------
//  Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create()
{
	//if (!pCar)  return;

	String strI = toStr(iIndex)+ (eType == CT_TRACK ? "Z" : (eType == CT_GHOST2 ? "V" :""));
	mtrId = strI;
	String sCarI = "Car" + strI;
	//resGrpId = "General";  // ?
	resGrpId = sCarI;

	String sCars = PATHMANAGER::Cars() + "/" + sDirname;
	resCar = sCars + "/textures";
	String rCar = resCar + "/" + sDirname;
	String sCar = sCars + "/" + sDirname;
	
	bool ghost = false;  //isGhost();  //1 || for ghost test
	bool bLogInfo = !isGhost();  // log mesh info
	bool ghostTrk = isGhostTrk();
	
	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	ResourceGroupManager::getSingleton().createResourceGroup(resGrpId);
	ResourceGroupManager::getSingleton().addResourceLocation(sCars, "FileSystem", resGrpId);
	ResourceGroupManager::getSingleton().addResourceLocation(sCars + "/textures", "FileSystem", resGrpId);
		
	SceneNode* ndRoot = mSceneMgr->getRootSceneNode();
	pMainNode = ndRoot->createChildSceneNode();  ToDel(pMainNode);
	SceneNode* ndCar = pMainNode->createChildSceneNode();  ToDel(ndCar);

	//  --------  Follow Camera  --------
	if (mCamera && pCar)
	{
		fCam = new FollowCamera(mCamera, pSet);
		fCam->chassis = pCar->dynamics.chassis;
		fCam->loadCameras();
		
		//  set in-car camera position to driver position
		for (auto cam : fCam->mCameraAngles)
		{
			cam->mDist *= camDist;
			if (cam->mName == "Car driver")
				cam->mOffset = Vector3(driver_view[0], driver_view[2], -driver_view[1]);
			else if (cam->mName == "Car bonnet")
				cam->mOffset = Vector3(hood_view[0], hood_view[2], -hood_view[1]);
			else if (cam->mName == "Car ground")
				cam->mOffset = Vector3(ground_view[0], ground_view[2], -ground_view[1]);
	}	}
	
	CreateReflection();
	

	//  next checkpoint marker
	bool deny = pApp->gui->pChall && !pApp->gui->pChall->chk_beam;
	if (eType == CT_LOCAL && !deny && !pApp->bHideHudBeam)
	{
		entNextChk = mSceneMgr->createEntity("Chk"+strI, "check.mesh");  ToDel(entNextChk);
		entNextChk->setRenderQueueGroup(RQG_Weather);  entNextChk->setCastShadows(false);
		MaterialPtr mtr = MaterialManager::getSingleton().getByName("checkpoint_normal");
		entNextChk->setMaterial(mtr);
		ndNextChk = ndRoot->createChildSceneNode();  ToDel(ndNextChk);
		ndNextChk->attachObject(entNextChk);  entNextChk->setVisibilityFlags(RV_Hud);
		ndNextChk->setVisible(false);
	}


	///()  grass sphere test
	#if 0
	Entity* es = mSceneMgr->createEntity(sCarI+"s", "sphere.mesh");  ToDel(es);
	es->setRenderQueueGroup(RQG_CarGhost);
	MaterialPtr mtr = MaterialManager::getSingleton().getByName("pipeGlass");
	es->setMaterial(mtr);
	ndSph = ndRoot->createChildSceneNode();  ToDel(ndSph);
	ndSph->attachObject(es);
	#endif


	///  Create Models:  body, interior, glass
	//-------------------------------------------------
	Vector3 vPofs(0,0,0);
	AxisAlignedBox bodyBox;  uint8 g = RQG_CarGhost;
	all_subs=0;  all_tris=0;  //stats
	
	if (bRotFix)
		ndCar->setOrientation(Quaternion(Degree(90),Vector3::UNIT_Y)*Quaternion(Degree(180),Vector3::UNIT_X));


	//const String& res = "General"; //ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
	const String& res = resGrpId;
	CreatePart(ndCar, vPofs, sCar, res, "_body.mesh",     "",  ghost, RV_Car,  &bodyBox,  sMtr[Mtr_CarBody],  bLogInfo);

	vPofs = Vector3(interiorOffset[0],interiorOffset[1],interiorOffset[2]);  //x+ back y+ down z+ right
	if (!ghost)
	CreatePart(ndCar, vPofs, sCar, res, "_interior.mesh", "i", ghost, RV_Car,      0, sMtr[Mtr_CarBody]+"i",  bLogInfo);

	vPofs = Vector3::ZERO;
	CreatePart(ndCar, vPofs, sCar, res, "_glass.mesh",    "g", ghost, RV_CarGlass, 0, sMtr[Mtr_CarBody]+"g",  bLogInfo);
	

	//  wheels  ----------------------
	int w2 = numWheels==2 ? 1 : 2;
	// if (0)  ///!!!
	for (int w=0; w < numWheels; ++w)
	{
		String siw = "Wheel" + strI + "_" + toStr(w);
		ndWh[w] = ndRoot->createChildSceneNode();  ToDel(ndWh[w]);

		String sMesh = "_wheel.mesh";  // custom
		if (w <  w2  && FileExists(sCar + "_wheel_front.mesh"))  sMesh = "_wheel_front.mesh"; else  // 2|
		if (w >= w2  && FileExists(sCar + "_wheel_rear.mesh") )  sMesh = "_wheel_rear.mesh";  else
		if (w%2 == 0 && FileExists(sCar + "_wheel_left.mesh") )  sMesh = "_wheel_left.mesh";  else  // 2-
		if (w%2 == 1 && FileExists(sCar + "_wheel_right.mesh"))  sMesh = "_wheel_right.mesh";
		
		if (FileExists(sCar + sMesh))
		{
			String name = sDirname + sMesh;
			Entity* eWh = mSceneMgr->createEntity(siw, sDirname + sMesh, res);  ToDel(eWh);
			if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			if (bLogInfo && (w==0 || w==2))  LogMeshInfo(eWh, name, 2);
		}		
		if (FileExists(sCar + "_brake.mesh") && !ghostTrk)
		{
			String name = sDirname + "_brake.mesh";
			Entity* eBrake = mSceneMgr->createEntity(siw + "_brake", name, res);  ToDel(eBrake);
			if (ghost)  {  eBrake->setRenderQueueGroup(g);  eBrake->setCastShadows(false);  }
			ndBrake[w] = ndWh[w]->createChildSceneNode();  ToDel(ndBrake[w]);
			ndBrake[w]->attachObject(eBrake);  eBrake->setVisibilityFlags(RV_Car);
			if (bLogInfo && w==0)  LogMeshInfo(eBrake, name, 4);
		}
	}
	if (bLogInfo)  // all
		LogO("MESH info:  "+sDirname+"\t ALL sub: "+toStr(all_subs)+"  tri: "+fToStr(all_tris/1000.f,1,4)+"k");
	
	
	///  brake flares  ++ ++
	if (pCar)
	if (!brakePos.empty())
	{
		SceneNode* nd = ndCar->createChildSceneNode();  ToDel(nd);
		brakes = mSceneMgr->createBillboardSet("Flr"+strI,2);
		brakes->setDefaultDimensions(brakeSize, brakeSize);
		brakes->setRenderQueueGroup(RQG_CarTrails);
		brakes->setVisibilityFlags(RV_Car);

		for (int i=0; i < brakePos.size(); ++i)
			brakes->createBillboard(brakePos[i], brakeClr);

		brakes->setVisible(false);
		brakes->setMaterialName("flare1");
		nd->attachObject(brakes);
	}
	
	if (!ghostTrk)
	{
		//  Particles
		//-------------------------------------------------
		///  world hit sparks
		if (!parHit)
		{	parHit = mSceneMgr->createParticleSystem("Hit" + strI, "Sparks");  ToDel(parHit);
			parHit->setVisibilityFlags(RV_Particles);
			SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
			np->attachObject(parHit);
			parHit->getEmitter(0)->setEmissionRate(0);
		}
		
		///  boost emitters  ------------------------
		for (int i=0; i < PAR_BOOST; ++i)
		{
			String si = strI + "_" +toStr(i);
			if (!parBoost[i])
			{	parBoost[i] = mSceneMgr->createParticleSystem("Boost"+si, sBoostParName);  ToDel(parBoost[i]);
				parBoost[i]->setVisibilityFlags(RV_Particles);
				if (1)  // || !manualExhaustPos)
				{
					// no exhaust pos in car file, guess from bounding box
					Vector3 bsize = (bodyBox.getMaximum() - bodyBox.getMinimum())*0.5,
						bcenter = bodyBox.getMaximum() + bodyBox.getMinimum();
					//LogO("Car body bbox :  size " + toStr(bsize) + ",  center " + toStr(bcenter));
					Vector3 vp = bRotFix ?
						Vector3(bsize.z * 0.97, bsize.y * 0.65, bsize.x * 0.65 * (i==0 ? 1 : -1)) :
						Vector3(bsize.x * 0.97, bsize.y * 0.65, bsize.z * 0.65 * (i==0 ? 1 : -1));
						//Vector3(1.9 /*back*/, 0.1 /*up*/, 0.6 * (i==0 ? 1 : -1)/*sides*/
					vp.z *= boostSizeZ;
					vp += Vector3(boostOffset[0],boostOffset[1],boostOffset[2]);
					SceneNode* nb = pMainNode->createChildSceneNode(bcenter+vp);  ToDel(nb);
					nb->attachObject(parBoost[i]);
				}else
				{	// use exhaust pos values from car file
					Vector3 pos;
					if (i==0)
						pos = Vector3(exhaustPos[0], exhaustPos[1], exhaustPos[2]);
					else if (!has2exhausts)
						continue;
					else
						pos = Vector3(exhaustPos[0], exhaustPos[1], -exhaustPos[2]);

					SceneNode* nb = pMainNode->createChildSceneNode(pos);  ToDel(nb);
					nb->attachObject(parBoost[i]); 
				}
				parBoost[i]->getEmitter(0)->setEmissionRate(0);
		}	}

		///  spaceship thrusters ^  ------------------------
		for (int w=0; w < PAR_THRUST; ++w)
		if (!sThrusterPar[w].empty())
		{
			int i2 = thrusterSizeZ[w] > 0.f ? 2 : 1;
			for (int i=0; i < i2; ++i)
			{
				int ii = w*2+i;
				String si = strI + "_" +toStr(ii);
				
				if (!parThrust[ii])
				{	parThrust[ii] = mSceneMgr->createParticleSystem("Thrust"+si, sThrusterPar[w]);  ToDel(parThrust[ii]);
					parThrust[ii]->setVisibilityFlags(RV_Particles);

					Vector3 vp = Vector3(thrusterOfs[w][0],thrusterOfs[w][1],
						thrusterOfs[w][2] + (i-1)*2*thrusterSizeZ[w]);
					SceneNode* nb = pMainNode->createChildSceneNode(vp);  ToDel(nb);
					nb->attachObject(parThrust[ii]);
					parThrust[ii]->getEmitter(0)->setEmissionRate(0);
		}	}	}

		///  wheel emitters  ------------------------
		if (!ghost)
		{
			const static String sPar[PAR_ALL] = {"Smoke","Mud","Dust","FlWater","FlMud","FlMudS"};  // for ogre name
			//  particle type names
			const String sName[PAR_ALL] = {sc->sParSmoke, sc->sParMud, sc->sParDust, "FluidWater", "FluidMud", "FluidMudSoft"};
			for (int w=0; w < numWheels; ++w)
			{
				String siw = strI + "_" +toStr(w);
				//  particles
				for (int p=0; p < PAR_ALL; ++p)
				if (!par[p][w])
				{
					par[p][w] = mSceneMgr->createParticleSystem(sPar[p]+siw, sName[p]);  ToDel(par[p][w]);
					par[p][w]->setVisibilityFlags(RV_Particles);
					SceneNode* np = ndRoot->createChildSceneNode();  ToDel(np);
					np->attachObject(par[p][w]);
					par[p][w]->getEmitter(0)->setEmissionRate(0.f);
				}
				//  trails
				if (!ndWhE[w])
				{	ndWhE[w] = ndRoot->createChildSceneNode();  ToDel(ndWhE[w]);  }

				if (!whTrail[w])
				{	NameValuePairList params;
					params["numberOfChains"] = "1";
					params["maxElements"] = toStr(320 * pSet->trails_len);

					whTrail[w] = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
					whTrail[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
					whTrail[w]->setFaceCamera(false,Vector3::UNIT_Y);
					ndRoot->attachObject(whTrail[w]);
					whTrail[w]->setMaterialName("TireTrail");
					whTrail[w]->setCastShadows(false);
					whTrail[w]->addNode(ndWhE[w]);
				}
				whTrail[w]->setTrailLength(90 * pSet->trails_len);  //30
				whTrail[w]->setInitialColour(0, 0.1f,0.1f,0.1f, 0);
				whTrail[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08f * 1.f / pSet->trails_len);
				whTrail[w]->setInitialWidth(0, 0.f);
		}	}
		UpdParsTrails();
	}

	RecreateMaterials();
		
	setMtrNames();
	
	//  this snippet makes sure the brake texture is pre-loaded.
	//  since it is not used until you actually brake, we have to explicitely declare it
	/**/ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	if (FileExists(rCar + "_body00_brake.png") && !resMgr.resourceExists(resGrpId, sDirname + "_body00_brake.png"))
		resMgr.declareResource(sDirname + "_body00_brake.png", "Texture", resGrpId);
	if (FileExists(rCar + "_body00_add.png") && !resMgr.resourceExists(resGrpId, sDirname + "_body00_add.png"))
		resMgr.declareResource(sDirname + "_body00_add.png", "Texture", resGrpId);
	
	//  now just preload the whole resource group
	resMgr.initialiseResourceGroup(resGrpId);
	resMgr.loadResourceGroup(resGrpId);
	/**/
}


//-------------------------------------------------------------------------------------------------------
//  materials
//-------------------------------------------------------------------------------------------------------
void CarModel::RecreateMaterials()
{
	String sCar = resCar + "/" + sDirname;
	bool ghost = false;  //isGhost();  //1 || for ghost test
	
	// --------- Materials  -------------------
	
	// if specialised car material (e.g. car_body_FM) exists, use this one instead of e.g. car_body
	// useful macro for choosing between these 2 variants
	#define chooseMat(s)  MaterialManager::getSingleton().resourceExists( \
		"car"+String(s) + "_"+sDirname) ? "car"+String(s) + "_"+sDirname : "car"+String(s)

	//  ghost car has no interior, particles, trails and uses same material for all meshes
	if (!ghost)
	{	sMtr[Mtr_CarBody]     = chooseMat("_body");
		sMtr[Mtr_CarBrake]    = chooseMat("_glass");
	}else
	for (int i=0; i < NumMaterials; ++i)
		sMtr[i] = "car_ghost";

	//  copy material to a new material with index
	//  only for car body to have different colors
	MaterialPtr mat;
	for (int i=0; i < 1/*NumMaterials*/; ++i)
	{
		sh::Factory::getInstance().destroyMaterialInstance(sMtr[i] + mtrId);
		sh::MaterialInstance* m = sh::Factory::getInstance().createMaterialInstance(sMtr[i] + mtrId, sMtr[i]);

		m->setListener(this);

		// change textures for the car
		if (m->hasProperty("diffuseMap"))
		{
			string v = sh::retrieveValue<sh::StringValue>(m->getProperty("diffuseMap"), 0).get();
			m->setProperty("diffuseMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		if (m->hasProperty("carPaintMap"))
		{
			string v = sh::retrieveValue<sh::StringValue>(m->getProperty("carPaintMap"), 0).get();
			m->setProperty("carPaintMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		if (m->hasProperty("reflMap"))
		{
			string v = sh::retrieveValue<sh::StringValue>(m->getProperty("reflMap"), 0).get();
			m->setProperty("reflMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		sMtr[i] = sMtr[i] + mtrId;
	}

	//ChangeClr();

	UpdateLightMap();
}

void CarModel::setMtrName(const String& entName, const String& mtrName)
{
	if (mSceneMgr->hasEntity(entName))
		mSceneMgr->getEntity(entName)->setMaterialName(mtrName);
	else
	if (mSceneMgr->hasManualObject(entName))
		mSceneMgr->getManualObject(entName)->setMaterialName(0, mtrName);
}

void CarModel::setMtrNames()
{
	//if (FileExists(resCar + "/" + sDirname + "_body00_add.png") ||
	//	FileExists(resCar + "/" + sDirname + "_body00_red.png"))
	setMtrName("Car"+mtrId, sMtr[Mtr_CarBody]);

	#if 0
	setMtrName("Car.interior"+mtrI, sMtr[Mtr_CarInterior]);
	setMtrName("Car.glass"+mtrI, sMtr[Mtr_CarGlass]);

	for (int w=0; w < numWheels; ++w)
	{
		String sw = "Wheel"+mtrI+"_"+toStr(w), sm = w < 2 ? sMtr[Mtr_CarTireFront] : sMtr[Mtr_CarTireRear];
		setMtrName(sw,          sm);
		setMtrName(sw+"_brake", sm);
	}
	#endif
}

//  ----------------- Reflection ------------------------
void CarModel::CreateReflection()
{
	char suffix = (eType == CT_TRACK ? 'Z' : (eType == CT_GHOST2 ? 'V' :' '));
	pReflect = new CarReflection(pSet, pApp, mSceneMgr, iIndex, suffix);
	for (int i=0; i < NumMaterials; ++i)
		pReflect->sMtr[i] = sMtr[i];

	pReflect->Create();
}

void CarModel::requestedConfiguration(sh::MaterialInstance* m, const string& configuration)
{
}

void CarModel::createdConfiguration(sh::MaterialInstance* m, const string& configuration)
{
	UpdateLightMap();
	ChangeClr();
}
