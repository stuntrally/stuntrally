#pragma once
#include <memory>

#include "model_joe03.h"
#include "texture.h"
#include "joepack.h"
#include "tracksurface.h"

#include "mathvector.h"
#include "quaternion.h"
#include "bezier.h"
#include "aabb.h"
#include "aabb_space_partitioning.h"
#include "optional.h"

#include "track_object.h"
#include "roadstrip.h"

class OBJECTLOADER;


///------- for ogre
class OGRE_MESH
{
public:
	bool newMtr, sky, alpha, newM, found;
	std::string name, material;
	class VERTEXARRAY* mesh;

	//bool operator<(const OGRE_MESH& other)
	//{
	//	return material < other.material;
	//}
};
///-------


class TRACK
{
public:
	TRACK();
	~TRACK();
	
	class GAME* pGame;  // for tire map
	bool asphalt;  // for car config switch
	std::string sDefaultTire;  //

	void Clear();

	///---- mesh list for ogre ----
	std::vector<OGRE_MESH> ogre_meshes;
	
	
	///returns true if successful.  only begins loading the track; the track won't be loaded until more calls to ContinueDeferredLoad().
	///  use Loaded() to see if loading is complete yet.
	bool DeferredLoad(
		const std::string & trackpath,
		bool reverse,
		int anisotropy,
		const std::string & texsize,
		bool shadows,
		bool doagressivecombining);
	
	bool ContinueDeferredLoad();
	
	int DeferredLoadTotalObjects();

	
	bool Loaded() const
	{
		return loaded;
	}
	
	bool CastRay(
		const MATHVECTOR<float,3> & origin,
		const MATHVECTOR<float,3> & direction,
		float seglen, MATHVECTOR<float,3> & outtri,
		const BEZIER * & colpatch,
		MATHVECTOR<float,3> & normal) const;
	
	const std::list <ROADSTRIP> & GetRoadList() const
	{
		return roads;
	}
	
	void Unload()
	{
		Clear();
	}
	
	const std::list<TRACK_OBJECT> & GetTrackObjects()
	{
		return objects;
	}

private:

	std::string texture_size;
public:
	std::map <std::string, MODEL_JOE03> model_library;
private:
	std::map <std::string, TEXTURE_GL> texture_library;
	std::list <TRACK_OBJECT> objects;

private:
	
	//road information
	std::list <ROADSTRIP> roads;
	
	bool LoadObjects(
		const std::string & trackpath,
		//SCENENODE & sceneroot,
		int anisotropy);
	
	std::auto_ptr <OBJECTLOADER> objload;
	
	///returns false on error
	bool BeginObjectLoad(
		const std::string & trackpath,
		int anisotropy,
		bool dynamicshadowsenabled,
		bool doagressivecombining);
	
	///returns a pair of bools: the first bool is true if there was an error, the second bool is true if an object was loaded
	std::pair <bool, bool> ContinueObjectLoad();
	
	bool LoadRoads(const std::string & trackpath, bool reverse);
	
	void ClearRoads() {  roads.clear();  }
	
	///read from the file stream and put it in "output".
	/// return true if the get was successful, else false
	template <typename T>
	bool GetParam(std::ifstream & f, T & output)
	{
		if (!f.good())
			return false;
		
		std::string instr;
		f >> instr;
		if (instr.empty())
			return false;
		
		while (!instr.empty() && instr[0] == '#' && f.good())
		{
			f.ignore(1024, '\n');
			f >> instr;
		}
		
		if (!f.good() && !instr.empty() && instr[0] == '#')
			return false;
		
		std::stringstream sstr(instr);
		sstr >> output;
		return true;
	}
	
	bool loaded;
};
