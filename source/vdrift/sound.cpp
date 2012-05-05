#include "pch.h"
#include "sound.h"
#include "unittest.h"
#include "endian_utility.h"

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include <sstream>
using std::stringstream;

#include <list>
using std::list;

#include <iostream>
using std::endl;

#include <string>
using std::string;

#ifdef __APPLE__
#include <Vorbis/vorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif
//#include "../ogre/common/Defines.h"


//  Load sound file
//--------------------------------------------------------------------------------------------------------------------
bool SOUNDBUFFER::LoadWAV(const string & filename, const SOUNDINFO & sound_device_info, std::ostream & error_output)
{
	if (loaded)
		Unload();

	name = filename;
	FILE *fp;
	unsigned int size;

	fp = fopen(filename.c_str(), "rb");
	if (!fp)
	{	error_output << "Can't open sound file: "+filename << endl;  goto error;	}
	else
	{
		char id[5];  //four bytes to hold 'RIFF'
		if (fread(id,sizeof(char),4,fp) != 4)  goto error;
		id[4] = '\0';

		if (strcmp(id,"RIFF"))
		{	error_output << "Sound file doesn't have RIFF header: "+filename << endl;  goto error;	}
		else
		{
			if (fread(&size,sizeof(unsigned int),1,fp) != 1)  goto error;  //read in 32bit size value
			size = ENDIAN_SWAP_32(size);

			if (fread(id,sizeof(char),4,fp)!= 4)  goto error;  //read in 4 byte string now

			if (strcmp(id,"WAVE"))
			{	error_output << "Sound file doesn't have WAVE header: "+filename << endl;  goto error;	}
			else
			{
				if (fread(id,sizeof(char),4,fp)!= 4)  goto error;  //read in 4 bytes "fmt ";

				if (strcmp(id,"fmt "))
				{	error_output << "Sound file doesn't have \"fmt\" header: "+filename << endl;  goto error;	}
				else
				{
					unsigned int format_length, sample_rate, avg_bytes_sec;
					short format_tag, channels, block_align, bits_per_sample;

					if (fread(&format_length, sizeof(unsigned int),1,fp) != 1)  goto error;
					format_length = ENDIAN_SWAP_32(format_length);

					if (fread(&format_tag, sizeof(short), 1, fp) != 1)  goto error;
					format_tag = ENDIAN_SWAP_16(format_tag);

					if (fread(&channels, sizeof(short),1,fp) != 1)  goto error;
					channels = ENDIAN_SWAP_16(channels);

					if (fread(&sample_rate, sizeof(unsigned int), 1, fp) != 1)  goto error;
					sample_rate = ENDIAN_SWAP_32(sample_rate);

					if (fread(&avg_bytes_sec, sizeof(unsigned int), 1, fp) != 1)  goto error;
					avg_bytes_sec = ENDIAN_SWAP_32(avg_bytes_sec);

					if (fread(&block_align, sizeof(short), 1, fp) != 1)  goto error;
					block_align = ENDIAN_SWAP_16(block_align);

					if (fread(&bits_per_sample, sizeof(short), 1, fp) != 1)  goto error;
					bits_per_sample = ENDIAN_SWAP_16(bits_per_sample);


					//new wave seeking code
					//find data chunk
					bool found_data_chunk = false;
					long filepos = format_length + 4 + 4 + 4 + 4 + 4;
					int chunknum = 0;
					while (!found_data_chunk && chunknum < 10)
					{
						fseek(fp, filepos, SEEK_SET); //seek to the next chunk
						if (fread(id, sizeof(char), 4, fp) != 4)  goto error; //read in 'data'
						if (fread(&size, sizeof(unsigned int), 1, fp) != 1)  goto error; //how many bytes of sound data we have
						size = ENDIAN_SWAP_32(size);
						if (!strcmp(id,"data"))
						{
							found_data_chunk = true;
						}
						else
						{
							filepos += size + 4 + 4;
						}

						chunknum++;
					}

					if (chunknum >= 10)
					{
						error_output << "Couldn't find wave data in first 10 chunks of " << filename << endl;
						goto error;
					}

					sound_buffer = new char[size];

					if (fread(sound_buffer, sizeof(char), size, fp) != size)  goto error; //read in our whole sound data chunk

					#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					if (bits_per_sample == 16)
					{
						for (unsigned int i = 0; i < size/2; i++)
						{
							//cout << "preswap i: " << sound_buffer[i] << "preswap i+1: " << sound_buffer[i+1] << endl;
							//short preswap = ((short *)sound_buffer)[i];
							((short *)sound_buffer)[i] = ENDIAN_SWAP_16(((short *)sound_buffer)[i]);
							//cout << "postswap i: " << sound_buffer[i] << "postswap i+1: " << sound_buffer[i+1] << endl;
							//cout << (int) i << "/" << (int) size << endl;
							//short postswap = ((short *)sound_buffer)[i];
							//cout << preswap << "/" << postswap << endl;

						}
					}
					//else if (bits_per_sample != 8)
					else
					{
						error_output << "Sound file with " << bits_per_sample << " bits per sample not supported" << endl;
						goto error;
					}
					#endif

					info = SOUNDINFO(size/(bits_per_sample/8), sample_rate, channels, bits_per_sample/8);
					SOUNDINFO original_info(size/(bits_per_sample/8), sample_rate, channels, bits_per_sample/8);

					loaded = true;
					SOUNDINFO desired_info(original_info.GetSamples(), sound_device_info.GetFrequency(), original_info.GetChannels(), sound_device_info.GetBytesPerSample());
					//ConvertTo(desired_info);

					if (!(desired_info == original_info))
					{
						error_output << "SOUND FORMAT:" << endl;
						original_info.DebugPrint(error_output);
						error_output << "DESIRED FORMAT:" << endl;
						desired_info.DebugPrint(error_output);

						error_output << "Sound file isn't in desired format: "+filename << endl;
						goto error;
					}
				}
			}
		}
	}

	//cout << size << endl;
	return true;
error:
	fclose(fp);
	return false;
}

bool SOUNDBUFFER::LoadOGG(const string & filename, const SOUNDINFO & sound_device_info, std::ostream & error_output)
{
	if (loaded)
		Unload();

	name = filename;
	FILE *fp;
	unsigned int samples;

	fp = fopen(filename.c_str(), "rb");
	if (fp)
	{
		vorbis_info *pInfo;
		OggVorbis_File oggFile;

		ov_open_callbacks(fp, &oggFile, NULL, 0, OV_CALLBACKS_DEFAULT);
		pInfo = ov_info(&oggFile, -1);

		//I assume ogg is always 16-bit (2 bytes per sample) -Venzon
		samples = ov_pcm_total(&oggFile,-1);
		info = SOUNDINFO(samples*pInfo->channels, pInfo->rate, pInfo->channels, 2);
		
		SOUNDINFO desired_info(info.GetSamples(), sound_device_info.GetFrequency(), info.GetChannels(), sound_device_info.GetBytesPerSample());

		if (!(desired_info == info))
		{
			error_output << "SOUND FORMAT:" << endl;
			info.DebugPrint(error_output);
			error_output << "DESIRED FORMAT:" << endl;
			desired_info.DebugPrint(error_output);

			error_output << "Sound file isn't in desired format: "+filename << endl;
			ov_clear(&oggFile);
			return false;
		}

		//allocate space
		unsigned int size = info.GetSamples()*info.GetChannels()*info.GetBytesPerSample();
		sound_buffer = new char[size];
		int bitstream;
		int endian = 0; //0 for Little-Endian, 1 for Big-Endian
		int wordsize = 2; //again, assuming ogg is always 16-bits
		int issigned = 1; //use signed data

		int bytes = 1;
		unsigned int bufpos = 0;
		while (bytes > 0)
		{
			bytes = ov_read(&oggFile, sound_buffer+bufpos, size-bufpos, endian, wordsize, issigned, &bitstream);
			bufpos += bytes;
			//cout << bytes << "...";
		}

		loaded = true;

		//note: no need to call fclose(); ov_clear does it for us
		ov_clear(&oggFile);

		return true;
	}
	else
	{
		error_output << "Can't open sound file: "+filename << endl;
		return false;
	}
}

void SOUND_CallbackWrapper(void *soundclass, Uint8 *stream, int len)
{
	((SOUND*)soundclass)->Callback16bitstereo(soundclass, stream, len);
}

//  Init
//--------------------------------------------------------------------------------------------------------------------
bool SOUND::Init(int buffersize, std::ostream & info_output, std::ostream & error_output)
{
	if (disable || initdone)
		return false;
	
	sourcelistlock = SDL_CreateMutex();

	SDL_AudioSpec desired, obtained;
	desired.freq = 44100;
	desired.format = AUDIO_S16SYS;
	desired.samples = buffersize;
	desired.callback = SOUND_CallbackWrapper;
	desired.userdata = this;
	desired.channels = 2;

	if (SDL_OpenAudio(&desired, &obtained) < 0)
	{
		//string error = SDL_GetError();
		error_output << "Error opening audio device, disabling sound." << endl;
		DisableAllSound();
		return false;
	}

	int frequency = obtained.freq;
	int channels = obtained.channels;
	int samples = obtained.samples;
	int bytespersample = 2;
	if (obtained.format == AUDIO_U8 || obtained.format == AUDIO_S8)
		bytespersample = 1;

	if (obtained.format != desired.format)
	{
		error_output << "Obtained audio format isn't the same as the desired format, disabling sound." << std::endl;
		DisableAllSound();
		return false;
	}

	std::stringstream dout;
	dout << "Obtained audio device:" << std::endl;
	dout << "Frequency: " << frequency << std::endl;
	dout << "Format: " << obtained.format << std::endl;
	dout << "Bits per sample: " << bytespersample * 8 << std::endl;
	dout << "Channels: " << channels << std::endl;
	dout << "Silence: " << (int) obtained.silence << std::endl;
	dout << "Samples: " << samples << std::endl;
	dout << "Size: " << (int) obtained.size << std::endl;
	info_output << "Sound initialization information:" << std::endl << dout.str();
	//cout << dout.str() << endl;

	if (bytespersample != 2 || obtained.channels != desired.channels || obtained.freq != desired.freq)
	{
		error_output << "Sound interface did not create a 44.1kHz, 16 bit, stereo device as requested.  Disabling sound." << endl;
		DisableAllSound();
		return false;
	}

	deviceinfo = SOUNDINFO(samples, frequency, channels, bytespersample);

	initdone = true;
	SetMasterVolume(1.0);

	return true;
}

void SOUND::Pause(const bool pause_on)
{
	if (paused == pause_on) //take no action if no change
		return;

	paused = pause_on;
	if (pause_on)
	{
		//cout << "sound pause on" << endl;
		SDL_PauseAudio(1);
	}
	else
	{
		//cout << "sound pause off" << endl;
		SDL_PauseAudio(0);
	}
}

//  Sound callback
//--------------------------------------------------------------------------------------------------------------------
void SOUND::Callback16bitstereo(void *myself, Uint8 *stream, int len)
{
	assert(this == myself);
	assert(initdone);

	std::list <SOUNDSOURCE *> active_sourcelist;
	std::list <SOUNDSOURCE *> inactive_sourcelist;
	
	LockSourceList();
	
	DetermineActiveSources(active_sourcelist, inactive_sourcelist);
	Compute3DEffects(active_sourcelist);//, cam.GetPosition().ScaleR(-1), cam.GetRotation());

	//increment inactive sources
	for (std::list <SOUNDSOURCE *>::iterator s = inactive_sourcelist.begin(); s != inactive_sourcelist.end(); s++)
	{
		(*s)->IncrementWithPitch(len/4);
	}

	int* buffer1 = new int[len/4];
	int* buffer2 = new int[len/4];
	int* out = new int[len/2];
	for (std::list <SOUNDSOURCE *>::iterator s = active_sourcelist.begin(); s != active_sourcelist.end(); s++)
	{
		SOUNDSOURCE * src = *s;
		src->SampleAndAdvanceWithPitch16bit(buffer1, buffer2, len/4);

		for (int f = 0; f < src->NumFilters(); ++f)
		{
			src->GetFilter(f).Filter(buffer1, buffer2, len/4);
		}
		volume_filter.Filter(buffer1, buffer2, len/4);

		if (s == active_sourcelist.begin())
			for (int i = 0; i < len/4; ++i)
			{
				int pos = i*2;
				short y0 = std::min(32767, std::max(-32767, buffer1[i] ));
				short y1 = std::min(32767, std::max(-32767, buffer2[i] ));
				//short y0 = buffer1[i];
				//short y1 = buffer2[i];
				out[pos]   = y0;
				out[pos+1] = y1;
			}
		else
			for (int i = 0; i < len/4; ++i)
			{
				int pos = i*2;
				short y0 = std::min(32767, std::max(-32767, buffer1[i] ));
				short y1 = std::min(32767, std::max(-32767, buffer2[i] ));
				//short y0 = buffer1[i];
				//short y1 = buffer2[i];
				out[pos]   += y0;
				out[pos+1] += y1;
			}
	}
	delete[]buffer1;
	delete[]buffer2;
	
	//  send to out
	for (int i = 0; i < len/4; ++i)
	{
		int pos = i*2;

		short y0 = std::min(32767, std::max(-32767, out[pos]   ));
		short y1 = std::min(32767, std::max(-32767, out[pos+1] ));

		((short *) stream)[pos]   = y0;
		((short *) stream)[pos+1] = y1;

		waveL[i] = y0;  // for vis osc only ..
		waveR[i] = y1;  // if (pSet->graph_type == 1) ..
	}
	delete[]out;
	
	UnlockSourceList();

	//cout << active_sourcelist.size() << "," << inactive_sourcelist.size() << endl;

	if (active_sourcelist.empty())
	{
		for (int i = 0; i < len/4; i++)
		{
			int pos = i*2;
			((short *) stream)[pos] = ((short *) stream)[pos+1] = 0;
		}
	}

	CollectGarbage();
	//LogO("Snd len: "+toStr(len));
	//cout << "Callback: " << len << endl;
}

SOUND::~SOUND()
{
	if (initdone)
	{
		SDL_CloseAudio();
	}
	
	if (sourcelistlock)
		SDL_DestroyMutex(sourcelistlock);
}


//  Advance
//--------------------------------------------------------------------------------------------------------------------
void SOUNDSOURCE::SampleAndAdvanceWithPitch16bit(int * chan1, int * chan2, int len)
{
	// output buffers chan1 (left) and chan2 (right) will be filled with "len" 16-bit samples
	assert(buffer);
	int samples = buffer->info.GetSamples();  // the number of 16-bit samples in the buffer with left and right channels SUMMED (so for stereo signals the number of samples per channel is samples/2)

	float n_remain = sample_pos_remainder;  // the fractional portion of the current playback position for this soundsource
	int n = sample_pos;  // the integer portion of the current playback position for this soundsource PER CHANNEL (i.e., this will range from 0 to samples/2)
	float samplecount = 0;  // floating point record of how far the playback position has increased during this callback

	const int chan = buffer->info.GetChannels();
	samples -= samples % chan;  // correct the number of samples in odd situations where we have stereo channels but an odd number of channels

	const int samples_per_channel = samples / chan;  // how many 16-bit samples are in a channel of audio
	assert((int)sample_pos <= samples_per_channel);

	last_pitch = pitch;  // this bypasses pitch rate limiting, because it isn't a very useful effect, turns out.

	if (playing)
	{
		assert(len > 0);

		int samp1[2];  // sample(s) to the left of the floating point sample position
		int samp2[2];  // sample(s) to the right of the floating point sample position

		int idx = n*chan;  // the integer portion of the current 16-bit playback position in the input buffer, accounting for duplication of samples for stereo waveforms

		const float maxrate = 1.0/(44100.0*0.01);  // the maximum allowed rate of gain change per sample
		const float negmaxrate = -maxrate;
		//const float maxpitchrate = maxrate;  // the maximum allowed rate of pitch change per sample
		//const float negmaxpitchrate = -maxpitchrate;
		int16_t * buf = (int16_t *)buffer->sound_buffer;  // access to the 16-bit sound input buffer
		const int chaninc = chan - 1;  // the offset to use when fetching the second channel from the sound input buffer (yes, this will be zero for a mono sound buffer)

		float gaindiff1(0), gaindiff2(0);//, pitchdiff(0); //allocate memory here so we don't have to in the loop

		for (int i = 0; i  < len; ++i)
		{
			//do gain change rate limiting
			gaindiff1 = computed_gain1 - last_computed_gain1;
			gaindiff2 = computed_gain2 - last_computed_gain2;
				 if (gaindiff1 > maxrate)  gaindiff1 = maxrate;
			else if (gaindiff1 < negmaxrate) gaindiff1 = negmaxrate;
				 if (gaindiff2 > maxrate)  gaindiff2 = maxrate;
			else if (gaindiff2 < negmaxrate)  gaindiff2 = negmaxrate;
			last_computed_gain1 = last_computed_gain1 + gaindiff1;
			last_computed_gain2 = last_computed_gain2 + gaindiff2;

			//do pitch change rate limiting
			/*pitchdiff = pitch - last_pitch;
				 if (pitchdiff > maxpitchrate)  pitchdiff = maxpitchrate;
			else if (pitchdiff < negmaxpitchrate)  pitchdiff = negmaxpitchrate;
			last_pitch = last_pitch + pitchdiff;*/

			if (n >= samples_per_channel && !loop) //end playback if we've finished playing the buffer and looping is not enabled
			{
				//stop playback
				chan1[i] = chan2[i] = 0;
				playing = 0;
			}
			else //if not at the end of a non-looping sample, or if the sample is looping
			{
				idx = chan*(n % samples_per_channel);  // recompute the buffer position accounting for looping
				assert(idx+chaninc < samples);  // make sure we don't read past the end of the buffer

				samp1[0] = buf[idx];
				samp1[1] = buf[idx+chaninc];

				idx = (idx + chan) % samples;
				assert(idx+chaninc < samples);

				samp2[0] = buf[idx];
				samp2[1] = buf[idx+chaninc];

				//samp2[0] = samp1[0]; //disable interpolation, for debug purposes
				//samp2[1] = samp1[1];

				// set the output buffer to the linear interpolation between the left and right samples for channels
				chan1[i] = (int) ((n_remain*(samp2[0] - samp1[0]) + samp1[0])*last_computed_gain1);
				chan2[i] = (int) ((n_remain*(samp2[1] - samp1[1]) + samp1[1])*last_computed_gain2);

				n_remain += last_pitch;		// increment the playback position
				const unsigned int ninc = (unsigned int) n_remain;
				n += ninc;					// update the integer portion of the playback position
				n_remain -= (float) ninc;	// update the fractional portion of the playback position

				samplecount += last_pitch; //increment the playback delta position counter.  this will eventually be added to the soundsource playback position variables.
			}
		}

		double newpos = sample_pos + sample_pos_remainder + samplecount;  // calculate a floating point new playback position based on where we started plus how many samples we just played

		if (newpos >= samples_per_channel && !loop)  // end playback if we've finished playing the buffer and looping is not enabled
			playing = 0;
		else
		{
			while (newpos >= samples_per_channel)  // newpos = newpos % samples
				newpos -= samples_per_channel;
			sample_pos = (unsigned int) newpos;			 // save the integer portion of the current playback position back to the soundsource
			sample_pos_remainder = newpos - sample_pos;	 // save the fractional portion of the current playback position back to the soundsource
		}
		
		if (playing)
			assert((int)sample_pos <= samples_per_channel);
		//assert(0);
	}
	else  // if not playing
	{
		for (int i = 0; i  < len; ++i)
		{
			chan1[i] = chan2[i] = 0;  // fill output buffers with silence
		}
	}
}


inline void SOUNDSOURCE::IncrementWithPitch(int num)
{
	int samples = buffer->GetSoundInfo().GetSamples()/buffer->GetSoundInfo().GetChannels();
	double newpos = sample_pos + sample_pos_remainder + (num)*pitch;
	if (newpos >= samples && !loop)
		playing = 0;
	else
	{
		while (newpos >= samples)
			newpos -= samples;
		sample_pos = (unsigned int) newpos;
		sample_pos_remainder = newpos - sample_pos;
	}
}

//  Sources
//--------------------------------------------------------------------------------------------------------------------
void SOUND::CollectGarbage()
{
	if (disable)
		return;

	std::list <SOUNDSOURCE *> todel;
	for (std::list <SOUNDSOURCE *>::iterator i = sourcelist.begin(); i != sourcelist.end(); ++i)
	{
		if (!(*i)->Audible() && (*i)->GetAutoDelete())
		{
			todel.push_back(*i);
		}
	}

	for (std::list <SOUNDSOURCE *>::iterator i = todel.begin(); i != todel.end(); ++i)
	{
		RemoveSource(*i);
	}

	//cout << sourcelist.size() << endl;
}

void SOUND::DetermineActiveSources(std::list <SOUNDSOURCE *> & active_sourcelist, std::list <SOUNDSOURCE *> & inaudible_sourcelist) const
{
	active_sourcelist.clear();
	inaudible_sourcelist.clear();
	//int sourcenum = 0;
	for (std::list <SOUNDSOURCE *>::const_iterator i = sourcelist.begin(); i != sourcelist.end(); ++i)
	{
		if ((*i)->Audible())
		{
			active_sourcelist.push_back(*i);
			//cout << "Tick: " << &(*i) << endl;
			//cout << "Source is audible: " << i->GetName() << ", " << i->GetGain() << ":" << i->ComputedGain(1) << "," << i->ComputedGain(2) << endl;
			//cout << "Source " << sourcenum << " is audible: " << i->GetName() << endl;
		}
		else
		{
			inaudible_sourcelist.push_back(*i);
		}
		//sourcenum++;
	}
	//cout << "sounds active: " << active_sourcelist.size() << ", sounds inactive: " << inaudible_sourcelist.size() << endl;
}

void SOUND::AddSource(SOUNDSOURCE & newsource)
{
	if (disable)
		return;

	LockSourceList();
	sourcelist.push_back(&newsource);
	UnlockSourceList();
}

void SOUND::RemoveSource(SOUNDSOURCE * todel)
{
	if (disable)
		return;

	assert(todel);

	std::list <SOUNDSOURCE *>::iterator delit = sourcelist.end();
	for (std::list <SOUNDSOURCE *>::iterator i = sourcelist.begin(); i != sourcelist.end(); ++i)
	{
		if (*i == todel)
			delit = i;
	}

	//assert(delit != sourcelist.end()); //can't find source to delete //update: don't assert, just do a check

	LockSourceList();
	if (delit != sourcelist.end())
		sourcelist.erase(delit);
	UnlockSourceList();
}

/*void SOUND::NewSourcePlayOnce(const string & buffername)
{
	if (disable)
		return;

	SOUNDSOURCE & src = NewSource(buffername);
	src.SetAutoDelete(true);
	src.Set3DEffects(false);
	src.Play();
}*/


//  3D effects
//--------------------------------------------------------------------------------------------------------------------
void SOUND::Compute3DEffects(std::list <SOUNDSOURCE *> & sources, const MATHVECTOR <float, 3> & listener_pos, const QUATERNION <float> & listener_rot) const
{
	for (std::list <SOUNDSOURCE *>::iterator i = sources.begin(); i != sources.end(); ++i)
	{
		if ((*i)->Get3DEffects())
		{
			MATHVECTOR <float, 3> relvec = (*i)->GetPosition() - listener_pos;
			float len = relvec.Magnitude();
			if (len < 0.1)
			{
				relvec[2] = 0.1;
				len = relvec.Magnitude();
			}
			listener_rot.RotateVector(relvec);

			//  attenuation
			//float cgain = 0.25 / log(100.0) * (log(1000.0) - 1.6 * log(len));
			float cgain = log(1000.0 / pow((double)len, 1.3)) / log(100.0);
			cgain = std::min(1.f, std::max(0.f, cgain));

			//  pan
			float xcoord = -relvec.Normalize()[0];
			float pgain1 = std::max(0.f, -xcoord);
			float pgain2 = std::max(0.f,  xcoord);

			(*i)->SetComputationResults(cgain*(*i)->GetGain()*(1.f-pgain1), cgain*(*i)->GetGain()*(1.f-pgain2));
		}
		else
		{
			(*i)->SetComputationResults((*i)->GetGain(), (*i)->GetGain());
		}
	}
}

void SOUNDSOURCE::SetGainSmooth(const float newgain, const float dt)
{
	//float coeff = dt*40.0;
	if (dt <= 0)
		return;

	//low pass filter

	//rate limit
	float ndt = dt * 4.0;
	float delta = newgain - gain;
	if (delta > ndt)  delta = ndt;
	if (delta < -ndt)  delta = -ndt;
	gain = gain + delta;
}

void SOUNDSOURCE::SetPitchSmooth(const float newpitch, const float dt)
{
	//float coeff = dt*40.0;
	if (dt > 0)
	{
		//low pass filter

		//rate limit
		float ndt = dt * 4.0;
		float delta = newpitch - pitch;
		if (delta > ndt)  delta = ndt;
		if (delta < -ndt)  delta = -ndt;
		pitch = pitch + delta;
	}
}

//the coefficients are arrays of size xcoeff[neworder+1]
void SOUNDFILTER::SetFilter(const int neworder, const float * xcoeff, const float * ycoeff)
{
	assert(!(neworder > MAX_FILTER_ORDER || neworder < 0));
		//cerr << __FILE__ << "," << __LINE__ << "Filter order is larger than maximum order" << endl;
		//UNRECOVERABLE_ERROR_FUNCTION(__FILE__,__LINE__,"Filter order is larger than maximum order");

	order = neworder;
	for (int i = 0; i < neworder+1; i++)
	{
		xc[i] = xcoeff[i];
		yc[i] = ycoeff[i];
	}
}

void SOUNDFILTER::ClearState()
{
	for (int i = 0; i < MAX_FILTER_ORDER+1; ++i)
	{	xc[i] = 0.f;  yc[i] = 0.f;  }
	for (int c = 0; c < 2; ++c)
	for (int i = 0; i < MAX_FILTER_ORDER+1; ++i)
	{	statex[c][i] = 0.f;  statey[c][i] = 0.f;  }
}

void SOUNDFILTER::Filter(int * chan1, int * chan2, const int len)
{
	for (int i = 0; i < len; i++)
	{
		//store old state
		for (int s = 1; s <= order; s++)
		{
			statex[0][s] = statex[0][s-1];
			statex[1][s] = statex[1][s-1];

			statey[0][s] = statey[0][s-1];
			statey[1][s] = statey[1][s-1];
		}

		//set the sample state for now to the current input
		statex[0][0] = chan1[i];
		statex[1][0] = chan2[i];

		switch (order)
		{
			case 1:
				chan1[i] = (int) (statex[0][0]*xc[0]+statex[0][1]*xc[1]+statey[0][1]*yc[1]);
				chan2[i] = (int) (statex[1][0]*xc[0]+statex[1][1]*xc[1]+statey[1][1]*yc[1]);
				break;
			case 0:
				chan1[i] = (int) (statex[0][0]*xc[0]);
				chan2[i] = (int) (statex[1][0]*xc[0]);
				break;
			default:
				break;
		}

		//store the state of the output
		statey[0][0] = chan1[i];
		statey[1][0] = chan2[i];
	}
}

SOUNDFILTER & SOUNDSOURCE::GetFilter(int num)
{
	int curnum = 0;
	for (std::list <SOUNDFILTER>::iterator i = filters.begin(); i != filters.end(); ++i)
	{
		if (num == curnum)
			return *i;
		curnum++;
	}

	//cerr << __FILE__ << "," << __LINE__ << "Asked for a non-existant filter" << endl;
	//UNRECOVERABLE_ERROR_FUNCTION(__FILE__,__LINE__,"Asked for a non-existant filter");
	assert(0);
	SOUNDFILTER * nullfilt = NULL;
	return *nullfilt;
}

void SOUNDFILTER::SetFilterOrder1(float xc0, float xc1, float yc1)
{
	order = 1;
	xc[0] = xc0;
	xc[1] = xc1;
	yc[1] = yc1;
}

void SOUNDFILTER::SetFilterOrder0(float xc0)
{
	order = 0;
	xc[0] = xc0;
}

void SOUND::LockSourceList()
{
	if(SDL_mutexP(sourcelistlock)==-1){
		assert(0 && "Couldn't lock mutex");
	}
}

void SOUND::UnlockSourceList()
{
	if(SDL_mutexV(sourcelistlock)==-1){
		assert(0 && "Couldn't unlock mutex");
	}
}

