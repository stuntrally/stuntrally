#pragma once
#include <AL/al.h>
#include <Ogre.h>

enum RecomputeSource {
	REASON_PLAY, REASON_STOP, REASON_GAIN, REASON_LOOP, REASON_PTCH, REASON_POS, REASON_VEL, REASON_SEEK };
class SoundBaseMgr;


class SoundBase
{
	friend class SoundBaseMgr;

public:
	SoundBase(ALuint buffer1, SoundBaseMgr* sound_mgr1, int source_id1, int samples1);
	Ogre::String name;

	void setPitch(float pitch);
	void setGain(float gain);

	void setPosition(Ogre::Vector3 pos);
	void setVelocity(Ogre::Vector3 vel);

	void setLoop(bool loop);
	void setEnabled(bool e);

	void play();
	void stop();

	bool getEnabled();
	bool isPlaying();
	
	void seek(float pos);  // [0..1)
	bool is2D;

//private:
	void computeAudibility(Ogre::Vector3 pos);

	//  must not be changed during the lifetime of this object
	int source_id;
	int samples;

	float audibility;
	float gain;
	float pitch;
	
	bool loop;
	bool enabled;
	bool should_play;

	//  this value is changed dynamically, depending on whether the input is played or not.
	int hw_id;
	ALuint buffer;
	
	Ogre::Vector3 pos;
	Ogre::Vector3 vel;

	SoundBaseMgr* sound_mgr;
};
