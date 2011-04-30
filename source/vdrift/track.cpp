#include "stdafx.h"

#include "track.h"

#include "configfile.h"
#include "reseatable_reference.h"
#include "tracksurface.h"
#include "objectloader.h"

#include <functional>

#include <algorithm>

#include <list>
using std::list;

#include <map>
using std::pair;

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;

#include <sstream>
using std::stringstream;

TRACK::TRACK(std::ostream & info, std::ostream & error) 
: info_output(info),
  error_output(error),
  texture_size("large"),
  vertical_tracking_skyboxes(false),
  usesurfaces(false),
  //racingline_node(NULL),
  loaded(false),
  cull(false)
{
	surface.type = TRACKSURFACE::ASPHALT;
	surface.bumpWaveLength = 1;
	surface.bumpAmplitude = 0;
	surface.frictionNonTread = 1;
	surface.frictionTread = 1;
	surface.rollResistanceCoefficient = 1;
	surface.rollingDrag = 0;
}

TRACK::~TRACK()
{
	Clear();
}

bool TRACK::Load(
	const std::string & trackpath,
	const std::string & effects_texturepath,
	//SCENENODE & rootnode,
	bool reverse,
	int anisotropy,
	const std::string & texsize)
{
	Clear();

	info_output << "Loading track from path: " << trackpath << endl;

	//load parameters
	if (!LoadParameters(trackpath))
		return false;
	
	if (!LoadSurfaces(trackpath))
		info_output << "No surfaces file. Continuing with standard surfaces" << endl;

	//load roads
	if (!LoadRoads(trackpath, reverse))
	{
		error_output << "Error during road loading; continuing with an unsmoothed track" << std::endl;
		ClearRoads();
	}

	//load the lap sequence
	if (!LoadLapSequence(trackpath, reverse))
		return false;

	if (!CreateRacingLines(/*&rootnode,* effects_texturepath, texsize*/))
		return false;

	//load objects
	if (!LoadObjects(trackpath, /*rootnode,*/ anisotropy))
	{
		info_output << "Track with no objects." << endl;
		//return false;
	}

	info_output << "Track loading was successful: " << model_library.size() << " models, " << texture_library.size() << " textures" << endl;

	return true;
}

bool TRACK::LoadLapSequence(const std::string & trackpath, bool reverse)
{
	string parampath = trackpath + "/track.txt";
	CONFIGFILE trackconfig;
	if (!trackconfig.Load(parampath))
	{
		error_output << "Can't find track configfile: " << parampath << endl;
		return false;
	}
	
	trackconfig.GetParam("cull faces", cull);

	int lapmarkers = 0;
	if (trackconfig.GetParam("lap sequences", lapmarkers))
	{
		for (int l = 0; l < lapmarkers; l++)
		{
			float lapraw[3];
			std::stringstream lapname;
			lapname << "lap sequence " << l;
			trackconfig.GetParam(lapname.str(), lapraw);
			int roadid = lapraw[0];
			int patchid = lapraw[1];

			//info_output << "Looking for lap sequence: " << roadid << ", " << patchid << endl;

			int curroad = 0;
			for (std::list <ROADSTRIP>::iterator i = roads.begin(); i != roads.end(); ++i)
			{
				if (curroad == roadid)
				{
					int curpatch = 0;
					for (std::list <ROADPATCH>::const_iterator p = i->GetPatchList().begin(); p != i->GetPatchList().end(); ++p)
					{
						if (curpatch == patchid)
						{
							lapsequence.push_back(&p->GetPatch());
							//info_output << "Lap sequence found: " << roadid << ", " << patchid << "= " << &p->GetPatch() << endl;
						}
						curpatch++;
					}
				}
				curroad++;
			}
		}
	}

	// calculate distance from starting line for each patch to account for those tracks
	// where starting line is not on the 1st patch of the road
	// note this only updates the road with lap sequence 0 on it
	if (!lapsequence.empty())
	{
		BEZIER* start_patch = const_cast <BEZIER *> (lapsequence[0]);
		start_patch->dist_from_start = 0.0;
		BEZIER* curr_patch = start_patch->next_patch;
		float total_dist = start_patch->length;
		int count = 0;
		while ( curr_patch && curr_patch != start_patch)
		{
			count++;
			curr_patch->dist_from_start = total_dist;
			total_dist += curr_patch->length;
			curr_patch = curr_patch->next_patch;
		}
	}

	if (lapmarkers == 0)
		info_output << "No lap sequence found; lap timing will not be possible" << std::endl;
	else
		info_output << "Track timing sectors: " << lapmarkers << endl;

	return true;
}

bool TRACK::DeferredLoad(
	const std::string & trackpath,
	bool reverse,
	int anisotropy,
	const std::string & texsize,
	bool dynamicshadowsenabled,
	bool doagressivecombining)
{
	Clear();

	texture_size = texsize;
	info_output << "Loading track from path: " << trackpath << endl;

	//load parameters
	if (!LoadParameters(trackpath))
		return false;

	if (!LoadSurfaces(trackpath))
		info_output << "No Surfaces File. Continuing with standard surfaces" << endl;
	size_t num = tracksurfaces.size();
	
	//load roads
	if (!LoadRoads(trackpath, reverse))
	{
		error_output << "Error during road loading; continuing with an unsmoothed track" << std::endl;
		ClearRoads();
	}

	//load the lap sequence
	if (!LoadLapSequence(trackpath, reverse))
		return false;

	if (!CreateRacingLines(/*&rootnode, effects_texturepath, texsize*/))
		return false;

	//load objects
	if (!BeginObjectLoad(trackpath, /*rootnode,*/ anisotropy, dynamicshadowsenabled, doagressivecombining))
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
	tracksurfaces.clear();
	model_library.clear();
	/**/ogre_meshes.clear();///
	texture_library.clear();
	ClearRoads();
	lapsequence.clear();
	start_positions.clear();
	//racingline_node = NULL;
	loaded = false;
	usesurfaces = false;
}

bool TRACK::CreateRacingLines(
	//SCENENODE * parentnode, 
	/*const std::string & texturepath,
	const std::string & texsize*/)
{
	/*assert(parentnode);
	if (!racingline_node)
	{
		racingline_node = &parentnode->AddNode();
	}*/
	
	/*if (!racingline_texture.Loaded())
	{
		TEXTUREINFO tex; 
		tex.SetName(texturepath + "/racingline.png");
		if (!racingline_texture.Load(tex, error_output, texsize))
			return false;
	}*/
	
	K1999 k1999data;
	int n = 0;
	for (std::list <ROADSTRIP>::iterator i = roads.begin(); i != roads.end(); ++i,++n)
	{
		if (k1999data.LoadData(&(*i)))
		{
			k1999data.CalcRaceLine();
			k1999data.UpdateRoadStrip(&(*i));
		}
		//else error_output << "Couldn't create racing line for roadstrip " << n << std::endl;
		
		//i->CreateRacingLine(racingline_node, racingline_texture, error_output);
	}
	
	return true;
}

bool TRACK::LoadParameters(const std::string & trackpath)
{
	string parampath = trackpath + "/track.txt";
	CONFIGFILE param;
	if (!param.Load(parampath))
	{
		error_output << "Can't find track configfile: " << parampath << endl;
		return false;
	}

	vertical_tracking_skyboxes = false; //default to false
	param.GetParam("vertical tracking skyboxes", vertical_tracking_skyboxes);
	//cout << vertical_tracking_skyboxes << endl;

	int sp_num = 0;
	std::stringstream sp_name;
	sp_name << "start position " << sp_num;
	float f3[3];
	float f1;
	while (param.GetParam(sp_name.str(), f3))
	{
		MATHVECTOR <float, 3> pos(f3[2], f3[0], f3[1]);

		sp_name.str("");
		sp_name << "start orientation-xyz " << sp_num;
		if (!param.GetParam(sp_name.str(), f3))
		{
			error_output << "No matching orientation xyz for start position " << sp_num << endl;
			return false;
		}
		sp_name.str("");
		sp_name << "start orientation-w " << sp_num;
		if (!param.GetParam(sp_name.str(), f1))
		{
			error_output << "No matching orientation w for start position " << sp_num << endl;
			return false;
		}

		QUATERNION <float> orient(f3[2], f3[0], f3[1], f1);
		//QUATERNION <float> orient(f3[0], f3[1], f3[2], f1);

		//due to historical reasons the initial orientation places the car faces the wrong way
		QUATERNION <float> fixer; 
		fixer.Rotate(3.141593, 0, 0, 1);
		orient = fixer * orient;

		start_positions.push_back(std::pair <MATHVECTOR <float, 3>, QUATERNION <float> >
				(pos, orient));

		sp_num++;
		sp_name.str("");
		sp_name << "start position " << sp_num;
	}

	return true;
}

bool TRACK::LoadSurfaces(const std::string & trackpath)
{
	string path = trackpath + "/surfaces.txt";
	CONFIGFILE param;
	if (!param.Load(path))
	{
		info_output << "Can't find surfaces configfile: " << path << endl;
		return false;
	}
	
	usesurfaces = true;
	
	std::list <std::string> sectionlist;
	param.GetSectionList(sectionlist);
		
	TRACKSURFACE tempsurface;
	
	// set the size of track surfaces to hold new elements
	//tracksurfaces.resize(sectionlist.size());
	tracksurfaces.clear();//
	
	for (std::list<std::string>::const_iterator section = sectionlist.begin(); section != sectionlist.end(); ++section)
	{
		tempsurface.name = *section;
		
		int indexnum;
		param.GetParam(*section + ".ID", indexnum);
		//-assert(indexnum >= 0 && indexnum < (int)tracksurfaces.size());
		tempsurface.setType(indexnum);
		
		float temp = 0.0;
		param.GetParam(*section + ".BumpWaveLength", temp, error_output);
		tempsurface.bumpWaveLength = temp;
		
		param.GetParam(*section + ".BumpAmplitude", temp, error_output);
		tempsurface.bumpAmplitude = temp;
		
		param.GetParam(*section + ".FrictionNonTread", temp, error_output);
		tempsurface.frictionNonTread = temp;
		
		param.GetParam(*section + ".FrictionTread", temp, error_output);
		tempsurface.frictionTread = temp;
		
		param.GetParam(*section + ".RollResistanceCoefficient", temp, error_output);
		tempsurface.rollResistanceCoefficient = temp;
		
		param.GetParam(*section + ".RollingDrag", temp, error_output);
		tempsurface.rollingDrag = temp;
		
		tracksurfaces.push_back(tempsurface);//
		info_output << "  new surface" << endl;//
		
		//std::list<TRACKSURFACE>::iterator it = tracksurfaces.begin();
		//while(indexnum-- > 0) it++;
		//*it = tempsurface;
	}
	info_output << "Found and loaded surfaces file" << endl;
	
	return true;
}

bool TRACK::BeginObjectLoad(
	const std::string & trackpath,
	//SCENENODE & sceneroot,
	int anisotropy,
	bool dynamicshadowsenabled,
	bool doagressivecombining)
{
	objload.reset(new OBJECTLOADER(trackpath, /*sceneroot,*/ anisotropy, dynamicshadowsenabled,
		info_output, error_output, cull, doagressivecombining));

	if (!objload->BeginObjectLoad())
		return false;

	return true;
}

std::pair <bool,bool> TRACK::ContinueObjectLoad()
{
	assert(objload.get());
	return objload->ContinueObjectLoad(this, model_library, texture_library,
		objects, tracksurfaces, usesurfaces, vertical_tracking_skyboxes, texture_size);
}

bool TRACK::LoadObjects(const std::string & trackpath, /*SCENENODE & sceneroot,*/ int anisotropy)
{
	BeginObjectLoad(trackpath, /*sceneroot,*/ anisotropy, false, false);
	pair <bool,bool> loadstatus = ContinueObjectLoad();
	while (!loadstatus.first && loadstatus.second)
	{
		loadstatus = ContinueObjectLoad();
	}
	return !loadstatus.first;
}

void TRACK::Reverse()
{
	//move timing sector 0 back 1 patch so we'll still drive over it when going in reverse around the track
	if (!lapsequence.empty())
	{
		int counts = 0;

		for (std::list <ROADSTRIP>::iterator i = roads.begin(); i != roads.end(); ++i)
		{
			optional <const BEZIER *> newstartline = i->FindBezierAtOffset(lapsequence[0],-1);
			if (newstartline)
			{
				lapsequence[0] = newstartline.get();
				counts++;
			}
		}

		assert(counts == 1); //do a sanity check, because I don't trust the FindBezierAtOffset function
	}

	//reverse the timing sectors
	if (lapsequence.size() > 1)
	{
		//reverse the lap sequence, but keep the first bezier where it is (remember, the track is a loop)
		//so, for example, now instead of 1 2 3 4 we should have 1 4 3 2
		std::vector <const BEZIER *>::iterator secondbezier = lapsequence.begin();
		++secondbezier;
		assert(secondbezier != lapsequence.end());
		std::reverse(secondbezier, lapsequence.end());
	}


	//flip start positions
	for (std::vector <std::pair <MATHVECTOR <float, 3>, QUATERNION <float> > >::iterator i = start_positions.begin();
		i != start_positions.end(); ++i)
	{
		i->second.Rotate(3.141593, 0,0,1);
	}

	//reverse start positions
	std::reverse(start_positions.begin(), start_positions.end());

	//reverse roads
	std::for_each(roads.begin(), roads.end(), std::mem_fun_ref(&ROADSTRIP::Reverse));
}

bool TRACK::LoadRoads(const std::string & trackpath, bool reverse)
{
	ClearRoads();

	ifstream trackfile;
	trackfile.open((trackpath + "/roads.trk").c_str());
	if (!trackfile)
	{
		//error_output << "Error opening roads file: " << trackpath + "/roads.trk" << std::endl;
		//return false;
	}

	int numroads;

	trackfile >> numroads;

	for (int i = 0; i < numroads && trackfile; i++)
	{
		roads.push_back(ROADSTRIP());
		roads.back().ReadFrom(trackfile, error_output);
	}

	if (reverse)
	{
		Reverse();
		direction = DIRECTION_REVERSE;
	}
	else
		direction = DIRECTION_FORWARD;

	return true;
}

bool TRACK::CastRay(
	const MATHVECTOR <float, 3> & origin,
	const MATHVECTOR <float, 3> & direction,
	float seglen, MATHVECTOR <float, 3> & outtri,
	const BEZIER * & colpatch,
	MATHVECTOR <float, 3> & normal) const
{
	bool col = false;
	for (std::list <ROADSTRIP>::const_iterator i = roads.begin(); i != roads.end(); ++i)
	{
		MATHVECTOR <float, 3> coltri, colnorm;
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
	std::list <ROADPATCH>::const_iterator it = patches.end(); //this iterator will hold the found ROADPATCH

	//search for the roadpatch containing the bezier and store an iterator to it in "it"
	for (std::list <ROADPATCH>::const_iterator i = patches.begin(); i != patches.end(); ++i)
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
				std::list <ROADPATCH>::const_reverse_iterator rit(it);
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

std::pair <MATHVECTOR <float, 3>, QUATERNION <float> > TRACK::GetStart(unsigned int index)
{
	//assert(!start_positions.empty());
	unsigned int laststart = start_positions.size()-1;
	if (index > laststart)
	{
		std::pair <MATHVECTOR <float, 3>, QUATERNION <float> > sp = start_positions[laststart];
		MATHVECTOR <float, 3> backward(6,0,0);
		backward = backward * (index-laststart);
		sp.second.RotateVector(backward);
		sp.first = sp.first + backward;
		return sp;
	}
	else
		return start_positions[index];
}
