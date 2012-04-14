#include "pch.h"
#include "common/Defines.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/track.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "SplitScreen.h"
#include "common/SceneXml.h"
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../road/Road.h"
#include "common/RenderConst.h"
#include "common/MaterialGen/MaterialFactory.h"

#include "boost/filesystem.hpp"
#define  FileExists(s)  boost::filesystem::exists(s)

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreEntity.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreRibbonTrail.h>

using namespace Ogre;


void CarModel::setVisible(bool vis)
{
	mbVisible = vis;
	//if (pMainNode->getVisible() == vis)  return;  //opt..
	hideTime = 0.f;
	pMainNode->setVisible(vis);
	for (int w=0; w < 4; ++w)
		ndWh[w]->setVisible(vis);
	UpdParsTrails(vis);
}

void CarModel::ResetChecks(bool bDist)  // needs to be done after road load!
{
	iCurChk = -1;  iNumChks = 0;  // reset lap, chk vars
	trackPercent = 0.f;
	if (!pApp || !pApp->road)  return;
	
	const SplineRoad* road = pApp->road;
	iNextChk = pSet->game.trackreverse ? road->iChkId1Rev : road->iChkId1;

	//  percent const  ------
	if (bDist && !road->mChks.empty())
	{
		const Vector3& firstC = road->mChks[road->iChkId1].pos, lastC = road->mChks[road->iChkId1Rev].pos;

		Vector3 vFirst = vStartPos - firstC;  distFirst = vFirst.length();
		Vector3 vLast  = lastC - vStartPos;   distLast = vLast.length();
		distTotal = distFirst + distLast + road->chksRoadLen;
		//LogO("Chk first: "+toStr(distFirst)+" last: "+toStr(distLast)+" total: "+toStr(distTotal));
	}
}

//  get track driven dist part in %
//--------------------------------------------------------------------------------------------------------
void CarModel::UpdTrackPercent()
{
	if (!pApp || !pApp->road)  return;
	const SplineRoad* road = pApp->road;
	
	float perc = 0.f;
	if (road && !road->mChks.empty() && eType != CarModel::CT_GHOST)
	{
		const Vector3& car = pMainNode->getPosition(), next = road->mChks[iNextChk].pos,
			start = vStartPos, curr = road->mChks[std::max(0,iCurChk)].pos;
		bool bRev = pSet->game.trackreverse;
		Real firstD = bRev ? distLast : distFirst;
		Real nextR = road->mChks[iNextChk].r;  // chk .r radius to tweak when entering chk..

		Real dist = 0.f;
		if (iNumChks > 0)  dist = firstD;  // already after 1st chk
		if (iNumChks > 1)  dist +=  // after 1st to 2nd chk or more
			road->mChks[iNumChks-2].dist[bRev ? 1 : 0];
							
		
		float dist01 = 0.f;  // smooth dist part
		//  start to 1st chk
		if (iNumChks == 0)
		{
			Vector3 curDist  = car - start;
			Vector3 chksDist = next - start;  // first
			dist01 = (curDist.length() /*- nextR*/) / (chksDist.length() - nextR);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * firstD;
		}
		//  last chk to finish
		else if (iNumChks == road->mChks.size())
		{
			Vector3 curDist  = start - car;
			Vector3 chksDist = curr - start;  // last
			dist01 = 1.f - (curDist.length() /*- nextR*/) / (chksDist.length() - nextR);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * (bRev ? distFirst : distLast);  //lastD;
		}
		else  // between 2 checks
		{
			Vector3 curDist  = car  - next;   // car dist to next checkpoint
			Vector3 chksDist = curr - next;  // divide by (cur to next) checks distance
			Real ckD = chksDist.length();

			dist01 = 1.f - (curDist.length() - nextR) / (ckD - road->mChks[iCurChk].r);

			float percD = std::min(1.f, std::max(0.f, dist01 ));  // clamp to 0..1
			dist += percD * (ckD + road->mChks[iCurChk].r*0.8f);  //road->mChks[iNumChks-1].dist;
		}
		perc = 100.f * dist / distTotal;

		if (perc > trackPercent)
			trackPercent = perc;
	}
}


//-------------------------------------------------------------------------------------------------------
//  Update
//-------------------------------------------------------------------------------------------------------
void CarModel::Update(PosInfo& posInfo, PosInfo& posInfoCam, float time)
{	
	//if (!posInfo.bNew)  return;  // new only ?
	//posInfo.bNew = false;
	/// dont get anything from pCar or car.dynamics here
	/// all must be read from posInfo (it is filled from vdrift car or from replay)
	
	if (!pMainNode) return;

	//  set car pos and rot
	pMainNode->setPosition(posInfo.pos);
	pMainNode->setOrientation(posInfo.rot);

	//  set camera view
	if (fCam)
		fCam->Apply(posInfoCam/*posInfo*/);


	//  upd rotY for minimap
	Quaternion q = posInfo.rot * Quaternion(Degree(90),Vector3(0,1,0));
	angCarY = q.getYaw().valueDegrees() + 90.f;
	
	//  brake state
	bool braking = posInfo.braking;
	if (bBraking != braking)
	{
		bBraking = braking;
		RefreshBrakingMaterial();
	}
	
	//  terrain lightmap enable/disable (depending on distance to terrain)
	#define MAX_TERRAIN_DIST 2.0 // meters
	bool changed = false;
	if (terrain)
	{
		Ogre::Vector3 carPos = pMainNode->getPosition();
		float terrainHeight = terrain->getHeightAtWorldPosition(carPos);
		float diff = std::abs(carPos.y - terrainHeight);
		if (diff > MAX_TERRAIN_DIST)
		{
			if (bLightMapEnabled)
			{
				changed = true;
				bLightMapEnabled = false;
			}
		}
		else if (!bLightMapEnabled)
		{
			changed = true;
			bLightMapEnabled = true;
		}
	}
	//  if no terrain, disable
	else if (bLightMapEnabled)
	{
		changed = true; bLightMapEnabled = false;
	}
	
	if (changed)
		UpdateLightMap();
		

	//  update particle emitters
	//  boost
	if (pSet->particles)
	for (int i=0; i < 2; i++)
	if (pb[i])
	{
		float emitB = posInfo.fboost * 40.f;  // par
		ParticleEmitter* pe = pb[i]->getEmitter(0);
		pe->setEmissionRate(emitB);
	}

	//  world hit
	if (ph)
	{	ParticleEmitter* pe = ph->getEmitter(0);
		if (posInfo.fHitTime > 0.f && pSet->particles)
		{
			pe->setPosition(posInfo.vHitPos);
			pe->setDirection(posInfo.vHitNorm);

			pe->setEmissionRate(pSet->particles_len * std::min(160.f, posInfo.fParIntens) * posInfo.fHitTime);
			pe->setParticleVelocity(posInfo.fParVel);
		}else
			pe->setEmissionRate(0.f);
	}

	//  wheels  ------------------------------------------------------------------------
	for (int w=0; w < 4; ++w)
	{
		float wR = posInfo.whR[w];
		ndWh[w]->setPosition(posInfo.whPos[w]);
		ndWh[w]->setOrientation(posInfo.whRot[w]);

		int whMtr = posInfo.whTerMtr[w];
		int whRd = posInfo.whRoadMtr[w];
		float whVel = posInfo.whVel[w] * 3.6f;  //kmh
		float slide = posInfo.whSlide[w], squeal = posInfo.whSqueal[w];
		float onGr = slide < 0.f ? 0.f : 1.f;

		//  wheel temp
		wht[w] += squeal * time * 7;
		wht[w] -= time*6;  if (wht[w] < 0.f)  wht[w] = 0.f;

		///  emit rates +
		Real emitS = 0.f, emitM = 0.f, emitD = 0.f;  //paused

		if (!pGame->pause)
		{
			Real sq = squeal* std::min(1.f, wht[w]), l = pSet->particles_len * onGr;
			emitS = sq * (whVel * 30) * l *0.3f;  //..
			emitM = slide < 1.4f ? 0.f :  (8.f * sq * std::min(5.f, slide) * l);
			emitD = (std::min(140.f, whVel) / 3.5f + slide * 1.f ) * l;  

			if (pd[w])  {	//  resume
				pd[w]->setSpeedFactor(1.f);  ps[w]->setSpeedFactor(1.f);  pm[w]->setSpeedFactor(1.f);
			if (w < 2)  pb[w]->setSpeedFactor(1.f);  }
			if (pflW[w])  {
				pflW[w]->setSpeedFactor(1.f);  pflM[w]->setSpeedFactor(1.f);  pflMs[w]->setSpeedFactor(1.f);  }
			if (ph)  ph->setSpeedFactor(1.f);
		}else{
			if (pd[w])  {	//  stop par sys
				pd[w]->setSpeedFactor(0.f);  ps[w]->setSpeedFactor(0.f);  pm[w]->setSpeedFactor(0.f);
			if (w < 2)  pb[w]->setSpeedFactor(0.f);  }
			if (pflW[w])  {
				pflW[w]->setSpeedFactor(0.f);  pflM[w]->setSpeedFactor(0.f);  pflMs[w]->setSpeedFactor(0.f);  }
			if (ph)  ph->setSpeedFactor(0.f);
			//if (whTrl[w])
			//	whTrl[w]->setFade 0
		}
		Real sizeD = (0.3f + 1.1f * std::min(140.f, whVel) / 140.f) * (w < 2 ? 0.5f : 1.f);

		//  ter mtr factors
		int mtr = std::max(0, std::min(whMtr-1, (int)(sc->td.layers.size()-1)));
		TerLayer& lay = whMtr==0 ? sc->td.layerRoad : sc->td.layersAll[sc->td.layers[mtr]];
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		if (whRd == 2)  emitD = 0;  // no dust in pipes
		if (posInfo.whH[w] > 0.1f)  emitD = 0;  // no dust in fluids

		bool ghost = eType == CT_GHOST;  // opt dis for ghost
		bool ghPar = !(ghost && !pSet->rpl_ghostpar);
		if (!ghPar)
		{	emitD = 0.f;  emitM = 0.f;  emitS = 0.f;  }

		///  emit particles
		Vector3 vpos = posInfo.whPos[w];
		if (pSet->particles)
		{
			if (ps[w] && sc->td.layerRoad.smoke > 0.f/*&& !sc->ter*/)  // only at vdr road
			{			//  smoke
				ParticleEmitter* pe = ps[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f);
				/**/ps[w]->getAffector(0)->setParameter("alpha", toStr(-0.4f - 0.07f/2.4f * whVel));
				/**/pe->setTimeToLive( std::max(0.1, 2 - whVel/2.4f * 0.04) );  // fade,live
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitS);
			}
			if (pm[w])	//  mud
			{	ParticleEmitter* pe = pm[w]->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM);
			}
			if (pd[w])	//  dust
			{	pd[w]->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = pd[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitD);
			}

			//  fluids .::.
			int idPar = posInfo.whP[w];
			if (pflW[w])  //  Water ~
			{
				float vel = posInfo.speed;  // depth.. only on surface?
				bool e = idPar == 0 && ghPar &&  vel > 10.f && posInfo.whH[w] < 1.f;
				float emitW = e ?  std::min(80.f, 3.0f * vel)  : 0.f;

				ParticleEmitter* pe = pflW[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitW * pSet->particles_len);
			}
			if (pflM[w])  //  Mud ^
			{
				float vel = Math::Abs(posInfo.whAngVel[w]);
				bool e = idPar == 2 && ghPar &&  vel > 30.f;
				float emitM = e ?  posInfo.whH[w] * std::min(80.f, 1.5f * vel)  : 0.f;

				ParticleEmitter* pe = pflM[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM * pSet->particles_len);
			}
			if (pflMs[w])  //  Mud soft ^
			{
				float vel = Math::Abs(posInfo.whAngVel[w]);
				bool e = idPar == 1 && ghPar &&  vel > 30.f;
				float emitM = e ?  posInfo.whH[w] * std::min(160.f, 3.f * vel)  : 0.f;

				ParticleEmitter* pe = pflMs[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f);
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM * pSet->particles_len);
			}
		}

		//  update trails h+
		if (pSet->trails)  {
			if (ndWhE[w])
			{	Vector3 vp = vpos + posInfo.carY * wR*0.72f;  // 0.22
				if (terrain && whMtr > 0)
					vp.y = terrain->getHeightAtWorldPosition(vp) + 0.05f;
					//if (/*whOnRoad[w]*/whMtr > 0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
			}
			float al = 0.5f * /*squeal*/ std::min(1.f, 0.7f * wht[w]) * onGr;  // par+
			if (whTrl[w])	whTrl[w]->setInitialColour(0,
				lay.tclr.r,lay.tclr.g,lay.tclr.b, lay.tclr.a * al/**/);
		}
	}

	// Reflection
	pReflect->camPosition = pMainNode->getPosition();
	
	// blendmaps
	UpdWhTerMtr();
	
	//  update brake meshes orientation
	for (int w=0; w<4; ++w)
	{
		if (ndBrake[w])
		{
			ndBrake[w]->_setDerivedOrientation( pMainNode->getOrientation() );
			
			// this transformation code is just so the brake mesh can have the same alignment as the wheel mesh
			ndBrake[w]->yaw(Ogre::Degree(-90), Node::TS_LOCAL);
			if (w%2 == 1)
				ndBrake[w]->setScale(-1, 1, 1);
				
			ndBrake[w]->pitch(Ogre::Degree(180), Node::TS_LOCAL);
			
			if (w < 2)  // turn only front wheels
				ndBrake[w]->yaw(-Degree(posInfo.whSteerAng[w]));
		}
	}
	
	UpdateKeys();
}

//-------------------------------------------------------------------------------------------------------
void CarModel::UpdateKeys()
{
	if (!pCar)  return;

	///  goto last checkp - reset cam
	if (pCar->bLastChk && !bLastChkOld)
	{
		if (fCam)
			fCam->first = true;
		else
			LogO("no fCam");
	}
	bLastChkOld = pCar->bLastChk;

	///  change Cameras  ---------------------------------
	//if (!pApp->isFocGui)
	if (pCar->iCamNext != 0 && iCamNextOld == 0)
	{
		//  with ctrl - change current camera car index  (mouse move camera for many players)
		if (pApp->ctrl && iIndex == 0)
			pApp->iCurCam = (pApp->iCurCam + pCar->iCamNext + pSet->game.local_players) % pSet->game.local_players;
		else
		{
			int visMask = 255;
			pApp->roadUpdTm = 1.f;

			if (fCam)
			{	fCam->Next(pCar->iCamNext < 0, pApp->shift);
				pApp->carsCamNum[iIndex] = fCam->miCurrent +1;  // save for pSet
				visMask = fCam->ca->mHideGlass ? RV_MaskAll-RV_CarGlass : RV_MaskAll;
				for (std::list<Viewport*>::iterator it = pApp->mSplitMgr->mViewports.begin();
					it != pApp->mSplitMgr->mViewports.end(); ++it)
					(*it)->setVisibilityMask(visMask);
			}


		}
	}
	iCamNextOld = pCar->iCamNext;
}



//-------------------------------------------------------------------------------------------------------
//  utility
//-------------------------------------------------------------------------------------------------------
void CarModel::UpdateLightMap()
{
	MaterialPtr mtr;
	for (int i=0; i < NumMaterials; i++)
	{
		mtr = MaterialManager::getSingleton().getByName(sMtr[i]);
		if (!mtr.isNull())
		{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
			while (techIt.hasMoreElements())
			{	Technique* tech = techIt.getNext();
				Technique::PassIterator passIt = tech->getPassIterator();
				while (passIt.hasMoreElements())
				{
					Pass* pass = passIt.getNext();

					if (pass->hasFragmentProgram())
					{
						GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
						params->setIgnoreMissingParams(true); // don't throw exception if material doesnt use lightmap
						params->setNamedConstant("enableTerrainLightMap", bLightMapEnabled ? Real(1) : Real(0));
					}
	}	}	}	}
}

void CarModel::RefreshBrakingMaterial()
{
	std::string texName;
	if (bBraking)
	texName = sDirname + "_body00_brake.png";
	else
		texName = sDirname + "_body00_add.png";
	MaterialPtr mtr;
	for (int i=0; i < NumMaterials; i++)
	{
		mtr = MaterialManager::getSingleton().getByName(sMtr[i]);
		if (!mtr.isNull())
		{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
			while (techIt.hasMoreElements())
			{	Technique* tech = techIt.getNext();
				Technique::PassIterator passIt = tech->getPassIterator();
				while (passIt.hasMoreElements())
				{	Pass* pass = passIt.getNext();
					Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
					while (tusIt.hasMoreElements())
					{
						TextureUnitState* tus = tusIt.getNext();

						if (tus->getName() == "blendMap")
							tus->setTextureName( texName );
	}	}	}	}	}
}


void CarModel::UpdParsTrails(bool visible)
{
	bool vis = visible && pSet->particles;
	for (int w=0; w < 4; ++w)
	{
		Ogre::uint8 grp = RQG_CarTrails;  //9=road  after glass
		if (w < 2 &&
			pb[w])	{	pb[w]->setVisible(vis);  pb[w]->setRenderQueueGroup(grp);  }
		if (whTrl[w]){  whTrl[w]->setVisible(visible && pSet->trails);  whTrl[w]->setRenderQueueGroup(grp);  }
		grp = RQG_CarParticles;
		if (ps[w])	{	ps[w]->setVisible(vis);  ps[w]->setRenderQueueGroup(grp);  }  // vdr only && !sc.ter
		if (pm[w])	{	pm[w]->setVisible(vis);  pm[w]->setRenderQueueGroup(grp);  }
		if (pd[w])	{	pd[w]->setVisible(vis);  pd[w]->setRenderQueueGroup(grp);  }

		if (pflW[w]){	pflW[w]->setVisible(vis);  pflW[w]->setRenderQueueGroup(grp);  }
		if (pflM[w]){	pflM[w]->setVisible(vis);  pflM[w]->setRenderQueueGroup(grp);  }
		if (pflMs[w]){	pflMs[w]->setVisible(vis);  pflMs[w]->setRenderQueueGroup(grp);  }
		if (ph)		{	ph->setVisible(vis);     ph->setRenderQueueGroup(grp);     }
	}
}


///  terrain mtr from blend maps
//-------------------------------------------------------------------------------------------------------
void CarModel::UpdWhTerMtr()
{
	if (!pCar || !ndWh[0])  return;
	if (!terrain || !blendMtr)	// vdr trk
	{
		for (int i=0; i<4; ++i)  // for particles/trails only
		{	whTerMtr[i] = pCar->dynamics.bWhOnRoad[i] ? 0 : 1;
			whRoadMtr[i] = pCar->dynamics.bWhOnRoad[i];  }
		return;
	}
	// if whTerMtr == 0 wheel is on road and mtr is in whRoadMtr (now only for road/pipe)
	// TODO: road has only 1 surface, extend to 4, editor tabs, surfaces.txt, alpha transition?...

	int t = blendMapSize;
	Real tws = sc->td.fTerWorldSize;

	//  wheels
	for (int i=0; i<4; ++i)
	{
		Vector3 w = ndWh[i]->getPosition();
		int mx = (w.x + 0.5*tws)/tws*t, my = (w.z + 0.5*tws)/tws*t;
		mx = std::max(0,std::min(t-1, mx)), my = std::max(0,std::min(t-1, my));

		int mtr = pCar->dynamics.bWhOnRoad[i] ? 0 : blendMtr[my*t + mx];
		whTerMtr[i] = mtr;
		whRoadMtr[i] = pCar->dynamics.bWhOnRoad[i];

		///  vdr set surface for wheel
		TRACKSURFACE* tsu = &pGame->track.tracksurfaces[mtr];
		pCar->dynamics.terSurf[i] = tsu;
		pCar->dynamics.bTerrain = true;
	}
}


//  utils
//-------------------------------------------------------------------------------------------------------

void CarModel::ChangeClr(int car)
{
	float c_h = pSet->gui.car_hue[car], c_s = pSet->gui.car_sat[car], c_v = pSet->gui.car_val[car];
	color.setHSB(1-c_h,c_s,c_v);  //set, mini pos clr
	MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr[Mtr_CarBody]);
	if (!mtr.isNull())
		mtr->setDiffuse(color);

	if (pNickTxt)
		pNickTxt->setTextColour(MyGUI::Colour(color.r,color.g,color.b));
}
