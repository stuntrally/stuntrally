#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "SoundMgr.h"
#include "SoundBase.h"
#include "SoundBaseMgr.h"
using namespace Ogre;

const float Sound::PITCHDOWN_FADE_FACTOR   = 2.0f;
const float Sound::PITCHDOWN_CUTOFF_FACTOR = 2.0f;


//  Init
//---------------------------------------------------------------------------------------
SoundMgr::SoundMgr()
	:disabled(true), instance_counter(0), sound_mgr(0)
{
	sound_mgr = new SoundBaseMgr();
	if (!sound_mgr)
	{	LogO(">  SoundScript: Failed to create Sound Manager");  return;  }

	disabled = sound_mgr->isDisabled();
	if (disabled)
	{	LogO(">  SoundScript: Sound Manager is disabled");  return;  }

	LogO(">  SoundScript: Sound Manager started with " + toStr(sound_mgr->getNumHardwareSources())+" sources");
}

SoundMgr::~SoundMgr()
{
	delete sound_mgr;
}


//  Update
//---------------------------------------------------------------------------------------
void SoundMgr::setCamera(Vector3 position, Vector3 direction, Vector3 up, Vector3 velocity)
{
	if (disabled)  return;
	sound_mgr->setCamera(position, direction, up, velocity);
}

SoundTemplate* SoundMgr::createTemplate(String name, String filename)
{
	// first, search if there is a template name collision
	if (templates.find(name) != templates.end())
		OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "SoundScript with the name " + name + " already exists.", "SoundScriptManager::createTemplate");

	SoundTemplate *ssi = new SoundTemplate(name, filename);
	templates[name] = ssi;
	return ssi;
}


Sound* SoundMgr::createInstance(Ogre::String name, int car)
{
	//  search template
	if (templates.find(name) == templates.end())
		return NULL;

	SoundTemplate* templ = templates[name];

	Sound* inst = new Sound(car, templ, sound_mgr,
		/*templ->file_name+*/"c"+toStr(car)+"-"+toStr(instance_counter));
	++instance_counter;

	//  start looped
	inst->setGain(0.f);
	if (!inst->start_sound)
		inst->start();

	return inst;
}


//  Parse
//---------------------------------------------------------------------------------------
void SoundMgr::parseScript(FileStreamDataStream* stream)
{
	SoundTemplate* sst = 0;
	String line = "";  int cnt=0;

	LogO(">  SoundScript: Parsing"/*+stream->getName()*/);

	while (!stream->eof())
	{
		line = stream->getLine();
		//  ignore comments & blanks
		if (!(line.length() == 0 || line.substr(0,2) == "//"))
		{
			if (sst == 0)
			{
				//  no current SoundScript
				//  so first valid data should be a SoundScript name
				//LogO(">  SoundScriptManager: creating template "+line);
				//LogO(">  "+line);
				sst = createTemplate(line, stream->getName());
				if (!sst)
				{
					//  there is a name collision for this Sound Script
					LogO(">  Error, this sound script is already defined: "+line);
					skipToNextOpenBrace(stream);
					skipToNextCloseBrace(stream);
					continue;
				}
				skipToNextOpenBrace(stream);  ++cnt;
			}else
			{
				//  already in a ss
				if (line == "}")
					sst = 0;  // finished ss
				else
				{	//  attribute
					//  split params on space
					Ogre::StringVector veclineparams = StringUtil::split(line, "\t ", 0);

					if (!sst->setParameter(veclineparams))
						LogO(">  Bad SoundScript attribute line: '"+line);
				}
			}
	}	}
	LogO(">  SoundScript: Parsed: "+toStr(cnt)+" templates.");
}

void SoundMgr::skipToNextCloseBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "}")
	{
		line = stream->getLine();
	}
}

void SoundMgr::skipToNextOpenBrace(FileStreamDataStream* stream)
{
	String line = "";

	while (!stream->eof() && line != "{")
	{
		line = stream->getLine();
	}
}


//  utility
//---------------------------------------------------------------------------------------
void SoundMgr::setPaused(bool mute)
{
	sound_mgr->pauseAll(mute);
}

void SoundMgr::setMasterVolume(float vol)
{
	sound_mgr->setMasterVolume(vol);
}


//  Load script
//---------------------------------------------------------------------------------------
SoundTemplate::SoundTemplate(String name1, String filename1)
	:file_name(filename1), name(name1), free_sound(0)
	,has_start_sound(false), has_stop_sound(false)
	,start_sound_pitch(0.0f), stop_sound_pitch(0.0f), unpitchable(false)
{
}

bool SoundTemplate::setParameter(Ogre::StringVector vec)
{
	if (vec.empty())  return false;

	if (vec[0] == String("start_sound"))
	{
		if (vec.size() < 3)  return false;
		start_sound_pitch = StringConverter::parseReal(vec[1]); // unpitched = 0.0
		start_sound_name  = vec[2];
		has_start_sound   = true;
		return true;
	}

	if (vec[0] == String("stop_sound"))
	{
		if (vec.size() < 3)  return false;
		stop_sound_pitch = StringConverter::parseReal(vec[1]); // unpitched = 0.0
		stop_sound_name  = vec[2];
		has_stop_sound   = true;
		return true;
	}

	if (vec[0] == String("sound"))
	{
		if (vec.size() < 3)  return false;
		if (free_sound >= MAX_SOUNDS_PER_SCRIPT)
		{
			LogO(">  SoundScript: Reached MAX_SOUNDS_PER_SCRIPT limit (" + toStr(MAX_SOUNDS_PER_SCRIPT) + ")");
			return false;
		}
		sound_pitches[free_sound] = StringConverter::parseReal(vec[1]); // unpitched = 0.0
		if (sound_pitches[free_sound] == 0)
			unpitchable = true;

		if (free_sound > 0 && !unpitchable && sound_pitches[free_sound] <= sound_pitches[free_sound - 1])
			return false;

		sound_names[free_sound] = vec[2];
		++free_sound;
		return true;
	}
	return false;
}


///  Sound
//---------------------------------------------------------------------------------------------------------
Sound::Sound(int car1, SoundTemplate* tpl, SoundBaseMgr* mgr1, Ogre::String name)
	:car(car1), templ(tpl), sound_mgr(mgr1)
	,start_sound(NULL), start_sound_pitchgain(0.0f)
	,stop_sound(NULL), stop_sound_pitchgain(0.0f), lastgain(1.0f)
{
	// create sounds
	if (tpl->has_start_sound)
		start_sound = sound_mgr->createSound(tpl->start_sound_name);
	
	if (tpl->has_stop_sound)
		stop_sound = sound_mgr->createSound(tpl->stop_sound_name);
	
	for (int i=0; i < tpl->free_sound; i++)
		sounds[i] = sound_mgr->createSound(tpl->sound_names[i]);
	
	setPitch(0.0f);
	setGain(1.0f);

	LogO(">  Sound created: "+name);
}

void Sound::setPitch(float value)
{
	if (start_sound)
	{
		start_sound_pitchgain = pitchgain_cutoff(templ->start_sound_pitch, value);

		if (start_sound_pitchgain != 0.0f && templ->start_sound_pitch != 0.0f)
			start_sound->setPitch(value / templ->start_sound_pitch);
	}

	if (templ->free_sound)
	{
		// searching the interval
		int up = 0;

		for (up=0; up < templ->free_sound; up++)
			if (templ->sound_pitches[up] > value)
				break;

		if (up == 0)
		{
			// low sound case
			sounds_pitchgain[0] = pitchgain_cutoff(templ->sound_pitches[0], value);

			if (sounds_pitchgain[0] != 0.0f && templ->sound_pitches[0] != 0.0f && sounds[0])
				sounds[0]->setPitch(value / templ->sound_pitches[0]);

			for (int i=1; i < templ->free_sound; i++)
			{
				if (templ->sound_pitches[i] != 0.0f)
					sounds_pitchgain[i] = 0.0f;  // pause?
				else
					sounds_pitchgain[i] = 1.0f;  // unpitched
			}
		}else if (up == templ->free_sound)
		{
			// high sound case
			for (int i=0; i < templ->free_sound-1; i++)
			{
				if (templ->sound_pitches[i] != 0.0f)
					sounds_pitchgain[i] = 0.0f;  // pause?
				else
					sounds_pitchgain[i] = 1.0f;  // unpitched
			}
			
			sounds_pitchgain[templ->free_sound - 1] = 1.0f;

			if (templ->sound_pitches[templ->free_sound - 1] != 0.0f && sounds[templ->free_sound-1])
				sounds[templ->free_sound-1]->setPitch(value / templ->sound_pitches[templ->free_sound - 1]);
		}else
		{
			// middle sound case
			int low = up - 1;

			for (int i=0; i < low; i++)
			{
				if (templ->sound_pitches[i] != 0.0f)
					sounds_pitchgain[i] = 0.0f;  // pause?
				else
					sounds_pitchgain[i] = 1.0f;  // unpitched
			}

			if (templ->sound_pitches[low] != 0.0f && sounds[low])
			{
				sounds_pitchgain[low] = (templ->sound_pitches[up] - value) / (templ->sound_pitches[up] - templ->sound_pitches[low]);
				sounds[low]->setPitch(value / templ->sound_pitches[low]);
			}else
				sounds_pitchgain[low] = 1.0f;  // unpitched

			if (templ->sound_pitches[up] != 0.0f && sounds[up])
			{
				sounds_pitchgain[up]=(value - templ->sound_pitches[low]) / (templ->sound_pitches[up] - templ->sound_pitches[low]);
				sounds[up]->setPitch(value / templ->sound_pitches[up]);
			}else
				sounds_pitchgain[up] = 1.0f;  // unpitched

			for (int i=up+1; i < templ->free_sound; i++)
			{
				if (templ->sound_pitches[i] != 0.0f)
					sounds_pitchgain[i] = 0.0f;  // pause?
				else
					sounds_pitchgain[i] = 1.0f;  // unpitched
			}
		}
	}

	if (stop_sound)
	{
		stop_sound_pitchgain = pitchgain_cutoff(templ->stop_sound_pitch, value);

		if (stop_sound_pitchgain != 0.0f && templ->stop_sound_pitch != 0.0f)
			stop_sound->setPitch(value / templ->stop_sound_pitch);
	}

	//  propagate new gains
	setGain(lastgain);
}

float Sound::pitchgain_cutoff(float sourcepitch, float targetpitch)
{
	if (sourcepitch == 0.0f)
		return 1.0f;  // unpitchable

	if (targetpitch > sourcepitch / PITCHDOWN_FADE_FACTOR)
		return 1.0f;  // pass

	if (targetpitch < sourcepitch / PITCHDOWN_CUTOFF_FACTOR)
		return 0.0f;  // cutoff

	//  linear fading
	return (targetpitch - sourcepitch / PITCHDOWN_CUTOFF_FACTOR) /
		(sourcepitch / PITCHDOWN_FADE_FACTOR - sourcepitch / PITCHDOWN_CUTOFF_FACTOR);
}

void Sound::setGain(float value)
{
	if (start_sound)
		start_sound->setGain(value * start_sound_pitchgain);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setGain(value * sounds_pitchgain[i]);

	if (stop_sound)
		stop_sound->setGain(value * stop_sound_pitchgain);

	lastgain = value;
}

void Sound::seek(float pos)
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->seek(pos);
}

void Sound::setPosition(Vector3 pos, Vector3 velocity)
{
	if (start_sound)
	{	start_sound->setPosition(pos);
		start_sound->setVelocity(velocity);
	}

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setPosition(pos);
			sounds[i]->setVelocity(velocity);
		}

	if (stop_sound)
	{	stop_sound->setPosition(pos);
		stop_sound->setVelocity(velocity);
	}
}

bool Sound::isAudible()
{
	if (start_sound)
		return start_sound->isPlaying();

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			return sounds[i]->isPlaying();

	if (stop_sound)
		return stop_sound->isPlaying();

	return false;
}

void Sound::runOnce()
{
	if (start_sound)
	{
		if (start_sound->isPlaying())
			return;

		start_sound->play();
	}

	for (int i=0; i < templ->free_sound; ++i)
	{
		if (sounds[i])
		{
			if (sounds[i]->isPlaying())
				continue;

			sounds[i]->setLoop(false);
			sounds[i]->play();
		}
	}

	if (stop_sound)
	{
		if (stop_sound->isPlaying())
			return;

		stop_sound->play();
	}
}

void Sound::start()
{
	if (start_sound)
	{	start_sound->stop();
		start_sound->play();
	}

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
		{	sounds[i]->setLoop(true);
			sounds[i]->play();
		}
}

void Sound::stop()
{
	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::kill()
{
	for (int i = 0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->stop();

	if (start_sound)
		start_sound->stop();

	if (stop_sound)
	{	stop_sound->stop();
		stop_sound->play();
	}
}

void Sound::setEnabled(bool e)
{
	if (start_sound)
		start_sound->setEnabled(e);

	if (stop_sound)
		stop_sound->setEnabled(e);

	for (int i=0; i < templ->free_sound; ++i)
		if (sounds[i])
			sounds[i]->setEnabled(e);
}
