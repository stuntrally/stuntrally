#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../vdrift/pathmanager.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
#include <AL/alc.h>
#include <AL/alext.h>
#include "SoundReverbSets.h"
#include <OgreDataStream.h>
#include <vorbis/vorbisfile.h>
using namespace Ogre;


//  Create
//---------------------------------------------------------------------------------------------
SoundBase* SoundBaseMgr::createSound(String file, String name)
{
	if (!device)  return NULL;

	if (buffers_use >= MAX_BUFFERS)
	{
		LogO("@  SoundManager: Reached MAX_AUDIO_BUFFERS limit (" + toStr(MAX_BUFFERS) + ")");
		return NULL;
	}

	ALuint buffer = 0;
	int samples = 0;

	//  is the file already loaded?
	for (int i=0; i < buffers_use; ++i)
	{
		if (file == buffer_file[i])
		{
			buffer = buffers[i];
			samples = sources[i]->samples;
			break;
		}
	}

	if (!buffer)
	{
		alGenBuffers(1, &buffers[buffers_use]);

		//  load the file
		String ext = file.substr(file.length()-3);
		if (ext == "wav")
		{
			if (!loadWAVFile(file, buffers[buffers_use], samples))
			{	//  error
				alDeleteBuffers(1, &buffers[buffers_use]);
				buffer_file[buffers_use] = "";
				return NULL;
			}
		}else if (ext == "ogg")
		{
			if (!loadOGGFile(file, buffers[buffers_use], samples))
			{	//  error
				alDeleteBuffers(1, &buffers[buffers_use]);
				buffer_file[buffers_use] = "";
				return NULL;
			}
		}else
			LogO("@  Not supported sound file extension: "+ext);
		
		buffer = buffers[buffers_use];
		buffer_file[buffers_use] = file;
	}

	//LogO("@  samples: "+toStr(samples));
	sources[buffers_use] = new SoundBase(buffer, this, buffers_use, samples);
	sources[buffers_use]->name = name;

	return sources[buffers_use++];
}


//---------------------------------------------------------------------------------------------
//  WAV
//---------------------------------------------------------------------------------------------
bool SoundBaseMgr::loadWAVFile(String file, ALuint buffer, int& outSamples)
{
	outSamples = 0;
	//LogO("@  Loading WAV file "+file);

	// create the Stream
	std::string path = PATHMANAGER::Sounds()+"/"+file;
	std::ifstream fi;
	fi.open(path.c_str(), std::ios_base::binary);
	Ogre::FileStreamDataStream fd(&fi,false), *stream = &fd;


	// load RIFF/WAVE
	char magic[5];  magic[4]=0;
	unsigned int   lbuf; // uint32_t
	unsigned short sbuf; // uint16_t

	// check magic
	String s = "Could not read file "+file;
	if (stream->read(magic, 4) != 4) {  LogO(s);  return false;  }
	if (String(magic) != String("RIFF")) {  LogO("Invalid WAV file (no RIFF): "+file);  return false;  }

	// skip 4 bytes (magic)
	stream->skip(4);

	// check file format
	if (stream->read(magic, 4) != 4) {  LogO(s);  return false;  }
	if (String(magic) != String("WAVE")) {  LogO("Invalid WAV file (no WAVE): "+file);  return false;  }

	// check 'fmt ' sub chunk (1)
	if (stream->read(magic, 4) != 4) {  LogO(s);  return false;  }
	if (String(magic) != String("fmt ")) {  LogO("Invalid WAV file (no fmt): "+file);  return false;  }

	// read (1)'s size
	if (stream->read(&lbuf, 4) != 4) {  LogO(s);  return false;  }
	unsigned long subChunk1Size = lbuf;
	if (subChunk1Size<16) {  LogO("Invalid WAV file (invalid subChunk1Size): "+file);  return false;  }

	// check PCM audio format
	if (stream->read(&sbuf, 2) != 2) {  LogO(s);  return false;  }
	unsigned short audioFormat = sbuf;
	if (audioFormat != 1) {  LogO("Invalid WAV file (invalid audioformat "+toStr(audioFormat)+"): "+file);  return false;  }

	// read number of channels
	if (stream->read(&sbuf, 2) != 2) {  LogO(s);  return false;  }
	unsigned short channels = sbuf;

	// read frequency (sample rate)
	if (stream->read(&lbuf, 4) != 4) {  LogO(s);  return false;  }
	unsigned long freq = lbuf;

	// skip 6 bytes (Byte rate (4), Block align (2))
	stream->skip(6);

	// read bits per sample
	if (stream->read(&sbuf, 2) != 2) {  LogO(s);  return false;  }
	unsigned short bps = sbuf;

	// check 'data' sub chunk (2)
	if (stream->read(magic, 4) != 4) {  LogO(s);  return false;  }
	if (String(magic) != String("data") && String(magic) != String("fact")) {  LogO("Invalid WAV file (no data/fact): "+file);  return false;  }

	// fact is an option section we don't need to worry about
	if (String(magic) == String("fact"))
	{
		stream->skip(8);
		// now we should hit the data chunk
		if (stream->read(magic, 4) != 4) {  LogO(s);  return false;  }
		if (String(magic) != String("data")) {  LogO("Invalid WAV file (no data): "+file);  return false;  }
	}
	// the next four bytes are the remaining size of the file
	if (stream->read(&lbuf, 4) != 4) {  LogO(s);  return false;  }
	unsigned long size = lbuf;
	int format = 0;
	outSamples = 0;

		 if (channels == 1 && bps == 16) {	format = AL_FORMAT_MONO16;    outSamples = size/2;  }
	else if (channels == 2 && bps == 16) {	format = AL_FORMAT_STEREO16;  outSamples = size/2 / 2;  }
	else
	{	LogO("@  Invalid WAV file wrong channels: "+toStr(channels)+" or bps: "+toStr(bps)+"  file: "+file);  return false;  }

	if (channels != 1)  // for 3D only mono!
		LogO("@  WAV file is not mono: "+file);

	//  creating buffer
	void* bdata = malloc(size);
	if (!bdata)
	{	LogO("@  Memory error reading file "+file);  return false;  }
	if (stream->read(bdata, size) != size)
	{	LogO("@  Could not read file "+file); free(bdata);  return false;  }

	//LOG("alBufferData: format "+toStr(format)+" size "+toStr(dataSize)+" freq "+toStr(freq));
	alGetError();  // reset errors
	ALint error;

	alBufferData(buffer, format, bdata, size, freq);
	error = alGetError();

	free(bdata);

	if (error != AL_NO_ERROR)
	{	LogO("@  OpenAL error while loading buffer for "+file+" : "+toStr(error));  return false;  }

	fd.close();  fi.close();
	return true;
}


//  OGG
//---------------------------------------------------------------------------------------------
bool SoundBaseMgr::loadOGGFile(String file, ALuint buffer, int& outSamples)
{
	outSamples = 0;
	//LogO("Loading OGG file "+file);

	String path = PATHMANAGER::Sounds()+"/"+file;
	FILE* fp = fopen(path.c_str(), "rb");
	if (fp)
	{
		OggVorbis_File oggFile;
		ov_open_callbacks(fp, &oggFile, NULL, 0, OV_CALLBACKS_DEFAULT);
		vorbis_info* info = ov_info(&oggFile, -1);

		//  assuming ogg is always 16-bit
		unsigned int samples = ov_pcm_total(&oggFile, -1);

		//  allocate space
		unsigned int size = samples * info->channels * 2;
		outSamples = samples;
		char* bdata = new char[size];

		int bitstream, bytes = 1;
		unsigned int bufpos = 0;
		while (bytes > 0)
		{
			bytes = ov_read(&oggFile, bdata+bufpos, size-bufpos,
				0/*little endian*/, 2/**/, 1/*issigned*/, &bitstream);
			bufpos += bytes;
		}

		int format = 0;
			 if (info->channels == 1)  format = AL_FORMAT_MONO16;
		else if (info->channels == 2)  format = AL_FORMAT_STEREO16;

		alGetError();  // reset errors
		ALint error;
		alBufferData(buffer, format, bdata, size, info->rate/*freq*/);
		error = alGetError();

		delete[] bdata;  // stream will be closed by itself

		if (error != AL_NO_ERROR)
		{	LogO("@  OpenAL error while loading buffer for "+file+" : "+toStr(error));  return false;  }

		//note: no need to call fclose(); ov_clear does it for us
		ov_clear(&oggFile);
		return true;
	}
	LogO("@  Can't open OGG sound file: "+file);  return false;
}


//---------------------------------------------------------------------------------------------
bool SoundBaseMgr::_checkALErrors(const char* file, int line)
{
	int err = alGetError();
	if (err != AL_NO_ERROR)
	{
		char buf[4096] = {};
		sprintf(buf, "))  OpenAL Error: %s (0x%x), @ %s:%d", alGetString(err), err, file, line);
		LogO(buf);
		return true;
	}
	return false;
}

//  Load Effect
//---------------------------------------------------------------------------------------------
ALuint SoundBaseMgr::LoadEffect(const REVERB_PRESET* r)
{
	//  Create the effect object and check if we can do EAX reverb.
	ALuint e = 0;
	alGenEffects(1, &e);
	if (alGetEnumValue("AL_EFFECT_EAXREVERB") != 0)
	{
		LogO("@  Using EAX Reverb");
		alEffecti(e, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
		alEffectf(e, AL_EAXREVERB_DENSITY, r->flDensity);
		alEffectf(e, AL_EAXREVERB_DIFFUSION, r->flDiffusion);
		alEffectf(e, AL_EAXREVERB_GAIN, r->flGain);
		alEffectf(e, AL_EAXREVERB_GAINHF, r->flGainHF);
		alEffectf(e, AL_EAXREVERB_GAINLF, r->flGainLF);
		alEffectf(e, AL_EAXREVERB_DECAY_TIME, r->flDecayTime);
		alEffectf(e, AL_EAXREVERB_DECAY_HFRATIO, r->flDecayHFRatio);
		alEffectf(e, AL_EAXREVERB_DECAY_LFRATIO, r->flDecayLFRatio);
		alEffectf(e, AL_EAXREVERB_REFLECTIONS_GAIN, r->flReflectionsGain);
		alEffectf(e, AL_EAXREVERB_REFLECTIONS_DELAY, r->flReflectionsDelay);
		alEffectfv(e,AL_EAXREVERB_REFLECTIONS_PAN, r->flReflectionsPan);
		alEffectf(e, AL_EAXREVERB_LATE_REVERB_GAIN, r->flLateReverbGain);
		alEffectf(e, AL_EAXREVERB_LATE_REVERB_DELAY, r->flLateReverbDelay);
		alEffectfv(e,AL_EAXREVERB_LATE_REVERB_PAN, r->flLateReverbPan);
		alEffectf(e, AL_EAXREVERB_ECHO_TIME, r->flEchoTime);
		alEffectf(e, AL_EAXREVERB_ECHO_DEPTH, r->flEchoDepth);
		alEffectf(e, AL_EAXREVERB_MODULATION_TIME, r->flModulationTime);
		alEffectf(e, AL_EAXREVERB_MODULATION_DEPTH, r->flModulationDepth);
		alEffectf(e, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, r->flAirAbsorptionGainHF);
		alEffectf(e, AL_EAXREVERB_HFREFERENCE, r->flHFReference);
		alEffectf(e, AL_EAXREVERB_LFREFERENCE, r->flLFReference);
		alEffectf(e, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, r->flRoomRolloffFactor);
		alEffecti(e, AL_EAXREVERB_DECAY_HFLIMIT, r->iDecayHFLimit);
	}else{
		LogO("@  Using Standard Reverb");
		alEffecti(e, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
		alEffectf(e, AL_REVERB_DENSITY, r->flDensity);
		alEffectf(e, AL_REVERB_DIFFUSION, r->flDiffusion);
		alEffectf(e, AL_REVERB_GAIN, r->flGain);
		alEffectf(e, AL_REVERB_GAINHF, r->flGainHF);
		alEffectf(e, AL_REVERB_DECAY_TIME, r->flDecayTime);
		alEffectf(e, AL_REVERB_DECAY_HFRATIO, r->flDecayHFRatio);
		alEffectf(e, AL_REVERB_REFLECTIONS_GAIN, r->flReflectionsGain);
		alEffectf(e, AL_REVERB_REFLECTIONS_DELAY, r->flReflectionsDelay);
		alEffectf(e, AL_REVERB_LATE_REVERB_GAIN, r->flLateReverbGain);
		alEffectf(e, AL_REVERB_LATE_REVERB_DELAY, r->flLateReverbDelay);
		alEffectf(e, AL_REVERB_AIR_ABSORPTION_GAINHF, r->flAirAbsorptionGainHF);
		alEffectf(e, AL_REVERB_ROOM_ROLLOFF_FACTOR, r->flRoomRolloffFactor);
		alEffecti(e, AL_REVERB_DECAY_HFLIMIT, r->iDecayHFLimit);
	}

	//  Check if an error occured, and clean up if so.
	if (hasALErrors())
	{
		if (alIsEffect(e))
			alDeleteEffects(1, &e);
		return 0;
	}
	return e;
}
