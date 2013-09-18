#pragma once
#include "bezier.h"
#include "mathvector.h"
#include "vertexarray.h"

//class DRAWABLE;
//class SCENENODE;
class TEXTURE_GL;

class ROADPATCH
{
public:
	ROADPATCH() : track_curvature(0)/*, racingline_draw(NULL)*/ {}
	
	const BEZIER & GetPatch() const {return patch;}
	
	BEZIER & GetPatch() {return patch;}
	
	///return true if the ray starting at the given origin going in the given direction intersects this patch.
	/// output the contact point and normal to the given outtri and normal variables.
	bool Collide(
		const MATHVECTOR<float,3> & origin,
		const MATHVECTOR<float,3> & direction,
		float seglen,
		MATHVECTOR<float,3> & outtri,
		MATHVECTOR<float,3> & normal) const;

	float GetTrackCurvature() const
	{
		return track_curvature;
	}

	MATHVECTOR<float,3> GetRacingLine() const
	{
		return racing_line;
	}

	void SetTrackCurvature ( float value )
	{
		track_curvature = value;
	}

	void SetRacingLine ( const MATHVECTOR< float, 3 >& value )
	{
		racing_line = value;
		patch.racing_line = value;
		patch.have_racingline = true;
	}
	
	/*void AddRacinglineScenenode(
		SCENENODE * node,
		ROADPATCH * nextpatch, 
		TEXTURE_GL & racingline_texture,
		std::ostream & error_output);*/
	
private:
	BEZIER patch;
	float track_curvature;
	MATHVECTOR<float,3> racing_line;
	
	//DRAWABLE * racingline_draw;
public:	
	VERTEXARRAY racingline_vertexarray;
};
