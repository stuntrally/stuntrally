#pragma once
#include "joepack.h"
#include "model_joe03.h"
#include "track_object.h"
#include "tracksurface.h"

class TRACK;

class OBJECTLOADER
{
public:
	OBJECTLOADER(
		const std::string & ntrackpath,
		int nanisotropy,
		bool newdynamicshadowsenabled,
		bool newcull,
		bool doagressivecombining);

	bool GetError() const
	{
		return error;
	}

	int GetNumObjects() const
	{
		return numobjects;
	}
	
	///returns false on error
	bool BeginObjectLoad();
	
	///returns a pair of bools: the first bool is true if there was an error, the second bool is true if an object was loaded
	std::pair <bool,bool> ContinueObjectLoad( TRACK* track,
		std::map <std::string, MODEL_JOE03> & model_library,
		std::map <std::string, TEXTURE_GL> & texture_library,
		std::list <TRACK_OBJECT> & objects,
		const std::string & texture_size);

	bool GetSurfacesBool();

private:
	const std::string & trackpath;
	std::string objectpath;
	
	JOEPACK pack;
	std::ifstream objectfile;
	
	bool error;
	int numobjects;
	bool packload;
	int anisotropy;
	bool cull;
	
	int params_per_object;
	const int expected_params;
	const int min_params;
	
	bool dynamicshadowsenabled;
	bool agressivecombine;
	
	void CalculateNumObjects();
	
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
};
