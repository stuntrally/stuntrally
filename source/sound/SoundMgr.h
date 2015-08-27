#pragma once


enum {
	MAX_SOUNDS_PER_SCRIPT = 8,  // per 1 template
	MAX_INSTANCES_PER_GROUP = 256
};
namespace Ogre  {  class FileStreamDataStream;  }
class SoundBase;  class SoundBaseMgr;


//---------------------------------------------------------------------------------------
class SoundTemplate
{
	friend class SoundMgr;
	friend class Sound;

public:
	SoundTemplate(Ogre::String name, Ogre::String filename);
	
private:
	bool setParameter(Ogre::StringVector vec);

	Ogre::String name;
	Ogre::String file_name;

	//bool  base_template;
	bool  has_start_sound, has_stop_sound;
	bool  unpitchable;

	Ogre::String sound_names[MAX_SOUNDS_PER_SCRIPT];
	float        sound_pitches[MAX_SOUNDS_PER_SCRIPT];
	Ogre::String start_sound_name;
	float        start_sound_pitch;
	Ogre::String stop_sound_name;
	float        stop_sound_pitch;

	int   trigger_source;
	int   free_sound;
};


//---------------------------------------------------------------------------------------
class Sound
{
	friend class SoundMgr;

public:
	Sound(int car, SoundTemplate* tpl, SoundBaseMgr* mgr, Ogre::String name);
	~Sound();

	void setGain(float value);
	void setPitch(float value);
	void setPosition(Ogre::Vector3 pos, Ogre::Vector3 velocity);

	bool isAudible();
	void start(), stop(), kill();
	bool is2D;  // hud sounds, no distance attenuation
	void set2D(bool b);

	void seek(float pos);
	void runOnce();
	void setEnabled(bool e);

	static const float PITCHDOWN_FADE_FACTOR;
	static const float PITCHDOWN_CUTOFF_FACTOR;

private:
	float pitchgain_cutoff(float sourcepitch, float targetpitch);

	SoundTemplate* templ;
	SoundBaseMgr* sound_mgr;

	SoundBase* start_sound, *stop_sound;
	SoundBase* sounds[MAX_SOUNDS_PER_SCRIPT];

	float start_sound_pitchgain;
	float stop_sound_pitchgain;
	float sounds_pitchgain[MAX_SOUNDS_PER_SCRIPT];
	float lastgain;

	int car;  //  number of the car this is for
};

//---------------------------------------------------------------------------------------
class SoundMgr
{
public:
	SoundMgr();
	~SoundMgr();

	//  interface
	void parseScript(Ogre::FileStreamDataStream* stream);

	Sound* createInstance(Ogre::String templatename, int car);


	//  functions
	void setPaused(bool mute);
	void setMasterVolume(float vol);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);

	SoundBaseMgr* sound_mgr;

private:
	SoundTemplate* createTemplate(Ogre::String name, Ogre::String filename);
	void skipToNextCloseBrace(Ogre::FileStreamDataStream* chunk);
	void skipToNextOpenBrace(Ogre::FileStreamDataStream* chunk);

	bool disabled;
	int instance_counter;

	std::map <Ogre::String, SoundTemplate*> templates;
};
