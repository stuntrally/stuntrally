#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
using namespace Ogre;


SoundBase::SoundBase(ALuint buffer1, SoundBaseMgr* sound_mgr1, int source_id1, int samples1)
	:buffer(buffer1), sound_mgr(sound_mgr1), source_id(source_id1)
	,audibility(0.0f), gain(0.0f), pitch(1.0f)
	,pos(Vector3::ZERO), vel(Vector3::ZERO)
	,enabled(true), loop(false), should_play(false), hw_id(-1)
	,samples(samples1), is2D(false)
{
}

void SoundBase::computeAudibility(Vector3 pos1)
{
	//audibility = 1.f;
	//return;
	
	//  disable sound?
	if (!enabled)
	{
		audibility = 0.0f;
		return;
	}

	//  first check if the sound is finished!
	if (!is2D && !loop && should_play && hw_id != -1)
	{
		int value = 0;
		alGetSourcei((ALuint)sound_mgr->getHwSource(hw_id), AL_SOURCE_STATE, &value);
		if (value != AL_PLAYING)
			should_play = false;
	}
	
	//  should it play at all?
	if (!should_play || gain == 0.0f)
	{
		audibility = 0.0f;
		return;
	}
	
	//  hud
	if (is2D)
	{
		audibility = gain;
		return;
	}		

	//  3d
	float distance = (pos1 - pos).length();
	
	if (distance > sound_mgr->MAX_DISTANCE)
		audibility = 0.0f;
	else if (distance < sound_mgr->REF_DISTANCE)
		audibility = gain;
	else  //  same eq as in alDistanceModel
		audibility = gain * (sound_mgr->REF_DISTANCE /
			(sound_mgr->REF_DISTANCE + sound_mgr->ROLLOFF_FACTOR * (distance - sound_mgr->REF_DISTANCE) ) );
}

bool SoundBase::isPlaying()
{
	if (hw_id != -1)
	{
		int value = 0;
		alGetSourcei((ALuint)sound_mgr->getHwSource(hw_id), AL_SOURCE_STATE, &value);
		return (value == AL_PLAYING);
	}
	return false;
}

void SoundBase::setEnabled(bool e)
{
	enabled = e;
	sound_mgr->recomputeSource(source_id, REASON_PLAY, 0.0f, NULL);
}

bool SoundBase::getEnabled()
{
	return enabled;
}

void SoundBase::play()
{
	should_play = true;
	sound_mgr->recomputeSource(source_id, REASON_PLAY, 0.0f, NULL);
}

void SoundBase::stop()
{
	should_play = false;
	sound_mgr->recomputeSource(source_id, REASON_STOP, 0.0f, NULL);
}

void SoundBase::setGain(float gain1)
{
	gain = gain1;
	sound_mgr->recomputeSource(source_id, REASON_GAIN, gain, NULL);
}

void SoundBase::setLoop(bool loop1)
{
	loop = loop1;
	sound_mgr->recomputeSource(source_id, REASON_LOOP, loop ? 1.0f : 0.0f, NULL);
}

void SoundBase::setPitch(float pitch1)
{
	pitch = pitch1;
	sound_mgr->recomputeSource(source_id, REASON_PTCH, pitch, NULL);
}

void SoundBase::setPosition(Ogre::Vector3 pos1)
{
	pos = pos1;
	sound_mgr->recomputeSource(source_id, REASON_POS, 0.0f, &pos);
}

void SoundBase::setVelocity(Ogre::Vector3 vel1)
{
	vel = vel1;
	sound_mgr->recomputeSource(source_id, REASON_VEL, 0.0f, &vel);
}

void SoundBase::seek(float pos)  // [0..1)
{
	sound_mgr->recomputeSource(source_id, REASON_SEEK, pos * samples, NULL);
}
