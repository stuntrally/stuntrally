#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
using namespace Ogre;


//  perf log car vel
void App::PerfLogVel(CAR* pCar, float time)
{
	pGame->info_output << fToStr(time,2,5) << "s, " << fToStr(pCar->GetSpeed()*3.6f, 1,5) << " kmh, gear " << pCar->GetGear() << ", rpm " << fToStr(pCar->GetEngineRPM(),0,4) \
		//<< ", clu " << fToStr(pCar->GetClutch(), 1,4)
		//<< ", sli " << fToStr(pCar->dynamics.tire[0].slide, 1,4)
		//<< ", slp " << fToStr(pCar->dynamics.tire[1].slip, 1,4)
		//<< ", f " << pCar->GetWheelContact(WHEEL_POSITION(0)).surface->frictionTread
		// !... downforce, drag
		<< std::endl;
}


///  newPoses for Perf Test
//---------------------------------------------------------------------------------------------------------------
void App::newPerfTest(float time)
{
	//  vars
	static float ti = 0.f;
	static int vi = 0;
	//  const
	const int logvel = 10, MaxVel = 210;  // 120 220 kmh max to test
	const float distQuarterMile = 402.336;  //m
	//  results
	static float timeQM=0.f,velAtQM=0.f;  // quarter mile
	static float t0to60=0,t0to100=0,t0to160=0,t0to200=0;  // accel
	static float tMaxTo0=0,tMaxTo60=0,tMaxTo100=0,tMaxTo160=0;  // brake
	static float drag60=0,down60=0,drag100=0,down100=0,drag160=0,down160=0,drag200=0,down200=0;

	CarModel* carM = carModels[0];
	CAR* pCar = carM->pCar;
	bool bNorm = pSet->game.sim_mode == "normal" && pCar->pCarM->sDirname != "M3" && pCar->pCarM->sDirname != "LK4";
	static MATHVECTOR<Dbl,3> posSt, dist;
	float kmh = pCar->GetSpeed()*3.6f;  static float kmhOld = 0.f;

	switch (iPerfTestStage)
	{
		case PT_StartWait:
		{	int whStill = 0;
			for (int i=0; i<4; ++i)
			{
				WHEEL_POSITION wp = (WHEEL_POSITION)i;
				bool inAir = pCar->GetWheelContact(wp).GetColObj() == NULL;
				const CARSUSPENSION& susp = pCar->dynamics.GetSuspension(wp);
				if (!inAir && susp.GetVelocity() < 0.001)  ++whStill;
			}
			if (whStill == 4)
			{
				iPerfTestStage = PT_Accel;  ti = 0.f;  vi = 0;
				posSt = pCar->GetPosition();  timeQM = 0.f;
			}
			kmhOld = 0.f;
			timeQM = 0.f;  velAtQM = 0.f;
			t0to60=0; t0to100=0; t0to160=0; t0to200=0;
			tMaxTo0=0; tMaxTo60=0; tMaxTo100=0; tMaxTo160=0;
			drag60=0; down60=0; drag100=0; down100=0; drag160=0; down160=0; drag200=0; down200=0;
		}	break;
	
		case PT_Accel:
		{
			dist = pCar->GetPosition() - posSt;
			if (timeQM == 0.f && dist.Magnitude() > distQuarterMile)
			{	timeQM = ti;  // will be 0 if didnt drive that far
				velAtQM = kmh;
			}
			//LogO("dist: "+fToStr(dist.Magnitude(),2,5));

			int ikmh = 	vi * logvel;
			if (kmh >= ikmh)
			{	PerfLogVel(pCar,ti);
				MATHVECTOR<Dbl,3> aero = pCar->dynamics.GetTotalAero();
				Dbl drag = -aero[0], down = -aero[2];
				if (ikmh == 60)  {  t0to60  = ti;  drag60  = drag;  down60  = down;  } else
				if (ikmh == 100) {  t0to100 = ti;  drag100 = drag;  down100 = down;  } else
				if (ikmh == 160) {  t0to160 = ti;  drag160 = drag;  down160 = down;  } else
				if (ikmh == 200) {  t0to200 = ti;  drag200 = drag;  down200 = down;  }
				++vi;
			}
			//**/LogO(fToStr((kmh - kmhOld)/*/time*/, 6,9));
			if (timeQM > 0.f &&
				(kmh >= MaxVel || (!bNorm && kmh - kmhOld < 0.0005)/**/))  //top speed..
			{
				//PerfLogVel(pCar,t);
				iPerfTestStage = PT_Brake;  ti = 0.f;  vi = 0;
			}
		}	break;
	
		case PT_Brake:
		{
			int ikmh = 	MaxVel - vi * logvel;
			if (kmh <= ikmh)
			{	PerfLogVel(pCar,ti);
				if (ikmh == 160)  tMaxTo160 = ti;  else
				if (ikmh == 100)  tMaxTo100 = ti;  else
				if (ikmh == 60)   tMaxTo60  = ti;
				++vi;
			}
			if (kmh <= 1.f)
			{	PerfLogVel(pCar,ti);
				bPerfTest = false;  tMaxTo0 = ti;  ti = 0.f;
				
				//  engine stats
				//------------------------
				//pGame->info_output << std::string("====  CAR engine  ====\n");
				const CARENGINE& eng = pCar->dynamics.engine;
				float maxTrq = 0.f, maxPwr = 0.f;
				int rpmMaxTq = 0, rpmMaxPwr = 0;

				for (int r = eng.GetStartRPM(); r < eng.GetRpmMax(); r += 10)
				{	float tq = eng.GetTorqueCurve(1.0, r);
					float pwr = tq * 2.0 * PI_d * r / 60.0 * 0.001;  //kW  // 1kW = 1.341 bhp
					if (tq > maxTrq)  {  maxTrq = tq;  rpmMaxTq = r;  }
					if (pwr > maxPwr)  {  maxPwr = pwr;  rpmMaxPwr = r;  }
					//if (r % 100 == 0)
					//	pGame->info_output << "rpm: "+fToStr(r,0,4)+" Nm:"+fToStr(tq,0,4)+" bhp:"+fToStr(pwr*1.341,0,4)+"\n";
				}

				//  summary
				//------------------------------------------------
				maxPwr *= 1.341;  // kW to bhp
				Dbl m = eng.real_pow_tq_mul;  // factor to match real cars data
				const MATHVECTOR<Dbl,3>& com = pCar->dynamics.center_of_mass;
				std::string sResult = 
					"Car:  "+pCar->pCarM->sDirname+"\n"+
					"Center of mass [m] L,W,H:  "+fToStr(com[0],3,5)+", "+fToStr(com[1],3,5)+", "+fToStr(com[2],3,5)+"\n"+
					"Mass [kg]:  "+fToStr(pCar->GetMass(),0,4)+"\n"+
					"---\n"+
					"Max torque [Nm]:  " +fToStr(maxTrq*m,1,5)+" ("+fToStr(maxTrq,1,5)+") at "+fToStr(rpmMaxTq ,0,4)+" rpm\n"+
					"Max power  [bhp]:  "+fToStr(maxPwr*m,1,5)+" ("+fToStr(maxPwr,1,5)+") at "+fToStr(rpmMaxPwr,0,4)+" rpm\n"+
					"Ratio [bhp/tonne]:  "+fToStr(maxPwr / (pCar->GetMass() * 0.001) ,1,5)+"\n"+
					"------\n"+
					"Time [s] 0.. 60 kmh:  "+fToStr(t0to60 ,2,5)+"  down "+fToStr(down60 ,0,4)+"  drag "+fToStr(drag60 ,0,4)+"\n"+
					"Time [s] 0..100 kmh:  "+fToStr(t0to100,2,5)+"  down "+fToStr(down100,0,4)+"  drag "+fToStr(drag100,0,4)+"\n"+
					"Time [s] 0..160 kmh:  "+fToStr(t0to160,2,5)+"  down "+fToStr(down160,0,4)+"  drag "+fToStr(drag160,0,4)+"\n"+
					(!bNorm ? "" :
					"Time [s] 0..200 kmh:  "+fToStr(t0to200,2,5)+"  down "+fToStr(down200,0,4)+"  drag "+fToStr(drag200,0,4)+"\n")+
					"---\n"+
					"1/4 mile (402m) time:  "+fToStr(timeQM,2,5)+" at "+fToStr(velAtQM,2,5)+" kmh\n"+
					"Stop time 160..0 kmh:  "+fToStr(tMaxTo0-tMaxTo160,2,5)+"\n"+
					"Stop time 100..0 kmh:  "+fToStr(tMaxTo0-tMaxTo100,2,5)+"\n"+
					"Stop time  60..0 kmh:  "+fToStr(tMaxTo0-tMaxTo60,2,5)+"\n";
					//todo: Maximum speed: 237.6 kmh at 16.9 s
				
				pGame->info_output << std::string("====  CAR Perf test summary  ====\n") + sResult + "====\n";
				edPerfTest->setCaption(sResult);
				//if (!mWndTweak->getVisible())  // show
				//	TweakToggle();
				mWndTweak->setVisible(true);
				tabTweak->setIndexSelected(2);
				
				//  save car stats.txt  ---------
				{
					std::string path, pathUser, pathUserDir;
					bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);
					path = pathUserDir + pCar->pCarM->sDirname + "_stats.txt";
					
					PATHMANAGER::CreateDir(pathUserDir, pGame->error_output);
					std::ofstream fo(path.c_str());
					//fo << sResult;
					fo << "Mass\n" << fToStr(pCar->GetMass(),0,4) << " kg\n";
					fo << "Max Torque\n" << fToStr(maxTrq*m,0,3) << " Nm at " << fToStr(rpmMaxTq ,0,4) << " rpm\n";
					fo << "Max Power\n"  << fToStr(maxPwr*m,0,3) << " bhp at " << fToStr(rpmMaxPwr,0,4) << " rpm\n";
					fo << "Time 0 to 100 kmh\n" << fToStr(t0to100,1,4) << " s\n";
					fo << "Time 0 to 160 kmh\n" << fToStr(t0to160,1,4) << " s\n";
					if (bNorm)
					fo << "Time 0 to 200 kmh\n" << fToStr(t0to200,1,4) << " s\n";
					fo << "Stop time 100 to 0 kmh\n" << fToStr(tMaxTo0-tMaxTo100,1,4) << " s\n";
				}
			}
		}	break;
	}
	ti += time;
	kmhOld = kmh;
}
