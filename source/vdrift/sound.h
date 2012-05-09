#ifndef _SOUND_H
#define _SOUND_H

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <fstream>

#include "quaternion.h"
#include "mathvector.h"

#include <SDL/SDL.h>


class SOUNDINFO
{
private:
	unsigned int samples, frequency, bytespersample, channels;
	
public:
	SOUNDINFO(unsigned int numsamples, unsigned int freq, unsigned int chan, unsigned int bytespersamp)
		: samples(numsamples), frequency(freq), bytespersample(bytespersamp), channels(chan)
	{  }

	void DebugPrint(std::ostream & out) const
	{
		out << "Samples: " << samples << std::endl;
		out << "Frequency: " << frequency << std::endl;
		out << "Channels: " << channels << std::endl;
		out << "Bits per sample: " << bytespersample*8 << std::endl;
	}
	inline int GetSamples() const   {  return samples;  }
	inline int GetFrequency() const {  return frequency;  }
	inline int GetChannels() const  {  return channels;  }
	inline int GetBytesPerSample() const {  return bytespersample;  }
	
	bool operator==(const SOUNDINFO & other) const
	{
		return (samples == other.samples && frequency == other.frequency && channels == other.channels && bytespersample == other.bytespersample);
	}
};


class SOUNDBUFFER
{
	friend class SOUNDSOURCE;
private:
	SOUNDINFO info;
	unsigned int size;
	bool loaded;
	char * sound_buffer;
	std::string name;

	bool LoadWAV(const std::string & filename, const SOUNDINFO & sound_device_info, std::ostream & error_output);
	bool LoadOGG(const std::string & filename, const SOUNDINFO & sound_device_info, std::ostream & error_output);

public:
	SOUNDBUFFER() : info(0,0,0,0),loaded(false),sound_buffer(NULL),size(0) {  }
	~SOUNDBUFFER() {  Unload();  }
	
	bool Load(const std::string & filename, const SOUNDINFO & sound_device_info, std::ostream & error_output)
	{
		if (filename.find(".wav") != std::string::npos)
			return LoadWAV(filename, sound_device_info, error_output);
		else
		if (filename.find(".ogg") != std::string::npos)
			return LoadOGG(filename, sound_device_info, error_output);
		else
		{	error_output << "Unable to determine file type from filename: " << filename << std::endl;
			return false;	}
	}
	void Unload()
	{
		if (loaded && sound_buffer != NULL)
			delete [] sound_buffer;
		sound_buffer = NULL;
	}
	inline int GetSample16bit(const unsigned int channel, const unsigned int position) const
	{
		return ((short *)sound_buffer)[ position*info.GetChannels() + (channel-1)*(info.GetChannels()-1) ];
	}
	const std::string & GetName() const {  return name;  }
	const SOUNDINFO & GetSoundInfo() const	{	return info;  }
	bool GetLoaded() const	{	return loaded;	}
	
};

class SOUNDBUFFERLIBRARY
{
	private:
		std::string librarypath;
		std::map <std::string, SOUNDBUFFER> buffermap;
		bool FileExists(const std::string & filename)
		{
			std::ifstream f(filename.c_str());
			return f;
		}
		
	public:
		//  set the path to the sound buffers, minus the trailing
		void SetLibraryPath(const std::string & newpath)
		{
			librarypath = newpath;
		}
		
		//  buffername is the path to the sound minus the path prefix and file extension postfix
		bool Load(const std::string & buffername, const SOUNDINFO & sound_device_info, std::ostream & error_output)
		{
			std::map <std::string, SOUNDBUFFER>::iterator existing = buffermap.find(buffername);
			if (existing != buffermap.end())  // already loaded
				return true;
			
			std::string /*filename = librarypath+"/"+buffername+".ogg";
			if (!FileExists(filename))*/
				filename = librarypath+"/"+buffername+".wav";
			 
			if (!buffermap[buffername].Load(filename, sound_device_info, error_output))
			{
				buffermap.erase(buffername);
				return false;
			}
			else
				return true;
		}
		
		const SOUNDBUFFER * GetBuffer(const std::string & name) const
		{
			std::map <std::string, SOUNDBUFFER>::const_iterator buff = buffermap.find(name);
			if (buff != buffermap.end())
				return &(buff->second);
			else
				return NULL;  // not found
		}
		const SOUNDBUFFER * GetBuffer(const std::string & name, std::ostream & error_output) const
		{
			const SOUNDBUFFER * buf = GetBuffer(name);
			if (!buf){	error_output << "Can't load sound: " << name << std::endl;	return NULL;	}
			return buf;
		}
};

#define MAX_FILTER_ORDER 5

class SOUNDFILTER
{
private:
	int order;
	float xc[MAX_FILTER_ORDER+1], statex[2][MAX_FILTER_ORDER+1];
	float yc[MAX_FILTER_ORDER+1], statey[2][MAX_FILTER_ORDER+1];

public:
	SOUNDFILTER() : order(0) {  ClearState();  }
	
	void ClearState();
	void SetFilter(const int neworder, const float * xcoeff, const float * ycoeff);  // the coefficients are arrays like xcoeff[neworder+1], note ycoeff[0] is ignored
	void Filter(int * chan1, int * chan2, const int len);
	void SetFilterOrder1(float xc0, float xc1, float yc1);  // yc0 is ignored
	void SetFilterOrder0(float xc0);  // yc0 is ignored
};


class SOUNDSOURCE
{
private:
	unsigned int sample_pos;
	float sample_pos_remainder;

	int playing, loop;
	bool autodelete, effects3d;

	float gain, relative_gain, pitch,last_pitch;
	float computed_gain1, last_computed_gain1;  // c-computed
	float computed_gain2, last_computed_gain2;

	MATHVECTOR <float,3> position, velocity;
	const SOUNDBUFFER * buffer;
	std::list <SOUNDFILTER> filters;

public:
	SOUNDSOURCE() :
		sample_pos(0),sample_pos_remainder(0.0f),playing(0),loop(false),autodelete(false),
		gain(1.0),pitch(1.0),last_pitch(1.0), computed_gain1(1.0),computed_gain2(1.0),last_computed_gain1(0.0),last_computed_gain2(0.0),
		effects3d(true),relative_gain(1.0),buffer(NULL)
	{  }

	void SetBuffer(const SOUNDBUFFER & newbuf)
	{	buffer = &newbuf;  SetLoop(false);  Stop();  }
	
	void Setup(const SOUNDBUFFER & newbuf,
		bool b3D, bool bLoop, float gain)
	{
		SetBuffer(newbuf);  Set3DEffects(b3D);  SetLoop(bLoop);  SetGain(gain);
	}

	bool Setup(const SOUNDBUFFERLIBRARY & sndLib, const std::string & name, std::ostream & error_output,
		bool b3D, bool bLoop, float gain)
	{
		const SOUNDBUFFER * buf = sndLib.GetBuffer(name, error_output);
		if (!buf)  return false;
		SetBuffer(*buf);  Set3DEffects(b3D);  SetLoop(bLoop);  SetGain(gain);
		return true;
	}

	void Seek4(int i)
	{
		int samples = GetSoundBuffer().GetSoundInfo().GetSamples();
		SeekToSample((samples/4)*i);  Play();
	}

	void SampleAndAdvanceWithPitch16bit(int * chan1, int * chan2, int len);
	void IncrementWithPitch(int num);
	void Increment(int num);

	void SampleAndAdvance16bit(int * chan1, int * chan2, int len);
	void Sample16bit(unsigned int peekoffset, int & chan1, int & chan2);
	void Advance(unsigned int offset);
	void SeekToSample(const unsigned int newpos) {assert(buffer);assert((int)newpos < buffer->info.GetSamples()/buffer->info.GetChannels());sample_pos = newpos;sample_pos_remainder=0;}

	void SetAutoDelete(const bool newauto) {autodelete = newauto;}
	bool GetAutoDelete() const {return autodelete;}

	void SetGain(const float newgain)   {  gain = newgain;  }
	void SetPitch(const float newpitch) {  pitch = newpitch;  }
	void SetGainSmooth(const float newgain, const float dt);
	void SetPitchSmooth(const float newpitch, const float dt);

	const MATHVECTOR <float,3>& GetPosition() const {  return position;  }
	void SetPosition(const MATHVECTOR <float,3>& pos) {  position = pos;  }
	void SetVelocity(const MATHVECTOR <float,3>& vel) {  velocity = vel;  }

	const MATHVECTOR <float,3> & GetVelocity() const {  return velocity;  }
	void Set3DEffects(bool new3d) {  effects3d = new3d;  }
	bool Get3DEffects() const	  {  return effects3d;  }

	void SetComputationResults(float cpg1, float cpg2)	{	computed_gain1 = cpg1;	computed_gain2 = cpg2;	}
	float ComputedGain(const int channel) const  {  if (channel == 1)  return computed_gain1;  else  return computed_gain2;  }
	float GetGain() const  {  return gain;  }
	void SetRelativeGain(const float relgain)  {  relative_gain = relgain;  }
	float GetRelativeGain()  {  return relative_gain;  }
	
	void SetLoop(const bool newloop) {  loop = newloop ? 1 : 0;  }
	void Reset() {  sample_pos = 0;  sample_pos_remainder = 0.f;  }
	void Stop()  {  playing = 0;  Reset();  }
	void Pause() {  playing = 0;  }
	void Play()  {  playing = 1;  }
	void StopPlay()  {  playing = 0;  Reset();  playing = 1;  }
	bool Audible() 	{  return (playing == 1) && (GetGain() > 0);  }

	const std::string GetName() const {  if (buffer == NULL)  return "NULL";  else  return buffer->GetName();  }
	
	const SOUNDBUFFER & GetSoundBuffer() const {  return *buffer;  }
	SOUNDFILTER & AddFilter() {  SOUNDFILTER newfilt;  filters.push_back(newfilt);  return filters.back();  }
	SOUNDFILTER & GetFilter(int num);
	int NumFilters() const {  return filters.size();  }
	void ClearFilters() {  filters.clear();  }
};


class SOUND
{
private:
	bool initdone, paused, disable;

	SOUNDINFO deviceinfo;

	void DetermineActiveSources(std::list <SOUNDSOURCE *> & active_sourcelist, std::list <SOUNDSOURCE *> & inaudible_sourcelist) const;
	void CollectGarbage();

	float gain_estimate;
	SOUNDFILTER volume_filter;

	MATHVECTOR <float,3> lpos, lvel;  // listener
	QUATERNION <float> lrot;
	
	std::list <SOUNDSOURCE *> sourcelist;
	SDL_mutex * sourcelistlock;
	void LockSourceList(),UnlockSourceList();

public:
	SOUND() : initdone(false),paused(true),deviceinfo(0,0,0,0),gain_estimate(1.0),disable(false),sourcelistlock(NULL)
	{	volume_filter.SetFilterOrder0(1.0);	 }
	~SOUND();

	short waveL[4096],waveR[4096];  // just for wave visualization
	
	bool Init(int buffersize, std::ostream & info_output, std::ostream & error_output);
	void Callback16bitstereo(void *unused, Uint8 *stream, int len);
	void Pause(const bool pause_on);

	void AddSource(SOUNDSOURCE & newsource);
	void RemoveSource(SOUNDSOURCE * todel);
	void Clear() {  sourcelist.clear();  }

	void SetListener(const MATHVECTOR <float,3> & npos, const QUATERNION <float> & nrot, const MATHVECTOR <float,3> & nvel)
	{	lpos = npos;  lrot = nrot;  lvel = nvel;  }
	void Compute3DEffects(std::list <SOUNDSOURCE *> & sources, const MATHVECTOR <float,3> & listener_pos, const QUATERNION <float> & listener_rot) const;
	void Compute3DEffects(std::list <SOUNDSOURCE *> & sources) const {  Compute3DEffects(sources, lpos, lrot);  }

	void SetMasterVolume(float newvol) {  volume_filter.SetFilterOrder0(newvol);  }
	void DisableAllSound() {  disable=true;  }
	bool Enabled() const {  return !disable;  }
	const SOUNDINFO & GetDeviceInfo() {  return deviceinfo;  }
};

#endif
