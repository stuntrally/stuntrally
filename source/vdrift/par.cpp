#include "pch.h"
#include "par.h"

SParams::SParams()
{
/// _Tool_ go back time rewind (1 for making best ghosts), save ghost always
	backTime = 0;  //0 !in release
//  SIM
	raylen = 1.5f;  // wheel ray cast length
//  GAME
	rewindSpeed = 5.f;  // 5 secs in 1 sec

/// <><> Damage factors <><>
	dmgFromHit  = 0.5f;  dmgFromScrap  = 1.0f;   // reduced
	dmgFromHit2 = 1.5f;  dmgFromScrap2 = 11.5f;  // normal
	dmgPow2 = 1.4f;

//  start pos next car distance
	startNextDist = 6.f;

//  HUD
//  time in sec to show wrong check warning
	timeShowChkWarn = 2.f;
	timeWonMsg = 4.f;
	fadeLapResults = 0.1f;  //0.04f;

//  chk beam size
	chkBeamSx = 5.f;  chkBeamSy = 44.f;
//  ghost
	ghostHideTime = 0.2f;
	ghostHideDist    = 4.f*4.f;
	ghostHideDistTrk = 5.f*5.f;

//  camera bounce, force factors
	camBncF  = -0.0016f;  camBncFo = -0.0016f;  camBncFof = -0.0016f;
	camBncFHit = 1500.f;  camBncFHitY = 1000.f;
	camBncSpring = -1500.f;  camBncDamp = -150.f;
	camBncScale = 10.f;  camBncMass = 0.02f;
}

//  for other params search //par
