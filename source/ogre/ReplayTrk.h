#pragma once
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
//#include <OgreVector3.h>

const static int ciTrkHdrSize = 32;


//  Track's ghost  reduced data
//----------------------------------------------------------------------------------------
struct TrackFrame  // for game
{
	//  time  since game start
	float time;
	//  car,  no wheels
	MATHVECTOR<float,3> pos;
	QUATERNION<float> rot;  //<half>?

	//  info
	char brake, steer;
	//short vel;  char gear;
	
	TrackFrame();
};

//  Track's ghost header
//--------------------------------------------
struct TrackHeader
{
	int ver;
	int frameSize;

	TrackHeader();
	void Default();
};

///  Track's ghost
//--------------------------------------------
class TrackGhost
{
public:
	TrackGhost();

	bool LoadFile(std::string file, bool bLog=true);
	bool SaveFile(std::string file);

	void AddFrame(const TrackFrame& frame);
	bool GetFrame(float time, TrackFrame* fr);

	const float GetTimeLength() const;
	void Clear();

	TrackHeader header;
//private:
	std::vector<TrackFrame> frames;
	int idLast;  // last index from GetFrame
	
	//  test only
	int getNumFrames();
	const TrackFrame& getFrame0(int id);
};
