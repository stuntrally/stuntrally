#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../vdrift/pathmanager.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include "SoundReverbSets.h"
using namespace Ogre;


//const float SoundBaseMgr::MAX_DISTANCE   = 500.f;  // for linear
//const float SoundBaseMgr::REF_DISTANCE   = 1.0f;
//const float SoundBaseMgr::ROLLOFF_FACTOR = 3.0f;
const float SoundBaseMgr::MAX_DISTANCE	 = 500.f;  // 500  inverse
const float SoundBaseMgr::REF_DISTANCE   = 1.0f;   // 1
const float SoundBaseMgr::ROLLOFF_FACTOR = 0.05f;  // 0.05 0.1


//  Init
//---------------------------------------------------------------------------------------------
SoundBaseMgr::SoundBaseMgr()
	:buffers_in_use(0), sources_in_use(0)
	,hw_sources_in_use(0), hw_sources_num(0)
	,context(NULL), device(NULL)
	,slot(0), effect(0), master_volume(1.f)
{
	hw_sources_map.resize(HW_SRC_ALL);
	hw_sources.resize(HW_SRC_ALL);
	sources.resize(MAX_BUFFERS);
	//std::pair<int, float> sources_most_audible[MAX_BUFFERS];
	buffers.resize(MAX_BUFFERS);
	buffer_file.resize(MAX_BUFFERS);


	//  open device
	String sdevice = "";  //par
	if (sdevice == "")
		device = alcOpenDevice(NULL);
	else
		device = alcOpenDevice(sdevice.c_str());

	if (!device)
	{
		LogO("@@@  Sound Init - Could not open device");
		hasALErrors();
		return;
	}

	//  efx
	ALCboolean efx = alcIsExtensionPresent(device, "ALC_EXT_EFX");
	if (efx == ALC_FALSE)		LogO("@  EFX extention not found !");
	else if (efx == ALC_TRUE)	LogO("@  EFX extension found.");

	ALint attr[4] = { 0 };
	attr[0] = ALC_MAX_AUXILIARY_SENDS;
	attr[1] = 4;


	//  context
	context = alcCreateContext(device, attr/*NULL*/);
	if (context == NULL ||
		alcMakeContextCurrent(context) == ALC_FALSE)
	{
		LogO("@@@  Sound Init - Could not create context");
		if (context != NULL)
			alcDestroyContext(context);
		alcCloseDevice(device);
		device = NULL;
		hasALErrors();
		return;
	}

	
	//  log info  ----
	String s,t;
	LogO("@ @  ---- SoundManager Info ----");
	s = alGetString(AL_VENDOR);		LogO("@  vendor: " + s);
	s = alGetString(AL_VERSION);	LogO("@  version: " + s);
	s = alGetString(AL_RENDERER);	LogO("@  renderer: " + s);
	//t = alcGetString(device, ALC_DEVICE_SPECIFIER);	LogO("@  renderer: " + s + "  alc device: " + t);
	s = alGetString(AL_EXTENSIONS);	LogO("@  extensions: " + s);
	//t = alcGetString(device, ALC_EXTENSIONS);	LogO("@  alc extensions: " + s);


	//  sends
	ALint iSends = 0;
	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &iSends);
	LogO("@  Aux Sends per Source: " + toStr(iSends));

   
	//  get function pointers
	alGenEffects    = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
	alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
	alEffecti  = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectf  = (LPALEFFECTF)alGetProcAddress("alEffectf");
	alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");


	//  doppler
	//alDopplerFactor(1.0f);
	//alDopplerVelocity(343.0f);

	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);  //+
	//alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	//alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);

	//  reverb
	//return;
	REVERB_PRESET reverb = ReverbPresets[
		//RVB_GENERIC];
		//RVB_CAVE];
		//RVB_ARENA];
		//RVB_HANGAR];
		//RVB_FOREST];
		RVB_MOUNTAINS];  //`
		//RVB_UNDERWATER];
		//RVB_CASTLE_LARGEROOM];
		//RVB_FACTORY_HALL];
		//RVB_SPACESTATION_HALL];
		//RVB_WOODEN_HALL];
		//RVB_DOME_TOMB];
		//RVB_PREFAB_CARAVAN];
		//RVB_PIPE_LARGE];
		//RVB_PIPE_RESONANT];
		//RVB_OUTDOORS_VALLEY];
		//RVB_OUTDOORS_ROLLINGPLAINS];  //`
		//RVB_MOOD_HELL];
		//RVB_DRUGGED];

	effect = LoadEffect(&reverb);
	if (!effect)
		LogO("@  Can't load effect !!");

	//  This is what plays an effect on sources that connect to it
	alGenAuxiliaryEffectSlots(1, &slot);

	//  Tell the effect slot to use the loaded effect object. Note that the this
	//  effectively copies the effect properties. You can modify or delete the
	//  effect object afterward without affecting the effect slot.
	alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
	if (alGetError() != AL_NO_ERROR)
		LogO("@  Failed to set effect slot");


	//  hud sources  --
	LogO("@ @  Creating hw sources hud.");
	int i;
	for (i = HW_SRC; i < HW_SRC + HW_SRC_HUD; ++i)
	{
		alGetError();
		alGenSources(1, &hw_sources[i]);
		//alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);

		if (alGetError() != AL_NO_ERROR)  break;
		alSourcef(hw_sources[i], AL_REFERENCE_DISTANCE, REF_DISTANCE);
		alSourcef(hw_sources[i], AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR);
		alSourcef(hw_sources[i], AL_MAX_DISTANCE, MAX_DISTANCE);
		++hw_sources_num;
	}

	for (i=0; i < HW_SRC_ALL; ++i)
		hw_sources_map[i] = -1;
}

//  Create  --
void SoundBaseMgr::CreateSources()
{
	if (!device)  return;
	LogO("@ @  Creating hw sources.");
	int i;
	for (i = 0; i < HW_SRC; ++i)
	{
		alGetError();
		alGenSources(1, &hw_sources[i]);
		//alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);

		if (alGetError() != AL_NO_ERROR)  break;
		alSourcef(hw_sources[i], AL_REFERENCE_DISTANCE, REF_DISTANCE);
		alSourcef(hw_sources[i], AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR);
		alSourcef(hw_sources[i], AL_MAX_DISTANCE, MAX_DISTANCE);
		//LogO(toStr(i)+" +SRC: "+toStr(hw_sources[i]));
		++hw_sources_num;
	}

	for (i = 0; i < HW_SRC; ++i)
		hw_sources_map[i] = -1;
}

//  Destroy  --
void SoundBaseMgr::DestroySources()
{
	if (!device)  return;
	//if (hw_sources_num <= HW_SRC_HUD)  return;
	
	/*for (int i = 0; i < sources.size(); ++i)
	{
		retire(i);
		delete sources[i];
	}/**/
	//camera_position = Vector3(10000.f,10000.f,10000.f);
	//recomputeSource()

	LogO("@ @  Destroying hw sources.");
	for (int i = 0; i < HW_SRC; ++i)
	{
		//LogO(toStr(i)+" -SRC: "+toStr(hw_sources[i]));
		alSourceStop(hw_sources[i]);
		alSourcei(hw_sources[i], AL_BUFFER, 0);
		alDeleteSources(1, &hw_sources[i]);
		--hw_sources_num;
	}
	// ??
	//buffers_in_use = 0;  sources_in_use = 0;
	hw_sources_in_use = 0;  //in retire  //hw_sources_num = 0;
}

//  Destroy
SoundBaseMgr::~SoundBaseMgr()
{
	//todo: leaks..
	//for (int i=0; i < buffers_in_use; ++i)
	//	if (sources[i] && sources[i]->hw_id != -1)
	//		delete sources[i];
	if (device)
	{
		alDeleteAuxiliaryEffectSlots(1, &slot);
		alDeleteEffects(1, &effect);

		//  delete sources and buffers
		//DestroySources();  //..
		//alDeleteSources(MAX_HW_SOURCES, hw_sources);
		int i;
		//for (i = HW_SRC; i < HW_SRC + HW_SRC_HUD; ++i)
		for (i = 0; i < HW_SRC_ALL; ++i)
		{
			alDeleteSources(1, &hw_sources[i]);
		}
		alDeleteBuffers(MAX_BUFFERS, &buffers[0]);
	}

	//  context and device
	ALCcontext* context = alcGetCurrentContext();
	if (context == NULL)
	{	LogO("@ @  SoundManager was disabled.");
		return;
	}
	ALCdevice* device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	if (device)
		alcCloseDevice(device);

	LogO("@ @  SoundManager destroyed.");
}


//  Update
//-----------------------------------------------------------------------------------
void SoundBaseMgr::setCamera(Vector3 pos, Vector3 dir, Vector3 up, Vector3 vel)
{
	if (!device)  return;
	camera_position = pos;
	recomputeAllSources();

	float o[6];
	o[0] = dir.x;  o[1] = dir.y;  o[2] = dir.z;
	o[3] = up.x;  o[4] = up.y;  o[5] = up.z;

	alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
	alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
	alListenerfv(AL_ORIENTATION, o);
}

bool compareByAudibility(std::pair<int, float> a, std::pair<int, float> b)
{
	return a.second > b.second;
}

//  called when camera moves
void SoundBaseMgr::recomputeAllSources()
{
	if (!device)  return;

	for (int i=0; i < sources_in_use; i++)
	{
		sources[i]->computeAudibility(camera_position);
		sources_most_audible[i].first = i;
		sources_most_audible[i].second = sources[i]->audibility;
	}

	//  sort first 'num_hardware_sources' sources by audibility
	//  see: https://en.wikipedia.org/wiki/Selection_algorithm
	if ((sources_in_use - 1) > hw_sources_num)
		std::nth_element(sources_most_audible, sources_most_audible+hw_sources_num, sources_most_audible+sources_in_use-1, compareByAudibility);

	// retire out of range sources first
	for (int i=0; i < sources_in_use; i++)
		if (sources[sources_most_audible[i].first]->hw_id != -1 && (i >= hw_sources_num || sources_most_audible[i].second == 0))
			retire(sources_most_audible[i].first);

	// assign new sources
	for (int i=0; i < std::min(sources_in_use, hw_sources_num); i++)
	if (sources[sources_most_audible[i].first]->hw_id == -1 && sources_most_audible[i].second > 0)
		for (int j=0; j < hw_sources_num; j++)
		if (hw_sources_map[j] == -1)
		{
			assign(sources_most_audible[i].first, j);
			break;
		}
}


//  recompute Source
//---------------------------------------------------------------------------------------------------------------------------
void SoundBaseMgr::recomputeSource(int id, int reason, float fl, Vector3* vec)
{
	if (!device)  return;
	sources[id]->computeAudibility(camera_position);

	if (sources[id]->audibility == 0.0f)
	{
		if (sources[id]->hw_id != -1)
			//  retire the source if it is currently assigned
			retire(id);
	}else
	{
		//  this is a potentially audible sources[id]
		if (sources[id]->hw_id != -1)
		{
			//  sources[id] already playing
			//  update the AL settings
			switch (reason)
			{
			case REASON_GAIN:  alSourcef(hw_sources[sources[id]->hw_id], AL_GAIN, fl * master_volume); break;
			case REASON_PTCH:  alSourcef(hw_sources[sources[id]->hw_id], AL_PITCH, fl); break;
			case REASON_POS:  alSource3f(hw_sources[sources[id]->hw_id], AL_POSITION, vec->x, vec->y, vec->z); break;
			case REASON_VEL:  alSource3f(hw_sources[sources[id]->hw_id], AL_VELOCITY, vec->x, vec->y, vec->z); break;

			case REASON_PLAY:  alSourcePlay(hw_sources[sources[id]->hw_id]); break;
			case REASON_STOP:  alSourceStop(hw_sources[sources[id]->hw_id]); break;
			case REASON_LOOP:  alSourcei(hw_sources[sources[id]->hw_id], AL_LOOPING, fl > 0.5f ? AL_TRUE : AL_FALSE); break;
			case REASON_SEEK:  alSourcei(hw_sources[sources[id]->hw_id], AL_SAMPLE_OFFSET, fl); break;
			default: break;
			}
		}else
		{
			//  try to make it play by the hardware
			//  check if there is one free sources[id] in the pool
			if (hw_sources_in_use < hw_sources_num)
			{
				for (int i=0; i < hw_sources_num; i++)
				{
					if (hw_sources_map[i] == -1)
					{
						assign(id, i);
						break;
					}
				}
			}else
			{
				//  now, compute who is the faintest
				//  note: we know the table m_hardware_sources_map is full!
				float fv = 1.0f;
				int al_faintest = 0;
				for (int i=0; i < hw_sources_num; i++)
				{
					if (hw_sources_map[i] >= 0 && sources[hw_sources_map[i]]->audibility < fv)
					{
						fv = sources[hw_sources_map[i]]->audibility;
						al_faintest = i;
					}
				}
				//  check to ensure that the sound is louder than the faintest sound currently playing
				if (fv < sources[id]->audibility)
				{
					// this new sources[id] is louder than the faintest!
					retire(hw_sources_map[al_faintest]);
					assign(id, al_faintest);
				}
				//  else this sources[id] is too faint, we don't play it!
			}
		}
	}
}


//  assign
//-----------------------------------------------------------------------------------

void SoundBaseMgr::assign(int id, int hw_id)
{
	if (!device)  return;
	sources[id]->hw_id = hw_id;
	hw_sources_map[hw_id] = id;

	//  the hardware source is supposed to be stopped!
	ALuint source = hw_sources[hw_id];
	alSourcei(source, AL_BUFFER, sources[id]->buffer);

	// use reverb +
	alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);

	alSourcef(source, AL_GAIN, sources[id]->gain * master_volume);
	alSourcei(source, AL_LOOPING, sources[id]->loop ? AL_TRUE : AL_FALSE);
	alSourcef(source, AL_PITCH, sources[id]->pitch);

	alSource3f(source, AL_POSITION, sources[id]->pos.x, sources[id]->pos.y, sources[id]->pos.z);
	alSource3f(source, AL_VELOCITY, sources[id]->vel.x, sources[id]->vel.y, sources[id]->vel.z);

	if (sources[id]->is2D)  // hud
	{
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(source, AL_POSITION, 0.0, 0.0, 0.0);
		alSourcef(source, AL_GAIN, 1.f);
		alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);

		//alSourcef(source, AL_REFERENCE_DISTANCE, FLT_MAX-10.0f);
		//alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
		//alSourcef(source, AL_MAX_DISTANCE, FLT_MAX-9.0f);
	}

	if (sources[id]->should_play)
		alSourcePlay(hw_sources[hw_id]);

	++hw_sources_in_use;
}


void SoundBaseMgr::retire(int id)
{
	if (!device)  return;
	if (sources[id]->hw_id == -1)  return;
	
	alSourceStop(hw_sources[sources[id]->hw_id]);
	hw_sources_map[sources[id]->hw_id] = -1;
	sources[id]->hw_id = -1;
	--hw_sources_in_use;
}


//  utility
void SoundBaseMgr::pauseAll(bool mute)
{
	if (!device)  return;
	alListenerf(AL_GAIN, mute ? 0.0f : master_volume);
}

void SoundBaseMgr::setMasterVolume(float vol)
{
	if (!device)  return;
	master_volume = vol;
	alListenerf(AL_GAIN, master_volume);
}
