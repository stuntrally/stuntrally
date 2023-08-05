//  based on RoR's  /source/main/audio/SoundScriptManager.h
//  https://github.com/RigsOfRods/rigs-of-rods
#pragma once
#include <OgreString.h>
#include <OgreStringVector.h>
#include <map>

namespace Ogre  {  class FileStreamDataStream;  }
class SoundBase;  class SoundBaseMgr;

const int MAX_SOUNDS_PER_SCRIPT = 8;  // per 1 template


///  sound template  from .cfg to create
//---------------------------------------------------------------------------------------
class SoundTemplate
{
	friend class SoundMgr;
	friend class Sound;

public:
	SoundTemplate(Ogre::String name, Ogre::String filename);
	
private:
	bool setParameter(Ogre::StringVector vec);

	Ogre::String name, file_name;

	bool  has_start, has_stop;
	bool  unpitchable;

	Ogre::String sound_names[MAX_SOUNDS_PER_SCRIPT];
	float        sound_pitches[MAX_SOUNDS_PER_SCRIPT];
	Ogre::String start_name, stop_name;

	int   free_sound;
};


///  Sound instance
//---------------------------------------------------------------------------------------
class Sound
{
	friend class SoundMgr;

public:
	Sound(int car, SoundTemplate* tpl, SoundBaseMgr* mgr);
	~Sound();

	void setGain(float value);
	void setPitch(float value);
	void setPosition(Ogre::Vector3 pos, Ogre::Vector3 velocity);

	bool isAudible();
	void start(), stop(), kill();
	bool is2D;  // hud sounds, no distance attenuation
	void set2D(bool b), setEngine(bool b);

	void seek(float pos);
	void runOnce();
	void setEnabled(bool e);

private:
	SoundTemplate* templ;
	SoundBaseMgr* sound_mgr;

	SoundBase* start_sound, *stop_sound;
	SoundBase* sounds[MAX_SOUNDS_PER_SCRIPT];

	float pitch_gain[MAX_SOUNDS_PER_SCRIPT];
	float lastgain;

	int car;  //  number of the car this is for
	bool engine;
};


///  Sounds manager
//---------------------------------------------------------------------------------------
class SoundMgr
{
public:
	SoundMgr();
	~SoundMgr();
	bool Init(std::string snd_device, bool reverb);

	void parseScript(Ogre::FileStreamDataStream* stream);  // sounds.cfg

	Sound* createInstance(Ogre::String templatename, int car);  // new Sound

	void setPaused(bool mute);
	void setMasterVolume(float vol);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
	bool isDisabled() {  return disabled;  }

	SoundBaseMgr* sound_mgr;

private:
	SoundTemplate* createTemplate(Ogre::String name, Ogre::String filename);
	void skipToNextCloseBrace(Ogre::FileStreamDataStream* chunk);
	void skipToNextOpenBrace(Ogre::FileStreamDataStream* chunk);

	bool disabled;

	std::map <Ogre::String, SoundTemplate*> templates;
	std::vector<SoundTemplate*> v_templ;  // to delete
};
