#pragma once
#include <Ogre.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
class SoundBase;  struct REVERB_PRESET;

//#define REVERB_BROWSER  // _tool_  keys 1,2: change prev,next reverb preset


class SoundBaseMgr
{
	friend class SoundBase;
public:
    SoundBaseMgr();
	~SoundBaseMgr();

	//  main  ---
	bool Init(std::string snd_device, bool reverb1);
	void CreateSources(), DestroySources(bool all=false);  // for game reload

	SoundBase* createSound(Ogre::String file, Ogre::String name);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
	void pauseAll(bool mute);
	void setMasterVolume(float vol);

	bool isDisabled() {  return device == 0;  }


	//  const
	static const float MAX_DISTANCE, REF_DISTANCE, ROLLOFF_FACTOR;
	static const unsigned int HW_SRC = 256;  //par
	static const unsigned int MAX_BUFFERS = 1024;  //

	//  reverb  ---
	void SetReverb(std::string name);

	bool reverb;
	std::string sReverb;  // info
	void InitReverbMap();
	std::map <std::string, int> mapReverbs;
	ALuint LoadEffect(const REVERB_PRESET* reverb);

	//  var
	int hw_sources_num;  // total number of available hardware sources < HW_SRC
	int hw_sources_use;
	int sources_use;
	int buffers_use, buffers_used_max;

//private:
	void recomputeAllSources();
	void recomputeSource(int source_id, int reason, float vfl, Ogre::Vector3* vvec);
	ALuint getHwSource(int hw_id) {  return hw_sources[hw_id];  };

	void assign(int source_id, int hw_id);
	void retire(int source_id);

	//  load file, return false on error
	bool loadWAVFile(Ogre::String file, ALuint buffer, int& outSamples);
	bool loadOGGFile(Ogre::String file, ALuint buffer, int& outSamples);

	//  ambient sound (own, low level)
	ALuint amb_source, amb_buffer;

	
	//  active audio sources (hardware sources)
	std::vector<int>  hw_sources_map;   // stores the hardware index for each source. -1 = unmapped
	std::vector<ALuint> hw_sources;     // this buffer contains valid AL handles up to hw_sources_num

	//  audio sources
	std::vector<SoundBase*> sources;
	
	//  helper for calculating the most audible sources
	std::pair<int, float> src_audible[MAX_BUFFERS];
	
	//  audio buffers: Array of AL buffers and filenames
	std::vector<ALuint>  buffers;
	std::vector<Ogre::String> buffer_file;

	Ogre::Vector3 camera_position;

	//  al vars
	ALCdevice*  device;
	ALCcontext* context;

	ALuint slot, effect;

	float master_volume;


	//  function pointers
	bool _checkALErrors(const char* file, int line);
	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALISEFFECT alIsEffect;
	LPALEFFECTI alEffecti;
	LPALEFFECTF alEffectf;
	LPALEFFECTFV alEffectfv;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
};

#define hasALErrors()  _checkALErrors(__FILE__, __LINE__)
