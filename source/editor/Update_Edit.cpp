#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
using namespace Ogre;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const FrameEvent& evt)
{
	if (!BaseApp::frameRenderingQueued(evt))
		return false;

	//  pos on minimap *
	if (ndPos)
	{
		Real x = (0.5 - mCamera->getPosition().z / sc.td.fTerWorldSize);
		Real y = (0.5 + mCamera->getPosition().x / sc.td.fTerWorldSize);
		ndPos->setPosition(xm1+(xm2-xm1)*x, ym1+(ym2-ym1)*y, 0);
		///  hud rpm,vel  --------------------------------
		float angrot = mCamera->getOrientation().getYaw().valueDegrees();
		float psx = 0.9f * pSet->size_minimap, psy = psx*asp;  // *par len

		const static float d2r = PI_d/180.f;
		static float px[4],py[4];
		for (int i=0; i<4; i++)
		{
			float ia = 135.f + float(i)*90.f;
			float p = -(angrot + ia) * d2r;
			px[i] = psx*cosf(p);  py[i] =-psy*sinf(p);
		}
		if (mpos)  {	mpos->beginUpdate(0);
			mpos->position(px[0],py[0], 0);  mpos->textureCoord(0, 1);	mpos->position(px[1],py[1], 0);  mpos->textureCoord(1, 1);
			mpos->position(px[2],py[2], 0);  mpos->textureCoord(1, 0);	mpos->position(px[3],py[3], 0);  mpos->textureCoord(0, 0);
			mpos->end();  }
	}
	
	//  status overlay
	if (fStFade > 0.f)
	{	fStFade -= evt.timeSinceLastFrame;
		//Real a = std::min(1.0f, fStFade*0.9f);
		//ColourValue cv(0.0,0.5,a, a );
		//ovStat->setColour(cv);	ovSt->setColour(cv);
		if (fStFade <= 0.f)
		{	ovSt->hide();	ovSt->setMaterialName("");  }
	}

	#define isKey(a)  mKeyboard->isKeyDown(OIS::KC_##a)
	const Real q = (shift ? 0.05 : ctrl ? 4.0 :1.0) * 20 * evt.timeSinceLastFrame;


	// key,mb info  ==================
	if (mStatsOn)
	{
		String ss="";
		if (shift)  ss += "Shift ";	if (mbLeft)  ss += "LMB ";
		if (ctrl)  ss += "Ctrl ";	if (mbRight)  ss += "RMB ";
		if (alt)  ss += "Alt ";		if (mbMiddle)  ss += "MMB ";

		using namespace OIS;
		for (int i=KC_DIVIDE; i > 0; --i)
		{	OIS::KeyCode k = (OIS::KeyCode)i;
			if (k != KC_LSHIFT && k != KC_RSHIFT &&   //opt=
				k != KC_LCONTROL && k != KC_RCONTROL && 
				k != KC_LMENU && k != KC_RMENU)
			if (mKeyboard->isKeyDown(k))
			{
				 if (k == KC_DIVIDE)	ss += "Num / ";		else if (k == KC_MULTIPLY)	ss += "Num * ";
			else if (k == KC_ADD)		ss += "Num + ";		else if (k == KC_SUBTRACT)	ss += "Num - ";
			else
				{	 if (k == KC_NUMPAD0)  k = KC_INSERT;	else if (k == KC_DECIMAL)  k = KC_DELETE;
				else if (k == KC_NUMPAD1)  k = KC_END;		else if (k == KC_NUMPAD2)  k = KC_DOWN;
				else if (k == KC_NUMPAD3)  k = KC_PGDOWN;	else if (k == KC_NUMPAD4)  k = KC_LEFT;
				else if (k == KC_NUMPAD5)  k = KC_DELETE;	else if (k == KC_NUMPAD6)  k = KC_RIGHT;
				else if (k == KC_NUMPAD7)  k = KC_HOME;		else if (k == KC_NUMPAD8)  k = KC_UP;
				else if (k == KC_NUMPAD9)  k = KC_PGUP;
					ss += mKeyboard->getAsString(k) + " ";
		}	}	}
		
		static int mzd = 0;
		if (mz != 0)  mzd = 30;
		if (mzd > 0)  {  ss += "Wheel ";  --mzd;  }
		//ovInfo->setCaption(ss);
		ovDbg->setCaption(ss);
	}


	//  keys dn/up - trklist
	static float dirU = 0.f,dirD = 0.f;
	if (bGuiFocus)
	{	if (isKey(UP)  ||isKey(NUMPAD8))	dirD += evt.timeSinceLastFrame;  else
		if (isKey(DOWN)||isKey(NUMPAD2))	dirU += evt.timeSinceLastFrame;  else
		{	dirU = 0.f;  dirD = 0.f;  }
		int d = ctrl ? 4 : 1;
		if (dirU > 0.0f) {  trkListNext( d);  dirU = -0.12f;  }
		if (dirD > 0.0f) {  trkListNext(-d);  dirD = -0.12f;  }
	}

	
	///  Update Info texts  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	if (edMode == ED_Road && bEdit() && road)
	{
		int ic = road->iChosen;  bool bCur = ic >= 0;
		SplinePoint& sp = bCur ? road->getPoint(ic) : road->newP;

		//  road point  --------------------------------
		if (rdTxt[0]){	if (sp.onTer)	Fmt(s, "On Terrain");
					else Fmt(s, "Height  %5.2f", sp.pos.y );  rdTxt[0]->setCaption(s);  }
		if (rdTxt[1]){  Fmt(s, "Width  %5.2f", sp.width);  rdTxt[1]->setCaption(s);  }

		if (rdTxt[2]){  Fmt(s, "yaw   %5.1f",  sp.aYaw);    rdTxt[2]->setCaption(s);  }
		if (rdTxt[3]){  Fmt(s, "roll  %5.1f", sp.aRoll);   rdTxt[3]->setCaption(s);  }

		if (rdTxt[4]){  Fmt(s, "%d %s", sp.aType, csAngType[sp.aType].c_str());   rdTxt[4]->setCaption(s);  }
		if (rdTxt[13]){  Fmt(s, "%3.0f", angSnap);   rdTxt[13]->setCaption(s);  }

		if (rdTxt[5]){	if (sp.pipe==0.f)	s[0]=0;
					else Fmt(s, "Pipe  %5.2f", sp.pipe);   rdTxt[5]->setCaption(s);  }
		if (rdTxt[6]){	if (sp.onTer)	s[0]=0;
					else Fmt(s, "column  %2d", sp.cols);   rdTxt[6]->setCaption(s);  }
		if (rdTxt[7]){  Fmt(s, "%d %s", sp.idMtr, road->getMtrStr(ic).c_str());   rdTxt[7]->setCaption(s);  }

		if (rdTxt[8]){	if (sp.chkR == 0.f)  s[0]=0;
					else Fmt(s, "chkR  %4.2f", sp.chkR);   rdTxt[8]->setCaption(s);  }

		if (rdTxt[9]){
			if (road->vSel.size() > 0)  Fmt(s, "sel: %d", road->vSel.size());
			else	Fmt(s, "%2d/%d", road->iChosen+1, road->vSegs.size());   rdTxt[9]->setCaption(s);  }

		if (rdTxt[11]){  rdTxt[11]->setCaption(bCur ? "Cur" : "New");
			rdTxt[11]->setTextColour(bCur ? MyGUI::Colour(0.85,0.75,1) : MyGUI::Colour(0.3,1,0.1));  }
		if (rdTxt[12]){  rdTxt[12]->setCaption(road->bMerge ? "Mrg":"");	}

		//  road stats  --------------------------------
		if (rdTxtSt[0]){  Fmt(s, "Length  %4.0f", road->st.Length);		rdTxtSt[0]->setCaption(s);  }
		if (rdTxtSt[1]){  Fmt(s, "Width   %5.2f", road->st.WidthAvg);	rdTxtSt[1]->setCaption(s);  }
		if (rdTxtSt[2]){  Fmt(s, "Height   %5.2f", road->st.HeightDiff);rdTxtSt[2]->setCaption(s);  }

		if (rdTxtSt[3]){  Fmt(s, "In air   %4.1f%%", road->st.OnTer);	rdTxtSt[3]->setCaption(s);  }
		if (rdTxtSt[4]){  Fmt(s, "Pipes   %4.1f%%", road->st.Pipes);	rdTxtSt[4]->setCaption(s);  }

		int lp = !bCur ? -1 : road->vSegs[road->iChosen].lpos.size();
		if (rdTxtSt[5]){  Fmt(s, "lod pnt:  %d", lp);					rdTxtSt[5]->setCaption(s);  }
		if (rdTxtSt[6]){  Fmt(s, "segs Mrg:  %2d", road->segsMrg+1);	rdTxtSt[6]->setCaption(s);  }
		if (rdTxtSt[7]){  Fmt(s, "vis:  %2d", road->iVis);				rdTxtSt[7]->setCaption(s);  }
		if (rdTxtSt[8]){  Fmt(s, "tri:  %4.1fk", road->iTris/1000.f);	rdTxtSt[8]->setCaption(s);  }

		//if (rdTxtSt[5]){  Fmt(s, "Pitch  %5.2f", road->st.Pitch);  rdTxtSt[5]->setCaption(s);  }
		//if (rdTxtSt[6]){  Fmt(s, "Yaw   %5.2f", road->st.Yaw);    rdTxtSt[6]->setCaption(s);  }
		//if (rdTxtSt[7]){  Fmt(s, "Roll  %5.2f", road->st.Roll);   rdTxtSt[7]->setCaption(s);  }

	///  Modify  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  
		//  road point
		//----------------------------------------------------------------
		Vector3 vx = mCamera->getRight();	   vx.y = 0;  vx.normalise();  // on xz
		Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
		Vector3 vy = Vector3::UNIT_Y;
		if (isKey(LEFT)||isKey(NUMPAD4))  road->Move(-vx*q);	if (isKey(RIGHT)||isKey(NUMPAD6))	road->Move( vx*q);
		if (isKey(UP)||isKey(NUMPAD8))	  road->Move( vz*q);	if (isKey(DOWN)||isKey(NUMPAD2))	road->Move(-vz*q);
		if (isKey(SUBTRACT))	road->Move(-vy*q);			if (isKey(ADD))		road->Move( vy*q);
		if (isKey(MULTIPLY))	road->AddWidth( q*0.4f);	if (isKey(DIVIDE))	road->AddWidth(-q*0.4f);
		if (iSnap == 0)  {
			if (isKey(1))		road->AddYaw(-q*3,0);		if (isKey(3))		road->AddRoll(-q*3,0);
			if (isKey(2))		road->AddYaw( q*3,0);		if (isKey(4))		road->AddRoll( q*3,0);  }
		if (isKey(LBRACKET))	road->AddPipe(-q*0.2);	if (isKey(K ))	road->AddChkR(-q*0.2);  // chk
		if (isKey(RBRACKET))	road->AddPipe( q*0.2);	if (isKey(L))	road->AddChkR( q*0.2);
		if (mz > 0)			road->NextPoint();
		else if (mz < 0)	road->PrevPoint();
	}
	else if (edMode == ED_Start && road)
	{
		//  start, box, dir
		//----------------------------------------------------------------
		Vector3 p;  if (ndCar)  p = ndCar->getPosition();
		Fmt(s, "%4.1f %4.1f %4.1f", p.x,p.y,p.z);	if (brTxt[0])	brTxt[0]->setCaption(/*s*/"");
		//Fmt(s, "start box");						if (brTxt[1])	brTxt[1]->setCaption(s);
		Fmt(s, "width %4.1f", road->vStBoxDim.z);	if (brTxt[1])	brTxt[1]->setCaption(s);
		Fmt(s, "height %4.1f",road->vStBoxDim.y);	if (brTxt[2])	brTxt[2]->setCaption(s);
		Fmt(s, "road dir %+d", road->iDir);			if (brTxt[3])	brTxt[3]->setCaption(s);

		if (isKey(LBRACKET))	{  road->AddBoxH(-q*0.2);  UpdStartPos();  }
		if (isKey(SEMICOLON ))	{  road->AddBoxW(-q*0.2);  UpdStartPos();  }
		if (isKey(RBRACKET))	{  road->AddBoxH( q*0.2);  UpdStartPos();  }
		if (isKey(APOSTROPHE))	{  road->AddBoxW( q*0.2);  UpdStartPos();  }
		//if (mz > 0)	// snap rot by 15 deg ..
	}
	else if (edMode < ED_Road)
	{
		//  brush params
		//----------------------------------------------------------------
		if (brTxt[0]){	Fmt(s, "Size:	 %4.1f   - =", mBrSize[curBr]);		brTxt[0]->setCaption(s);	}
		if (brTxt[1]){	Fmt(s, "Force:   %4.1f   [ ]", mBrIntens[curBr]);	brTxt[1]->setCaption(s);	}
		if (brTxt[2]){	Fmt(s, "Power:   %4.2f   ; \'", mBrPow[curBr]);		brTxt[2]->setCaption(s);	}

		if (brTxt[3])
		if (edMode == ED_Deform || edMode == ED_Height)
		{	Fmt(s, "Shape: %s", csBrShape[mBrShape[curBr]].c_str());	brTxt[3]->setCaption(s);	}
		else
			brTxt[3]->setCaption("");

		if (brTxt[4])
		if (edMode == ED_Height)
		{	Fmt(s, "Height: %4.1f", terSetH);	brTxt[4]->setCaption(s);	}
		else
			brTxt[4]->setCaption("");

		if (mz != 0)
			if (alt){			mBrPow[curBr]   *= 1.f - 0.4f*q*mz;  updBrush();  }
			else if (!shift){	mBrSize[curBr]  *= 1.f - 0.4f*q*mz;  updBrush();  }
			else				mBrIntens[curBr]*= 1.f - 0.4f*q*mz/0.05;
		if (isKey(MINUS)){		mBrSize[curBr]  *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(EQUALS)){		mBrSize[curBr]  *= 1.f + 0.04f*q;  updBrush();  }
		if (isKey(LBRACKET))	mBrIntens[curBr]*= 1.f - 0.04f*q;
		if (isKey(RBRACKET))	mBrIntens[curBr]*= 1.f + 0.04f*q;
		if (isKey(SEMICOLON )){  mBrPow[curBr]  *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(APOSTROPHE)){  mBrPow[curBr]  *= 1.f + 0.04f*q;  updBrush();  }
		//KC_COMMA KC_PERIOD
	}
	mz = 0;  // mouse wheel
	
	//  rebuild after end of selection change
	static bool bSelChngOld = false;
	if (road)
	{
		road->fLodBias = pSet->road_dist;  // after rebuild

		if (bSelChngOld && !road->bSelChng)
			road->RebuildRoad(true);

		bSelChngOld = road->bSelChng;
		road->bSelChng = false;
	}

	///  upd road lods
	static int dti = 5, ti = dti-1;  ++ti;
	if (road && ti >= dti)
	{	ti = 0;
		road->UpdLodVis(pSet->road_dist, edMode == ED_PrvCam);
	}/**/
	return true;
}

//---------------------------------------------------------------------------------------------------------------
///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::processMouse()  //! from Thread, cam vars only
{
	double m_interval = timer.iv;
	static double m_sumTime = 0.0;
	m_sumTime += mDTime;  int num = 0;
	//while (m_sumTime > m_interval)
	{
		num++;
		m_sumTime -= m_interval;
		Real fDT = m_interval;  //mDTime;
		
		//  static vars are smoothed
		Vector3 vInpC(0,0,0),vInp;  //static Vector3 vNew(0,0,0);
		Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * fDT;
		
		const Real sens = 0.13;
		if (bCam())
			vInpC = Vector3(mx, my, 0)*sens;
		vInp = Vector3(mx, my, 0)*sens;  mx = 0;  my = 0;
		vNew += (vInp-vNew) * fSmooth;
		//vNew = vInp;
		
		if (mbMiddle){	mTrans.z += vInpC.y * 1.6f;  }  //zoom
		if (mbRight){	mTrans.x += vInpC.x;  mTrans.y -= vInpC.y;  }  //pan
		if (mbLeft){	mRotX -= vInpC.x;  mRotY -= vInpC.y;  }  //rot
		//mTrans.z -= vInp.z;	//scroll

		//  move camera	//if (bCam())
		{
			Real cs = pSet->cam_speed;  Degree cr(pSet->cam_speed);
			Real fMove = 100*cs;  //par speed
			Degree fRot = 500*cr, fkRot = 80*cr;
		
			static Radian sYaw(0), sPth(0);
			static Vector3 sMove(0,0,0);

			Radian inYaw = rotMul * fDT * (fRot* mRotX + fkRot* mRotKX);
			Radian inPth = rotMul * fDT * (fRot* mRotY + fkRot* mRotKY);
			Vector3 inMove = moveMul * fDT * (fMove * mTrans);

			sYaw += (inYaw - sYaw) * fSmooth;
			//sYaw = inYaw;
			sPth += (inPth - sPth) * fSmooth;
			//sPth = inPth;
			sMove += (inMove - sMove) * fSmooth;
			//sMove = inMove;

			//if (abs(sYaw.valueRadians()) > 0.000001f)
				mCameraT->yaw( sYaw );
			//if (abs(sYaw.valueRadians()) > 0.000001f)
				mCameraT->pitch( sPth );
			//if (sMove.squaredLength() > 0.000001f)
				mCameraT->moveRelative( sMove );
		}
	}
	//LogO("dt: " + toStr((float)mDTime) + "  n.iv's: " + toStr(num));
}

void App::editMouse()
{
	///  mouse edit Road  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (road && bEdit() && edMode == ED_Road)
	{
		const Real fMove(5.0f), fRot(40.f);  //par speed

		if (!alt)
		{
			if (mbLeft)    // move on xz
			{	Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
				Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
				road->Move((vNew.x * vx - vNew.y * vz) * fMove * moveMul);
			}else
			if (mbRight)   // height
				road->Move(-vNew.y * Vector3::UNIT_Y * fMove * moveMul);
			else
			if (mbMiddle)  // width
				road->AddWidth(vNew.x * fMove * moveMul);
		}else
		{	//  alt
			if (mbLeft)    // rot pitch
				road->AddYaw(   vNew.x * fRot * moveMul,0.f);
			if (mbRight)   // rot yaw
				road->AddRoll(  vNew.y *-fRot * moveMul,0.f);
		}
	}

	///  edit ter height val
	if (bEdit() && edMode == ED_Height)
	{
		if (mbRight)
		{	Real ym = -vNew.y * 0.5f * moveMul;
			terSetH += ym;
		}
	}

	///  edit start pos	 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (bEdit() && edMode == ED_Start /*&&
		vStartPos.size() >= 4 && vStartRot.size() >= 4*/)
	{
		const Real fMove(0.5f), fRot(0.05f);  //par speed
		const int n = 0;  // 1st entry - all same / edit 4..
		if (!alt)
		{
			if (mbLeft)	// move on xz
			{
				Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
				Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
				Vector3 vm = (-vNew.y * vx - vNew.x * vz) * fMove * moveMul;
				vStartPos[n][0] += vm.z;
				vStartPos[n][1] += vm.x;  UpdStartPos();
			}else
			if (mbRight)
			{
				Real ym = -vNew.y * fMove * moveMul;
				vStartPos[n][2] += ym;  UpdStartPos();
			}
		}else
		{	//  alt
			typedef QUATERNION<float> Qf;
			if (mbLeft)    // rot pitch
			{
				Qf qr;  qr.Rotate(vNew.x * fRot * moveMul, 0,0,1);
				Qf& q = vStartRot[n];  // get yaw angle, add ..
				q = q * qr;  UpdStartPos();
			}else
			if (mbRight)   // rot yaw
			{
				Qf qr;  qr.Rotate(vNew.y *-fRot * moveMul, 0,1,0);
				Qf& q = vStartRot[n];
				q = q * qr;  UpdStartPos();
			}else
			if (mbMiddle)  // rot reset
			{
				Qf qr;  qr.Rotate(0, 0,0,1);
				Qf& q = vStartRot[n];	q = qr;  UpdStartPos();
			}
		}
	}
}


bool App::frameEnded(const FrameEvent& evt)
{
	//  track events
	if (eTrkEvent != TE_None)
	{	switch (eTrkEvent)  {
			case TE_Load:	LoadTrackEv();  break;
			case TE_Save:	SaveTrackEv();  break;
			case TE_Update: UpdateTrackEv();  break;  }
		eTrkEvent = TE_None;
	}
	
	///  input event queues  ------------------------------------
	for (int i=0; i < i_cmdKeyRel; ++i)
	{	const CmdKey& k = cmdKeyRel[i];
		mGUI->injectKeyRelease(MyGUI::KeyCode::Enum(k.key));  }
	i_cmdKeyRel = 0;

	for (int i=0; i < i_cmdKeyPress; ++i)
	{	const CmdKey& k = cmdKeyPress[i];
		KeyPress(k);  }
	i_cmdKeyPress = 0;

	for (int i=0; i < i_cmdMouseMove; ++i)
	{	const CmdMouseMove& c = cmdMouseMove[i];
		mGUI->injectMouseMove(c.ms.X.abs, c.ms.Y.abs, c.ms.Z.abs);  }
	i_cmdMouseMove = 0;

	for (int i=0; i < i_cmdMousePress; ++i)
	{	const CmdMouseBtn& b = cmdMousePress[i];
		mGUI->injectMousePress(b.ms.X.abs, b.ms.Y.abs, MyGUI::MouseButton::Enum(b.btn));
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		SetCursor(0);
		ShowCursor(0); 
// todo linux
#endif
 }  //?- cursor after alt-tab
	i_cmdMousePress = 0;

	for (int i=0; i < i_cmdMouseRel; ++i)
	{	const CmdMouseBtn& b = cmdMouseRel[i];
		mGUI->injectMouseRelease(b.ms.X.abs, b.ms.Y.abs, MyGUI::MouseButton::Enum(b.btn));  }
	i_cmdMouseRel = 0;
	

	//  road pick
	if (road)
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		Real mx = mp.left, my = mp.top;
		road->Pick(mCamera, mx/mWindow->getWidth(), my/mWindow->getHeight(),
			edMode == ED_Road,  !(edMode == ED_Road && bEdit()));
	}

	editMouse();  // edit


	///  paged  Upd  * * *
	if (bTrGrUpd)
	{	bTrGrUpd = false;
		pSet->bTrees = !pSet->bTrees;
		UpdTrees();
	}
	///  paged  * * *  ? frameStarted
	if (road)
	{	if (grass)  grass->update();
		if (trees)  trees->update();
	}
	
	///<>  Edit Ter
	TerCircleUpd();
	if (terrain && road && bEdit() && road->bHitTer)
	{
		float dt = evt.timeSinceLastFrame;
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft)   deform(road->posHit, dt, s);  else
			if (mbRight)  deform(road->posHit, dt,-s);
			break;
		case ED_Smooth:
			if (mbLeft)   smooth(road->posHit, dt);
			break;
		case ED_Height:
			if (mbLeft)   height(road->posHit, dt, s);
			break;
		}
	}

	///<>  Ter upd
	static int ti = 0;
	if (ti >= pSet->ter_skip)
	if (bTerUpd)
	{	bTerUpd = false;  ti = 0;
		mTerrainGroup->update();
	}	ti++;

	
	if (road)  // road
		road->RebuildRoadInt();
		

	///**  Render Targets update
	if (edMode == ED_PrvCam)
	{
		sc.camPos = mCameraT->getPosition();
		sc.camDir = mCameraT->getDirection();
		if (rt[RTs-1].rndTex)
			rt[RTs-1].rndTex->update();
	}else{
		static int ri = 0;
		if (ri >= pSet->mini_skip)
		{	ri = 0;
			for (int i=0; i < RTs-1/**/; ++i)
				if (rt[i].rndTex)
					rt[i].rndTex->update();
		}	ri++;
	}
	//LogO(toStr(evt.timeSinceLastFrame));

	return true;
}


bool App::frameStarted(const Ogre::FrameEvent& evt)
{
	BaseApp::frameStarted(evt);
	
	if (bGuiReinit)  // after language change from combo
	{	bGuiReinit = false;
		mGUI->destroyWidgets(vwGui);
		InitGui();
		SetGuiFromXmls();
		//bWindowResized = true;
		mWndTabs->setIndexSelected(8);  // switch back to view tab
	//}

	//if (bWindowResized)
	//{	bWindowResized = false;

		//  reposition Quit btn
		if (bnQuit)
			bnQuit->setCoord(pSet->windowx - 0.09*pSet->windowx, 0, 0.09*pSet->windowx, 0.03*pSet->windowy);
		//bSizeHUD = true;
		
		LoadTrack();  // shouldnt be needed but ...
	}
	return true;
}
