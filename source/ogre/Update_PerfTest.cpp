#include "pch.h"
#include "common/Def_Str.h"
#include "CGame.h"
#include "CGui.h"
#include "CarModel.h"
#include "common/data/SceneXml.h"
#include "common/CScene.h"
#include "../vdrift/game.h"
#include <MyGUI_EditBox.h>
#include <MyGUI_Window.h>
#include <MyGUI_TabControl.h>
using namespace Ogre;


//  perf log car vel
void App::PerfLogVel(CAR* pCar, float time)
{
	LogO(fToStr(time,2,5) +"s, "+ fToStr(pCar->GetSpeed()*3.6f, 1,5) +" kmh, gear "+ toStr(pCar->GetGear()) +", rpm "+ fToStr(pCar->GetEngineRPM(),0,4));
		//<< ", clu " << fToStr(pCar->GetClutch(), 1,4)
		//<< ", sli " << fToStr(pCar->dynamics.tire[0].slide, 1,4)
		//<< ", slp " << fToStr(pCar->dynamics.tire[1].slip, 1,4)
		//<< ", f " << pCar->GetWheelContact(WHEEL_POSITION(0)).surface->friction
		// !... downforce, drag
}


///  newPoses for Perf Test
//---------------------------------------------------------------------------------------------------------------
void App::newPerfTest(float time)
{
	//  vars
	static float ti = 0.f, kmhOld = 0.f, avg_kmh = 0.f;
	const float distQuarterMile = 402.336;  //m
	//  results
	static float
		maxVel=0.f,tiMaxVel=0.f,	// max vel
		timeQM=0.f,velAtQM=0.f,		// quarter mile
		t0to60=0, t0to100=0, t0to160=0, t0to200=0,			// accel
		tMaxTo0=0, tMaxTo60=0, tMaxTo100=0, tMaxTo160=0,	// brake
		drag60=0,down60=0, drag100=0,down100=0, drag160=0,down160=0, drag200=0,down200=0;  // aero drag, downforce

	CarModel* carM = carModels[0];
	CAR* pCar = carM->pCar;
	static MATHVECTOR<Dbl,3> posSt, dist;
	float kmh = pCar->GetSpeed()*3.6f;
	
	static std::vector<float> tkmh,ttim;
	static float kmhP=0.f;
	CARDYNAMICS& cd = pCar->dynamics;

	switch (iPerfTestStage)
	{
		case PT_StartWait:
		{
			//  wheels still count
			int whStill = cd.vtype == V_Spaceship ? 4 : 0;
			for (int i=0; i < cd.numWheels; ++i)
			{
				WHEEL_POSITION wp = (WHEEL_POSITION)i;
				bool inAir = pCar->GetWheelContact(wp).GetColObj() == NULL;
				const CARSUSPENSION& susp = cd.GetSuspension(wp);
				if (!inAir && susp.GetVelocity() < 0.001)  ++whStill;
			}
			if (whStill == cd.numWheels && iLoad1stFrames == -2)  //end
			{
				iPerfTestStage = PT_Accel;  ti = 0.f;
				posSt = pCar->GetPosition();  timeQM = 0.f;
			}
			kmhOld = 0.f;  avg_kmh = 0.f;  // zero results
			timeQM = 0.f;  velAtQM = 0.f;
			maxVel=0.f;  tiMaxVel=0.f;
			t0to60=0; t0to100=0; t0to160=0; t0to200=0;
			tMaxTo0=0; tMaxTo60=0; tMaxTo100=0; tMaxTo160=0;
			drag60=0; down60=0; drag100=0; down100=0; drag160=0; down160=0; drag200=0; down200=0;
			kmhP = 0.f;
			tkmh.clear();  ttim.clear();
		}	break;
	
		case PT_Accel:
		{
			//  quarter mile
			dist = pCar->GetPosition() - posSt;
			if (timeQM == 0.f && dist.Magnitude() > distQuarterMile)
			{	timeQM = ti;  // will be 0 if didnt drive that far
				velAtQM = kmh;
			}
			MATHVECTOR<Dbl,3> aero = cd.GetTotalAero();
			Dbl drag = -aero[0], down = -aero[2];

			//  stats  ---------
			if (kmh >= 60.f   && t0to60 ==0) {  t0to60  = ti;  drag60  = drag;  down60  = down;  PerfLogVel(pCar,ti);  } else
			if (kmh >= 100.f  && t0to100==0) {  t0to100 = ti;  drag100 = drag;  down100 = down;  PerfLogVel(pCar,ti);  } else
			if (kmh >= 160.f  && t0to160==0) {  t0to160 = ti;  drag160 = drag;  down160 = down;  PerfLogVel(pCar,ti);  } else
			if (kmh >= 200.f  && t0to200==0) {  t0to200 = ti;  drag200 = drag;  down200 = down;  PerfLogVel(pCar,ti);  }
						
			avg_kmh += (kmh - avg_kmh) * 0.02f;  // smoothed
			//LogO("t "+ fToStr(ti, 4,7) + " kmh "+ fToStr(kmh,3,6) + " avg_kmh "+ fToStr(avg_kmh,3,6) + "  d "+ fToStr(avg_kmh - kmhOld,3,6));

			//  graph  ---------
			if (kmh - kmhP > 5.f)
			{	kmhP = kmh;
				tkmh.push_back(kmh);  ttim.push_back(ti);
				//LogO("t "+ fToStr(ti, 4,7) +" kmh "+ fToStr(kmh,3,6));
			}

			///  end accel, reached max vel
			if (timeQM > 0.f &&
				avg_kmh - kmhOld < 0.01f)  //par
			{
				iPerfTestStage = PT_Brake;
				maxVel = kmh;  tiMaxVel = ti;  PerfLogVel(pCar,ti);
				tkmh.push_back(kmh);  ttim.push_back(ti);  //
				ti = 0.f;
			}

		}	break;
	
		case PT_Brake:
		{
			//  stats  ---------
			if (kmh <= 160.f  && tMaxTo160==0) {  tMaxTo160 = ti;  PerfLogVel(pCar,ti);  } else
			if (kmh <= 100.f  && tMaxTo100==0) {  tMaxTo100 = ti;  PerfLogVel(pCar,ti);  } else
			if (kmh <= 60.f   && tMaxTo60 ==0) {  tMaxTo60  = ti;  PerfLogVel(pCar,ti);  }

			if (kmh <= 1.f)  //end
			{
				PerfLogVel(pCar,ti);
				bPerfTest = false;  tMaxTo0 = ti;  ti = 0.f;
				
				//  engine stats
				//------------------------
				//LogO("====  CAR engine  ====");
				const CARENGINE& eng = cd.engine;
				float maxTrq = 0.f, maxPwr = 0.f;
				int rpmMaxTq = 0, rpmMaxPwr = 0;

				for (int r = eng.GetStartRPM(); r < eng.GetRpmMax(); r += 10)
				{	float tq = eng.GetTorqueCurve(1.0, r);
					float pwr = tq * 2.0 * PI_d * r / 60.0 * 0.001;  //kW  // 1kW = 1.341 bhp
					if (tq > maxTrq)  {  maxTrq = tq;  rpmMaxTq = r;  }
					if (pwr > maxPwr)  {  maxPwr = pwr;  rpmMaxPwr = r;  }
					//if (r % 100 == 0)
					//	LogO("rpm: "+fToStr(r,0,4)+" Nm:"+fToStr(tq,0,4)+" bhp:"+fToStr(pwr*1.341,0,4));
				}

				//  summary  gui txt
				//------------------------------------------------
				maxPwr *= 1.341;  // kW to bhp
				Dbl m = eng.real_pow_tq_mul;  // factor to match real cars data
				const MATHVECTOR<Dbl,3>& com = cd.center_of_mass;
				Dbl bhpPerTon = maxPwr / (pCar->GetMass() * 0.001);

				//  com ratio
				Dbl whf = cd.wheel[0].GetExtendedPosition()[0], whr = cd.wheel[cd.numWheels==2?1:2].GetExtendedPosition()[0];
				float comFrontPercent = (com[0]+whf) / (whf-whr)*100.f;
				MATRIX3 <Dbl> inertia = cd.body.GetInertiaConst();
				float inert[3];  inert[0] = inertia[0];  inert[1] = inertia[4];  inert[2] = inertia[8];  

				std::string sResult = 
					"Car:  "+pCar->pCarM->sDirname+"\n"+
					"Center of mass [m] L,W,H:  "+fToStr(com[0],3,5)+", "+fToStr(com[1],3,5)+", "+fToStr(com[2],3,5)+"\n"+
					"Mass [kg]:  "+fToStr(pCar->GetMass(),0,4)+"\n"+
					"---\n"+
					"Max torque [Nm]:  " +fToStr(maxTrq*m,1,5)+" ("+fToStr(maxTrq,1,5)+") at "+fToStr(rpmMaxTq ,0,4)+" rpm\n"+
					"Max power  [bhp]:  "+fToStr(maxPwr*m,1,5)+" ("+fToStr(maxPwr,1,5)+") at "+fToStr(rpmMaxPwr,0,4)+" rpm\n"+
					"Ratio [bhp/tonne]:  "+fToStr(bhpPerTon,1,5)+"\n"+
					"Top speed: "+fToStr(maxVel,1,5)+" kmh  at time:  "+fToStr(tiMaxVel,1,4)+" s\n"+
					"------\n"+
					"Time [s] 0.. 60 kmh:  "+fToStr(t0to60 ,2,5)+"  down "+fToStr(down60 ,0,4)+"  drag "+fToStr(drag60 ,0,4)+"\n"+
					"Time [s] 0..100 kmh:  "+fToStr(t0to100,2,5)+"  down "+fToStr(down100,0,4)+"  drag "+fToStr(drag100,0,4)+"\n"+
					(maxVel < 160.f ? "" :
					"Time [s] 0..160 kmh:  "+fToStr(t0to160,2,5)+"  down "+fToStr(down160,0,4)+"  drag "+fToStr(drag160,0,4)+"\n")+
					(maxVel < 200.f ? "" :
					"Time [s] 0..200 kmh:  "+fToStr(t0to200,2,5)+"  down "+fToStr(down200,0,4)+"  drag "+fToStr(drag200,0,4)+"\n")+
					"---\n"+
					"1/4 mile (402m) time:  "+fToStr(timeQM,2,5)+" at "+fToStr(velAtQM,2,5)+" kmh\n"+
					"Stop time 160..0 kmh:  "+fToStr(tMaxTo0-tMaxTo160,2,5)+"\n"+
					"Stop time 100..0 kmh:  "+fToStr(tMaxTo0-tMaxTo100,2,5)+"\n"+
					"Stop time  60..0 kmh:  "+fToStr(tMaxTo0-tMaxTo60,2,5)+"\n";
				
				//LogO("====  CAR Perf test summary  ====\n" + sResult + "====");
				gui->edPerfTest->setCaption(sResult);
				mWndTweak->setVisible(true);
				gui->tabTweak->setIndexSelected(3);
				

				//  save car _stats.xml
				//------------------------------------------------
				{
					std::string path, pathUser, pathUserDir;
					bool user = gui->GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], scn->sc->asphalt);
					path = pathUserDir + pCar->pCarM->sDirname + "_stats.xml";
					
					PATHMANAGER::CreateDir(pathUserDir);

					TiXmlDocument xml;	TiXmlElement root("perf");
					std::string s;

					TiXmlElement car("car");
						car.SetAttribute("mass",	toStrC(pCar->GetMass()) );
						s = fToStr(inert[0],0,3)+" "+fToStr(inert[1],0,3)+" "+fToStr(inert[2],0,3);
						car.SetAttribute("inertia",	s.c_str() );
					root.InsertEndChild(car);

					TiXmlElement co("com");
						co.SetAttribute("frontPercent",	toStrC(comFrontPercent) );
						s = fToStr(com[0],3,5)+" "+fToStr(com[1],3,5)+" "+fToStr(com[2],3,5);
						co.SetAttribute("pos",		s.c_str());
						co.SetAttribute("whf",		toStrC(Real(whf)));
						co.SetAttribute("whr",		toStrC(Real(whr)));
					root.InsertEndChild(co);

					TiXmlElement tq("torque");
						tq.SetAttribute("max",		toStrC(Real(maxTrq*m)) );
						tq.SetAttribute("rpm",		toStrC(rpmMaxTq) );
						tq.SetAttribute("mul",		toStrC(Real(m)) );
					root.InsertEndChild(tq);

					TiXmlElement pw("power");
						pw.SetAttribute("max",		toStrC(Real(maxPwr*m)) );
						pw.SetAttribute("rpm",		toStrC(rpmMaxPwr) );
					root.InsertEndChild(pw);

					TiXmlElement bh("bhpPerTon");
						bh.SetAttribute("val",		toStrC(Real(bhpPerTon)) );
					root.InsertEndChild(bh);

					TiXmlElement tp("top");
						tp.SetAttribute("speed",	toStrC(maxVel) );
						tp.SetAttribute("time",		toStrC(tiMaxVel) );
					root.InsertEndChild(tp);

					TiXmlElement qm("quarterMile");
						qm.SetAttribute("time",		toStrC(timeQM) );
						qm.SetAttribute("vel",		toStrC(velAtQM) );
					root.InsertEndChild(qm);

					TiXmlElement ta("accel"), dn("downForce");
						ta.SetAttribute("t60",		toStrC(t0to60) );
						ta.SetAttribute("t100",		toStrC(t0to100) );	dn.SetAttribute("d100",	toStrC(down100) );
						if (maxVel > 160.f)
						{	ta.SetAttribute("t160",	toStrC(t0to160) );	dn.SetAttribute("d160",	toStrC(down160) );  }
						if (maxVel > 200.f)
						{	ta.SetAttribute("t200",	toStrC(t0to200) );	dn.SetAttribute("d200",	toStrC(down200) );  }
					root.InsertEndChild(ta);
					root.InsertEndChild(dn);

					TiXmlElement st("stop");
						st.SetAttribute("s160",		toStrC(tMaxTo0-tMaxTo160) );
						st.SetAttribute("s100",		toStrC(tMaxTo0-tMaxTo100) );
						st.SetAttribute("s60",		toStrC(tMaxTo0-tMaxTo60) );
					root.InsertEndChild(st);


					/*  speed graph points  */
					TiXmlElement acc("velGraph");
					for (int i=0; i < ttim.size(); ++i)
					{
						TiXmlElement p("p");
						p.SetAttribute("t",		fToStr(ttim[i],2,4).c_str() );
						p.SetAttribute("v",		fToStr(tkmh[i],1,3).c_str() );
						acc.InsertEndChild(p);
					}
					root.InsertEndChild(acc);
					/**/

					xml.InsertEndChild(root);
					xml.SaveFile(path.c_str());
				}
			}
		}	break;
	}
	ti += time;
	kmhOld = avg_kmh;
}
