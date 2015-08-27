#pragma once
#include <OgreVector3.h>
#include <OgreString.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
class SoundBase;


class SoundBaseMgr
{
	friend class SoundBase;
public:
    SoundBaseMgr();
	~SoundBaseMgr();

	void CreateSources(), DestroySources();  // for game reload

	SoundBase* createSound(Ogre::String file);

	void setCamera(Ogre::Vector3 position, Ogre::Vector3 direction, Ogre::Vector3 up, Ogre::Vector3 velocity);
	void pauseAll(bool mute);
	void setMasterVolume(float vol);

	bool isDisabled() {  return device == 0;  }

	int getNumHardwareSources() {  return hw_sources_num;  }

	static const float MAX_DISTANCE, REF_DISTANCE, ROLLOFF_FACTOR;
	static const unsigned int HW_SRC = 128, HW_SRC_HUD = 0,//16
		HW_SRC_ALL = HW_SRC_HUD + HW_SRC;  //par 32
	static const unsigned int MAX_BUFFERS = 1024;  //8192

	ALuint LoadEffect(struct REVERB_PRESET* reverb);

	int hw_sources_num;  // total number of available hardware sources < HW_SRC
	int hw_sources_in_use;
	int sources_in_use;
	int buffers_in_use;

private:
	void recomputeAllSources();
	void recomputeSource(int source_id, int reason, float vfl, Ogre::Vector3 *vvec);
	ALuint getHwSource(int hw_id) {  return hw_sources[hw_id];  };

	void assign(int source_id, int hw_id);
	void retire(int source_id);

	//  load file, return false on error
	bool loadWAVFile(Ogre::String file, ALuint buffer, int& outSamples);
	bool loadOGGFile(Ogre::String file, ALuint buffer, int& outSamples);

	//  active audio sources (hardware sources)
	std::vector<int>  hw_sources_map;   // stores the hardware index for each source. -1 = unmapped
	std::vector<ALuint> hw_sources;     // this buffer contains valid AL handles up to hw_sources_num

	//  audio sources
	std::vector<SoundBase*> sources;
	
	//  helper for calculating the most audible sources
	std::pair<int, float> sources_most_audible[MAX_BUFFERS];
	
	//  audio buffers: Array of AL buffers and filenames
	std::vector<ALuint>  buffers;
	std::vector<Ogre::String> buffer_file;

	Ogre::Vector3 camera_position;

	ALCdevice*    device;
	ALCcontext*   context;

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
