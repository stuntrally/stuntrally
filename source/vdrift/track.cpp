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


TRACK::TRACK() 
	:pGame(0),
	texture_size("large"),
	loaded(false), asphalt(false),
	sDefaultTire("gravel")
{
}

TRACK::~TRACK()
{
	Clear();
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
	LogO("-=- Loading track from path: "+trackpath);

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
		loaded = true;

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

bool TRACK::BeginObjectLoad(
	const string & trackpath,
	int anisotropy,
	bool dynamicshadowsenabled,
	bool doagressivecombining)
{
	objload.reset(new OBJECTLOADER(trackpath, anisotropy, dynamicshadowsenabled,
		true, doagressivecombining));

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
		loadstatus = ContinueObjectLoad();

	return !loadstatus.first;
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

	for (int i = 0; i < numroads && trackfile; ++i)
	{
		roads.push_back(ROADSTRIP());
		roads.back().ReadFrom(trackfile, cerr);
	}

	if (reverse)
		for_each(roads.begin(), roads.end(), mem_fun_ref(&ROADSTRIP::Reverse));

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
				++rit;
				if (rit == patches.rend())
					rit = patches.rbegin();
				it = rit.base();
				if (it == patches.end())
					it = patches.begin();

				++curoffset;
			}
			else if (curoffset > 0)
			{
				++it;
				if (it == patches.end())
					it = patches.begin();

				--curoffset;
			}
		}

		assert(it != patches.end());
		return optional <const BEZIER *>(&it->GetPatch());
	}
}
