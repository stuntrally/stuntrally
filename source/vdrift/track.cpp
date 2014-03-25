#include "pch.h"
#include "par.h"
#include "track.h"

#include "configfile.h"
#include "reseatable_reference.h"
#include "tracksurface.h"
#include "objectloader.h"
#include <functional>
#include <algorithm>
#include "../ogre/common/Def_Str.h"
#include "game.h"  // for tires map

#include <list>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;


TRACK::TRACK(ostream & info, ostream & error) 
	:pGame(0),
	info_output(info), error_output(error),
	texture_size("large"),
	loaded(false),
	sDefaultTire("gravel")
{
}

TRACK::~TRACK()
{
	Clear();
}

bool TRACK::Load(
	const string & trackpath,
	const string & effects_texturepath,
	bool reverse,
	int anisotropy,
	const string & texsize)
{
	Clear();

	info_output << "Loading track from path: " << trackpath << endl;

	//load parameters
	if (!LoadParameters(trackpath))
		return false;
	
	//load roads
	if (!LoadRoads(trackpath, reverse))
	{
		//error_output << "Error during road loading; continuing with an unsmoothed track" << endl;
		ClearRoads();
	}

	//if (!CreateRacingLines())
	//	return false;

	//load objects
	if (!LoadObjects(trackpath, anisotropy))
	{
		info_output << "Track with no objects." << endl;
		//return false;
	}

	info_output << "Track loaded: " << model_library.size() << " models, " << texture_library.size() << " textures" << endl;

	return true;
}


bool TRACK::DeferredLoad(
	const string & trackpath,
	bool reverse,
	int anisotropy,
	const string & texsize,
	bool dynamicshadowsenabled,
	bool doagressivecombining)
{
	Clear();

	texture_size = texsize;
	info_output << "Loading track from path: " << trackpath << endl;

	//load parameters
	if (!LoadParameters(trackpath))
		return false;

	//load roads
	if (!LoadRoads(trackpath, reverse))
	{
		//error_output << "Error during road loading; continuing with an unsmoothed track" << endl;
		ClearRoads();
	}

	//if (!CreateRacingLines())
	//	return false;

	//load objects
	if (!BeginObjectLoad(trackpath, anisotropy, dynamicshadowsenabled, doagressivecombining))
		return false;

	return true;
}

bool TRACK::ContinueDeferredLoad()
{
	if (Loaded())
		return true;

	pair <bool,bool> loadstatus = ContinueObjectLoad();
	if (loadstatus.first)
		return false;
	if (!loadstatus.second)
	{
		loaded = true;
	}
	return true;
}

int TRACK::DeferredLoadTotalObjects()
{
	assert(objload.get());
	return objload->GetNumObjects();
}

void TRACK::Clear()
{
	objects.clear();
	model_library.clear();
	texture_library.clear();

	ogre_meshes.clear();///

	ClearRoads();
	
	loaded = false;
}

bool TRACK::CreateRacingLines()
{
	K1999 k1999data;
	int n = 0;
	for (list <ROADSTRIP>::iterator i = roads.begin(); i != roads.end(); ++i,++n)
	{
		if (k1999data.LoadData(&(*i)))
		{
			k1999data.CalcRaceLine();
			k1999data.UpdateRoadStrip(&(*i));
		}
		//else error_output << "Couldn't create racing line for roadstrip " << n << endl;
		
		//i->CreateRacingLine(racingline_node, racingline_texture, error_output);
	}
	return true;
}

bool TRACK::LoadParameters(const string & trackpath)
{
	string parampath = trackpath + "/track.txt";
	CONFIGFILE param;
	if (!param.Load(parampath))
	{
		//error_output << "Can't find track configfile: " << parampath << endl;
		return false;
	}

	float f3[3], f1;
	
	if (!param.GetParam("start position 0", f3))
		return false;

	start_position = MATHVECTOR<float,3>(f3[2], f3[0], f3[1]);

	if (!param.GetParam("start orientation-xyz 0", f3))
		return false;
	if (!param.GetParam("start orientation-w 0", f1))
		return false;

	QUATERNION<float> rot(f3[2], f3[0], f3[1], f1);
	QUATERNION<float> fixer;  fixer.Rotate(PI_d, 0, 0, 1);
	rot = fixer * rot;

	start_rotation = rot;

	return true;
}


bool TRACK::BeginObjectLoad(
	const string & trackpath,
	int anisotropy,
	bool dynamicshadowsenabled,
	bool doagressivecombining)
{
	objload.reset(new OBJECTLOADER(trackpath, anisotropy, dynamicshadowsenabled,
		info_output, error_output, true, doagressivecombining));

	if (!objload->BeginObjectLoad())
		return false;

	return true;
}

pair <bool,bool> TRACK::ContinueObjectLoad()
{
	assert(objload.get());
	return objload->ContinueObjectLoad(this, model_library, texture_library, objects, texture_size);
}

bool TRACK::LoadObjects(const string & trackpath, int anisotropy)
{
	BeginObjectLoad(trackpath, anisotropy, false, false);
	pair <bool,bool> loadstatus = ContinueObjectLoad();
	while (!loadstatus.first && loadstatus.second)
	{
		loadstatus = ContinueObjectLoad();
	}
	return !loadstatus.first;
}

void TRACK::Reverse()
{
/*	//flip start positions
	for (vector <pair <MATHVECTOR<float,3>, QUATERNION<float> > >::iterator i = start_positions.begin();
		i != start_positions.end(); ++i)
	{
		i->second.Rotate(PI_d, 0,0,1);
		i->second[0] = -i->second[0];
		i->second[1] = -i->second[1];
		//i->second[2] = -i->second[2];
		//i->second[3] = -i->second[3];
	}

	//reverse start positions
	reverse(start_positions.begin(), start_positions.end());
*/
	//reverse roads
	for_each(roads.begin(), roads.end(), mem_fun_ref(&ROADSTRIP::Reverse));
}

bool TRACK::LoadRoads(const string & trackpath, bool reverse)
{
	ClearRoads();

	ifstream trackfile;
	trackfile.open((trackpath + "/roads.trk").c_str());
	if (!trackfile)
	{
		//error_output << "Error opening roads file: " << trackpath + "/roads.trk" << endl;
		//return false;
	}

	int numroads=0;

	trackfile >> numroads;

	for (int i = 0; i < numroads && trackfile; i++)
	{
		roads.push_back(ROADSTRIP());
		roads.back().ReadFrom(trackfile, error_output);
	}

	if (reverse)
		Reverse();

	return true;
}

bool TRACK::CastRay(
	const MATHVECTOR<float,3> & origin,
	const MATHVECTOR<float,3> & direction,
	float seglen, MATHVECTOR<float,3> & outtri,
	const BEZIER * & colpatch,
	MATHVECTOR<float,3> & normal) const
{
	bool col = false;
	for (list <ROADSTRIP>::const_iterator i = roads.begin(); i != roads.end(); ++i)
	{
		MATHVECTOR<float,3> coltri, colnorm;
		const BEZIER * colbez = NULL;
		if (i->Collide(origin, direction, seglen, coltri, colbez, colnorm))
		{
			if (!col || (coltri-origin).Magnitude() < (outtri-origin).Magnitude())
			{
				outtri = coltri;
				normal = colnorm;
				colpatch = colbez;
			}

			col = true;
		}
	}

	return col;
}

optional <const BEZIER *> ROADSTRIP::FindBezierAtOffset(const BEZIER * bezier, int offset) const
{
	list <ROADPATCH>::const_iterator it = patches.end(); //this iterator will hold the found ROADPATCH

	//search for the roadpatch containing the bezier and store an iterator to it in "it"
	for (list <ROADPATCH>::const_iterator i = patches.begin(); i != patches.end(); ++i)
	{
		if (&i->GetPatch() == bezier)
		{
			it = i;
			break;
		}
	}

	if (it == patches.end())
		return optional <const BEZIER *>(); //return nothing
	else
	{
		//now do the offset
		int curoffset = offset;
		while (curoffset != 0)
		{
			if (curoffset < 0)
			{
				//why is this so difficult?  all i'm trying to do is make the iterator loop around
				list <ROADPATCH>::const_reverse_iterator rit(it);
				if (rit == patches.rend())
					rit = patches.rbegin();
				rit++;
				if (rit == patches.rend())
					rit = patches.rbegin();
				it = rit.base();
				if (it == patches.end())
					it = patches.begin();

				curoffset++;
			}
			else if (curoffset > 0)
			{
				it++;
				if (it == patches.end())
					it = patches.begin();

				curoffset--;
			}
		}

		assert(it != patches.end());
		return optional <const BEZIER *>(&it->GetPatch());
	}
}

pair <MATHVECTOR<float,3>, QUATERNION<float> > TRACK::GetStart(int index)
{
	pair <MATHVECTOR<float,3>, QUATERNION<float> > sp = make_pair(start_position, start_rotation);
	if (index == 0)
		return sp;

	MATHVECTOR<float,3> backward(-gPar.startNextDist * index,0,0);
	sp.second.RotateVector(backward);
	sp.first = sp.first + backward;
	return sp;
}
