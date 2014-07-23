#pragma once

//  for car prv screens
//#define CAR_PRV

///  const game params  -------
struct SParams
{
	float
//  SIM
	 raylen
//  GAME
	,rewindSpeed

//  damage factors
	,dmgFromHit, dmgFromScrap    // reduced
	,dmgFromHit2, dmgFromScrap2  // normal
	,dmgPow2

//  start pos, next car distance
	,startNextDist

//  HUD
//  time in sec
	,timeShowChkWarn
	,timeWonMsg
	,fadeLapResults

//  chk beam size
	,chkBeamSx, chkBeamSy
//  ghost
	,ghostHideTime
	,ghostHideDist
	,ghostHideDistTrk

//  camera bounce sim
	,camBncF, camBncFo, camBncFof
	,camBncFHit,camBncFHitY
	,camBncSpring, camBncDamp
	,camBncScale, camBncMass
	;

//  take back time in rewind (for track's ghosts)
	bool backTime;

	//  ctor, init values
	SParams();
};

static SParams gPar;
